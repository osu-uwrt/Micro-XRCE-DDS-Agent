// Copyright 2017 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/**
 * @file DataWriter.h
 *
 */

#ifndef DATAWRITER_H_
#define DATAWRITER_H_

#include <string>
#include <fastrtps/participant/Participant.h>
#include <fastrtps/publisher/Publisher.h>

#include <agent/types/ShapePubSubTypes.h>

namespace eprosima {
namespace micrortps {

/**
 * Class DataWriter, used to send data to associated datareaders.
 * @ingroup MICRORTPS_MODULE
 */
class DataWriter
{
    virtual ~DataWriter();

public:

    DataWriter(fastrtps::Participant* participant, const std::string &rtps_publisher_profile = "");

    bool init();
    bool write(void* data);

private:

    fastrtps::Participant* mp_rtps_participant;
    fastrtps::Publisher* mp_rtps_publisher;
    std::string m_rtps_publisher_prof;
    ShapeTypePubSubType m_shape_type;
};

} /* namespace micrortps */
} /* namespace eprosima */

#endif /* DATAWRITER_H_ */
