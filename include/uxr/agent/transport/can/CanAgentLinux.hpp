// Copyright 2017-present Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef UXR_AGENT_TRANSPORT_CAN_CANAGENTLINUX_HPP_
#define UXR_AGENT_TRANSPORT_CAN_CANAGENTLINUX_HPP_

#include <uxr/agent/transport/Server.hpp>
#include <uxr/agent/transport/endpoint/CanEndPoint.hpp>
#include <uxr/agent/transport/stream_framing/StreamFramingProtocol.hpp>
#include <sys/poll.h>

#include <uxr/agent/transport/can/canmore_msg_encoding.h>

namespace eprosima {
namespace uxr {

class CANmoreMsgDecoderWrapper
{
public:
    CANmoreMsgDecoderWrapper()
    {
        canmore_msg_decode_init(&decoder_state, NULL, NULL);
    }

    ~CANmoreMsgDecoderWrapper()
    {}

    void reset_state()
    {
        canmore_msg_decode_reset_state(&decoder_state);
    }

    bool decode_frame(uint8_t seq_num, uint8_t *data, size_t data_len)
    {
        return canmore_msg_decode_frame(&decoder_state, seq_num, data, data_len);
    }

    size_t decode_last_frame(uint8_t seq_num, uint8_t *data, size_t data_len, uint32_t crc, uint8_t *data_out)
    {
        return canmore_msg_decode_last_frame(&decoder_state, seq_num, data, data_len, crc, data_out);
    }

private:
    canmore_msg_decoder_t decoder_state;
};

class CanAgent : public Server<CanEndPoint>
{
public:
    CanAgent(
            char const * dev,
            std::vector<uint32_t> client_ids,
            Middleware::Kind middleware_kind);

    ~CanAgent();

    #ifdef UAGENT_DISCOVERY_PROFILE
        bool has_discovery() final { return false; }
    #endif

    #ifdef UAGENT_P2P_PROFILE
        bool has_p2p() final { return false; }
    #endif

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
    uint32_t client_id_mask_;
    struct pollfd poll_fd_;

    std::map<int, CANmoreMsgDecoderWrapper> msg_decoder;
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_TRANSPORT_CAN_CANAGENTLINUX_HPP_
