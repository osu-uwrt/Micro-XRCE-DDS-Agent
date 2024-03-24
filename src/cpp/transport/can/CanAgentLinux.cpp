// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <uxr/agent/transport/can/CanAgentLinux.hpp>
#include <uxr/agent/utils/Conversion.hpp>
#include <uxr/agent/logger/Logger.hpp>
#include "canmore_cpp/MsgAgent.hpp"
#include "canmore/protocol.h"

#include <assert.h>
#include <chrono>
#include <sstream>

#include <unistd.h>

#include <net/if.h>

namespace eprosima {
namespace uxr {

class CanAgent : public Server<CanEndPoint>, public Canmore::AgentMsgHandler
{
public:
    CanAgent(
            char const * dev,
            const std::vector<uint8_t> &client_ids,
            Middleware::Kind middleware_kind):
        Server<CanEndPoint>{middleware_kind}
        , dev_(dev)
        , clientIds_(client_ids),
        rxBuf_(CANMORE_MAX_MSG_LENGTH) {}

    ~CanAgent();

    #ifdef UAGENT_DISCOVERY_PROFILE
        bool has_discovery() final { return false; }
    #endif

    #ifdef UAGENT_P2P_PROFILE
        bool has_p2p() final { return false; }
    #endif

protected:
    void handleMessage(uint8_t clientId, uint8_t subtype, std::span<const uint8_t> data) override;
    void handleDecodeError(uint8_t clientId, unsigned int errorCode) override;
    void handleConflictingAgentError() override;

private:
    bool init() final;
    bool fini() final;
    bool handle_error(
            TransportRc transport_rc) final;

    bool recv_message(
            InputPacket<CanEndPoint>& input_packet,
            int timeout,
            TransportRc& transport_rc) final;

    bool send_message(
            OutputPacket<CanEndPoint> output_packet,
            TransportRc& transport_rc) final;

private:
    const std::string dev_;
    const std::vector<uint8_t> clientIds_;

    uint8_t rxClientId_ = 0;
    size_t rxRecvLen_ = 0;
    std::vector<uint8_t> rxBuf_;

    std::unique_ptr<Canmore::PollGroup> pollGroup_;
    std::unique_ptr<Canmore::MsgAgent> canmoreAgent_;
};

Server<CanEndPoint>* createNewCanAgent(char const * dev,
            const std::vector<uint8_t> &client_ids,
            Middleware::Kind middleware_kind) {
    return new CanAgent(dev, client_ids, middleware_kind);
}

CanAgent::~CanAgent()
{
    try
    {
        stop();
    }
    catch (std::exception& e)
    {
        UXR_AGENT_LOG_CRITICAL(
            UXR_DECORATE_RED("error stopping server"),
            "exception: {}",
            e.what());
    }
}

bool CanAgent::init()
{
    assert(canmoreAgent_ == NULL);

    try
    {
        int ifIdx = if_nametoindex(dev_.c_str());
        if (!ifIdx) {
            throw std::system_error(errno, std::generic_category(), "if_nametoindex");
        }

        canmoreAgent_.reset(new Canmore::MsgAgent(ifIdx, *this, clientIds_));
        pollGroup_.reset(new Canmore::PollGroup());
        pollGroup_->addFd(*canmoreAgent_);

        // Format client ID mask
        std::stringstream ss;
        ss << "{";
        if (clientIds_.empty()) {
            ss << " all ids ";
        } else {
            bool first = true;
            for (uint8_t id : clientIds_) {
                if (!first) {
                    ss << ", ";
                }
                else {
                    first = false;
                }
                ss << (int) id;
            }
        }
        ss << "}";

        UXR_AGENT_LOG_INFO(
            UXR_DECORATE_GREEN("running..."),
            "device: {}, id mask: {}",
            dev_, ss.str());

        return true;
    }
    catch (std::exception &e)
    {
        UXR_AGENT_LOG_ERROR(
            UXR_DECORATE_RED("Error Starting Agent"),
            "what(): {}", e.what());
        return false;
    }
}

bool CanAgent::fini()
{
    bool wasAlive = (canmoreAgent_ != NULL);

    pollGroup_.reset();
    canmoreAgent_.reset();

    if (wasAlive || true) {
        UXR_AGENT_LOG_INFO(
                UXR_DECORATE_GREEN("server stopped"),
                "device: {}", dev_.c_str());
    }
    return true;
}

void CanAgent::handleMessage(uint8_t clientId, uint8_t subtype, std::span<const uint8_t> data) {
    if (subtype != CANMORE_MSG_SUBTYPE_XRCE_DDS) {
        return;
    }
    size_t copyLen = data.size();
    if (rxBuf_.size() < copyLen) {
        copyLen = rxBuf_.size();
    }
    rxClientId_ = clientId;
    std::copy(data.begin(), data.begin() + copyLen, rxBuf_.begin());
    rxRecvLen_ = copyLen;
}

void CanAgent::handleDecodeError(uint8_t clientId, unsigned int errorCode) {
    UXR_AGENT_LOG_WARN(
            UXR_DECORATE_YELLOW("CANmore Decode Error"),
            "client id: {}, error code: {}", clientId, errorCode);
}

void CanAgent::handleConflictingAgentError() {
    UXR_AGENT_LOG_WARN(
            UXR_DECORATE_YELLOW("receive warning"),
            "Another agent detected communicating on dev: {}", dev_.c_str());
}


bool CanAgent::recv_message(
        InputPacket<CanEndPoint>& input_packet,
        int timeout,
        TransportRc& transport_rc)
{
    typedef std::chrono::high_resolution_clock Clock;
    typedef std::chrono::milliseconds ms;

    const auto start = Clock::now();
    rxRecvLen_ = 0;

    try {
        while (rxRecvLen_ == 0) {
            const auto now = Clock::now();
            const auto elapsedTime = std::chrono::duration_cast<ms>(now - start);
            const int64_t remainingMs = timeout - elapsedTime.count();

            if (remainingMs < 0) {
                // Ran out of time, report timeout
                transport_rc = TransportRc::timeout_error;
                return false;
            }
            pollGroup_->processEvent(remainingMs);
        }

        // Report received packet
        input_packet.message.reset(new InputMessage(rxBuf_.data(), rxRecvLen_));
        input_packet.source = CanEndPoint(rxClientId_);

        uint32_t raw_client_key;
        if (Server<CanEndPoint>::get_client_key(input_packet.source, raw_client_key))
        {
            UXR_AGENT_LOG_MESSAGE(
                UXR_DECORATE_YELLOW("[==>> CAN <<==]"),
                raw_client_key,
                input_packet.message->get_buf(),
                input_packet.message->get_len());
        }

        return true;
    }
    catch (std::exception &e) {
        // Write failed
        UXR_AGENT_LOG_ERROR(
            UXR_DECORATE_RED("receive message error"),
            "device: {}, what(): {}",
            dev_, e.what());

        transport_rc = TransportRc::server_error;
        return false;
    }
}

bool CanAgent::send_message(
        OutputPacket<CanEndPoint> output_packet,
        TransportRc& transport_rc)
{
    try {
        canmoreAgent_->transmitMessage(output_packet.destination.get_client_id(), CANMORE_MSG_SUBTYPE_XRCE_DDS,
                        std::span<const uint8_t>(output_packet.message->get_buf(), output_packet.message->get_len()));

        uint32_t raw_client_key;
        if (Server<CanEndPoint>::get_client_key(output_packet.destination, raw_client_key))
        {
            UXR_AGENT_LOG_MESSAGE(
                UXR_DECORATE_YELLOW("[** <<CAN>> **]"),
                raw_client_key,
                output_packet.message->get_buf(),
                output_packet.message->get_len());
        }

        return true;
    }
    catch (std::exception &e) {
        // Write failed
        UXR_AGENT_LOG_ERROR(
            UXR_DECORATE_RED("send message error"),
            "device: {}, client id: {}, what(): {}",
            dev_, output_packet.destination.get_client_id(), e.what());

        transport_rc = TransportRc::server_error;
        return false;
    }

    return false;
}

bool CanAgent::handle_error(
        TransportRc /*transport_rc*/)
{
    if (!fini()) {
        return false;
    }
    return init();
}

} // namespace uxr
} // namespace eprosima
