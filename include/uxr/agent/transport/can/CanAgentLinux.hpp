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
#include <memory>

namespace eprosima {
namespace uxr {

typedef Server<CanEndPoint> CanAgentWrapper;

CanAgentWrapper* createNewCanAgent(char const * dev,
            const std::vector<uint8_t> &client_ids,
            Middleware::Kind middleware_kind);

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_TRANSPORT_CAN_CANAGENTLINUX_HPP_
