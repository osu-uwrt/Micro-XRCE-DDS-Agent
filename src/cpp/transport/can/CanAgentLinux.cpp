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

#include <assert.h>
#include <chrono>
#include <sstream>

#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>

#include <linux/can.h>
#include <linux/can/raw.h>

namespace eprosima {
namespace uxr {

CanAgent::CanAgent(
        char const* dev,
        std::vector<uint32_t> client_ids,
        Middleware::Kind middleware_kind)
    : Server<CanEndPoint>{middleware_kind}
    , dev_{dev}
    , msg_decoder{}
{
    static_assert(sizeof(client_id_mask_) * 8 >= (1<<CANMORE_CLIENT_ID_LENGTH), "Client ID mask cannot fit all possible client IDs");

    // Generate client ID mask (significantly reduces overhead when searching for client ID matches during receive)
    client_id_mask_ = 0;
    for (uint32_t client_id : client_ids)
    {
        if (client_id < 1 || client_id < (1<<CANMORE_CLIENT_ID_LENGTH))
        {
            client_id_mask_ |= (1<<client_id);
        }
        else
        {
            UXR_AGENT_LOG_ERROR(
                UXR_DECORATE_RED("invalid client id"),
                "0x{:x} requested, max ID 0x{:x}, min ID 0x01",
                client_id, (1<<CANMORE_CLIENT_ID_LENGTH)-1);
        }
    }
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
    bool rv = false;

    poll_fd_.fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);

    if (-1 != poll_fd_.fd)
    {
        struct sockaddr_can address {};
        struct ifreq ifr;

        // Get interface index by name
        strcpy(ifr.ifr_name, dev_.c_str());
        if (-1 == ioctl(poll_fd_.fd, SIOCGIFINDEX, &ifr)) {
            UXR_AGENT_LOG_ERROR(
                UXR_DECORATE_RED("ioctl error"),
                "errno: {} ({})",
                strerror(errno), errno);
            return rv;
        }

        memset(&address, 0, sizeof(address));
        address.can_family = AF_CAN;
        address.can_ifindex = ifr.ifr_ifindex;

        // Bind to interface to receive CAN frames
        if (-1 != bind(poll_fd_.fd,
                reinterpret_cast<struct sockaddr*>(&address),
                sizeof(address)))
        {
            poll_fd_.events = POLLIN;
            rv = true;

            // Format client ID mask
            std::stringstream ss;
            ss << "{";
            uint32_t mask = client_id_mask_;
            if (mask == 0) {
                ss << " all ids ";
            } else {
                int i = 0;
                while (mask != 0) {
                    if (mask & 1) {
                        mask >>= 1;
                        ss << fmt::format(" 0x{:x}{}", i, (mask == 0 ? ' ' : ','));
                    } else {
                        mask >>= 1;
                    }
                    i++;
                }
            }
            ss << "}";

            UXR_AGENT_LOG_INFO(
                UXR_DECORATE_GREEN("running..."),
                "device: {}, id mask: {}, fd: {}",
                dev_, ss.str(), poll_fd_.fd);
        }
        else
        {
            UXR_AGENT_LOG_ERROR(
                UXR_DECORATE_RED("SocketCan bind error"),
                "device: {}, errno: {} ({})",
                dev_, strerror(errno), errno);
        }
    }
    else
    {
        UXR_AGENT_LOG_ERROR(
            UXR_DECORATE_RED("SocketCan error"),
            "errno: {} ({})",
            strerror(errno), errno);
    }

    return rv;
}

bool CanAgent::fini()
{
    // Clear all existing decoder states
    msg_decoder.clear();

    if (-1 == poll_fd_.fd)
    {
        return true;
    }

    bool rv = false;
    if (0 == ::close(poll_fd_.fd))
    {
        UXR_AGENT_LOG_INFO(
            UXR_DECORATE_GREEN("server stopped"),
            "fd: {}, device: {}",
            poll_fd_.fd, dev_);
        rv = true;
    }
    else
    {
        UXR_AGENT_LOG_ERROR(
            UXR_DECORATE_RED("close server error"),
            "fd: {}, device: {}, errno: {} ({})",
            poll_fd_.fd, dev_, strerror(errno), errno);
    }

    poll_fd_.fd = -1;
    return rv;
}

bool CanAgent::recv_message(
        InputPacket<CanEndPoint>& input_packet,
        int timeout,
        TransportRc& transport_rc)
{
    // Timeout calculations
    const auto start_time = std::chrono::steady_clock::now();
    const auto end_time = start_time + std::chrono::duration<double, std::milli>(timeout);

    while (true) {
        const auto current_timeout_duration = std::chrono::duration<double, std::milli>(end_time - std::chrono::steady_clock::now());
        int current_timeout_ms = current_timeout_duration.count();
        if (current_timeout_ms < 0) {
            current_timeout_ms = 0;
        }

        int poll_rv = poll(&poll_fd_, 1, current_timeout_ms);

        if (0 < poll_rv)
        {
            can_frame rx_frame;
            if (0 < read(poll_fd_.fd, &rx_frame, sizeof(rx_frame)))
            {

                // Omit RTR, ERR frames
                if ((rx_frame.can_id & CAN_ERR_FLAG) || (rx_frame.can_id & CAN_RTR_FLAG))
                {
                    continue;
                }

                // Decode CAN ID
                bool is_extended = (rx_frame.can_id & CAN_EFF_FLAG ? true : false);
                canmore_id_t id;
                id.identifier = (rx_frame.can_id & CAN_ERR_MASK);

                uint32_t client_id;
                uint32_t type;
                uint32_t direction;
                uint32_t noc;
                uint32_t crc;

                if (is_extended)
                {
                    client_id = id.pkt_ext.client_id;
                    type = id.pkt_ext.type;
                    direction = id.pkt_ext.direction;
                    noc = id.pkt_ext.noc;
                    crc = id.pkt_ext.crc;
                }
                else
                {
                    client_id = id.pkt_std.client_id;
                    type = id.pkt_std.type;
                    direction = id.pkt_std.direction;
                    noc = id.pkt_std.noc;
                    crc = 0;
                }

                // Drop unwanted frames
                if (type != CANMORE_TYPE_MSG)
                {
                    // Frame isn't message type, ignore it
                    continue;
                }

                if (client_id == 0)
                {
                    // client_id 0 is reserved
                    continue;
                }

                if (client_id_mask_ != 0 && ((1<<client_id) & client_id_mask_) == 0)
                {
                    // Message not for this client, ignore it
                    continue;
                }

                if (direction != CANMORE_DIRECTION_CLIENT_TO_AGENT)
                {
                    // Received a frame that only this node is allowed to send?
                    UXR_AGENT_LOG_WARN(
                        UXR_DECORATE_YELLOW("receive warning"),
                        "Another agent detected communicating with client id %d",
                        client_id);
                    continue;
                }

                std::map<int, CANmoreMsgDecoderWrapper>::iterator it = msg_decoder.find(client_id);

                CANmoreMsgDecoderWrapper &decoder = [&]()->CANmoreMsgDecoderWrapper&
                {
                    if (it == msg_decoder.end())
                    {
                        CANmoreMsgDecoderWrapper newDecoder;

                        auto inserted = msg_decoder.insert(std::pair<int, CANmoreMsgDecoderWrapper>(client_id, newDecoder));
                        return inserted.first->second;
                    }
                    else
                    {
                        return it->second;
                    }
                }();

                if (!is_extended) {
                    decoder.decode_frame(noc, rx_frame.data, rx_frame.can_dlc);
                } else {
                    uint8_t msg_tmp[CANMORE_MAX_MSG_LENGTH];
                    size_t msg_len = decoder.decode_last_frame(noc, rx_frame.data, rx_frame.can_dlc, crc, msg_tmp);

                    if (msg_len == 0) {
                        // Decode failed, drop the frame
                        continue;
                    }

                    input_packet.message.reset(new InputMessage(msg_tmp, static_cast<size_t>(msg_len)));
                    input_packet.source = CanEndPoint(client_id);

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
            }
            else
            {
                UXR_AGENT_LOG_ERROR(
                    UXR_DECORATE_RED("recv message error"),
                    "fd: {}, device: {}, errno: {} ({})",
                    poll_fd_.fd, dev_, strerror(errno), errno);

                transport_rc = TransportRc::server_error;
                break;
            }
        }
        else
        {
            if (poll_rv == 0)
            {
                transport_rc = TransportRc::timeout_error;
            }
            else
            {
                UXR_AGENT_LOG_ERROR(
                    UXR_DECORATE_RED("poll message error"),
                    "fd: {}, device: {}, errno: {} ({})",
                    poll_fd_.fd, dev_, strerror(errno), errno);

                transport_rc = TransportRc::server_error;
            }
            break;
        }
    }

    return false;
}

bool CanAgent::send_message(
        OutputPacket<CanEndPoint> output_packet,
        TransportRc& transport_rc)
{
    bool rv = false;

    canmore_msg_encoder_t encoder;
    canmore_msg_encode_init(&encoder, output_packet.destination.get_client_id(), CANMORE_DIRECTION_AGENT_TO_CLIENT);
    canmore_msg_encode_load(&encoder, output_packet.message->get_buf(), output_packet.message->get_len());

    while (!canmore_msg_encode_done(&encoder)) {
        struct can_frame frame = {};

        bool is_extended;
        unsigned dlc;
        canmore_msg_encode_next(&encoder, frame.data, &dlc, &frame.can_id, &is_extended);
        frame.can_dlc = dlc;

        if (is_extended){
            frame.can_id |= CAN_EFF_FLAG;
        }

        if (0 < ::write(poll_fd_.fd, &frame, sizeof(frame)))
        {
            rv = true;
        }
        else
        {
            // Write failed
            UXR_AGENT_LOG_ERROR(
                UXR_DECORATE_RED("send message error"),
                "fd: {}, device: {}, errno: {} ({})",
                poll_fd_.fd, dev_, strerror(errno), errno);

            transport_rc = TransportRc::server_error;
        }
    }

    if (rv)
    {
        uint32_t raw_client_key;
        if (Server<CanEndPoint>::get_client_key(output_packet.destination, raw_client_key))
        {
            UXR_AGENT_LOG_MESSAGE(
                UXR_DECORATE_YELLOW("[** <<CAN>> **]"),
                raw_client_key,
                output_packet.message->get_buf(),
                output_packet.message->get_len());
        }
    }
    return rv;
}

bool CanAgent::handle_error(
        TransportRc /*transport_rc*/)
{
    fini();
    return false;
}

} // namespace uxr
} // namespace eprosima
