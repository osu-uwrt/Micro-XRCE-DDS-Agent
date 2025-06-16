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

#ifndef UXR__AGENT__MIDDLEWARE__RMW__MIDDLEWARE_HPP_
#define UXR__AGENT__MIDDLEWARE__RMW__MIDDLEWARE_HPP_

#include <uxr/agent/middleware/Middleware.hpp>

#include <unordered_map>
#include <memory>
#include <mutex>

#include <rcl/rcl.h>

#define MAX_MSG_LENGTH 2048

struct TopicInfo
{
    uint16_t participant_id;
    std::string
        topic_name,
        topic_type;
};

struct PubSubIngredients
{
    std::string
        topic_name,
        topic_type;

    std::shared_ptr<rcl_node_t> node;
};

template<typename T>
struct PubSubInfo
{
    std::string verified_type_name; //user must guarantee that this type name is in ROSIDL_TYPES
    std::shared_ptr<rcl_node_t> node;
    std::shared_ptr<T> t;
};

namespace eprosima {
namespace uxr {
namespace middleware {
class CallbackFactory;
} // namespace middleware

class RmwMiddleware : public Middleware
{
public:

    size_t next_participant_id;

    RmwMiddleware();
    ~RmwMiddleware();

/**********************************************************************************************************************
 * Create functions.
 **********************************************************************************************************************/
    bool create_participant_by_ref(
        uint16_t participant_id,
        int16_t domain_id,
        const std::string& ref) override;

    bool create_participant_by_xml(
        uint16_t participant_id,
        int16_t domain_id,
        const std::string& xml) override;

    bool create_participant_by_bin(
        uint16_t participant_id,
        const dds::xrce::OBJK_DomainParticipant_Binary& participant_xrce);

    bool create_topic_by_ref(
        uint16_t topic_id,
        uint16_t participant_id,
        const std::string& ref) override;

    bool create_topic_by_xml(
        uint16_t topic_id,
        uint16_t participant_id,
        const std::string& xml) override;

    bool create_topic_by_bin(
        uint16_t topic_id,
        uint16_t participant_id,
        const dds::xrce::OBJK_Topic_Binary& topic_xrce) override;

    bool create_publisher_by_xml(
        uint16_t publisher_id,
        uint16_t participant_id,
        const std::string&) override;

    bool create_publisher_by_bin(
        uint16_t publisher_id,
        uint16_t participant_id,
        const dds::xrce::OBJK_Publisher_Binary& publisher_xrce) override;

    bool create_subscriber_by_xml(
        uint16_t subscirber_id,
        uint16_t participant_id,
        const std::string&) override;

    bool create_subscriber_by_bin(
        uint16_t subscriber_id,
        uint16_t participant_id,
        const dds::xrce::OBJK_Subscriber_Binary& subscriber_xrce) override;

    bool create_datawriter_by_ref(
        uint16_t datawriter_id,
        uint16_t publisher_id,
        const std::string& ref) override;

    bool create_datawriter_by_xml(
        uint16_t datawriter_id,
        uint16_t publisher_id,
        const std::string& xml) override;

    bool create_datawriter_by_bin(
        uint16_t datawriter_id,
        uint16_t publisher_id,
        const dds::xrce::OBJK_DataWriter_Binary& datawriter_xrce) override;

    bool create_datareader_by_ref(
        uint16_t datareader_id,
        uint16_t subscriber_id,
        const std::string& ref) override;

    bool create_datareader_by_xml(
        uint16_t datareader_id,
        uint16_t subscriber_id,
        const std::string& xml) override;

    bool create_datareader_by_bin(
        uint16_t datareader_id,
        uint16_t subscriber_id,
        const dds::xrce::OBJK_DataReader_Binary& datareader_xrce) override;

    bool create_requester_by_ref(
        uint16_t requester_id,
        uint16_t participant_id,
        const std::string& ref) override;

    bool create_requester_by_xml(
        uint16_t requester_id,
        uint16_t participant_id,
        const std::string& xml) override;

    bool create_requester_by_bin(
        uint16_t requester_id,
        uint16_t participant_id,
        const dds::xrce::OBJK_Requester_Binary& requester_xrce) override;

    bool create_replier_by_ref(
        uint16_t replier_id,
        uint16_t participant_id,
        const std::string& ref) override;

    bool create_replier_by_xml(
        uint16_t replier_id,
        uint16_t participant_id,
        const std::string& xml) override;

    bool create_replier_by_bin(
        uint16_t replier_id,
        uint16_t participant_id,
        const dds::xrce::OBJK_Replier_Binary& replier_xrce) override;

/**********************************************************************************************************************
 * Delete functions.
 **********************************************************************************************************************/
    bool delete_participant(uint16_t participant_id) override;

    bool delete_topic(uint16_t topic_id) override;

    bool delete_publisher(uint16_t publisher_id) override;

    bool delete_subscriber(uint16_t subscriber_id) override;

    bool delete_datawriter(uint16_t datawriter_id) override;

    bool delete_datareader(uint16_t datareader_id) override;

    bool delete_requester(uint16_t requester_id) override;

    bool delete_replier(uint16_t replier_id) override;

/**********************************************************************************************************************
 * Write/Read functions.
 **********************************************************************************************************************/
    bool write_data(
        uint16_t datawriter_id,
        const std::vector<uint8_t>& data) override;

    bool write_request(
        uint16_t requester_id,
        uint32_t sequence_number,
        const std::vector<uint8_t>& data) override;

    bool write_reply(
        uint16_t replier_id,
        const std::vector<uint8_t>& data) override;

    bool read_data(
        uint16_t datareader_id,
        std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout) override;

    bool read_request(
        uint16_t replier_id,
        std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout) override;

    bool read_reply(
        uint16_t reply_id,
        uint32_t& sequence_number,
        std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout) override;

/**********************************************************************************************************************
 * Matched functions.
 **********************************************************************************************************************/
    bool matched_participant_from_ref(
        uint16_t participant_id,
        int16_t domain_id,
        const std::string& ref) const override;

    bool matched_participant_from_xml(
        uint16_t participant_id,
        int16_t domain_id,
        const std::string& xml) const override;

    bool matched_participant_from_bin(
        uint16_t participant_id,
        int16_t domain_id,
        const dds::xrce::OBJK_DomainParticipant_Binary& participant_xrce ) const override;

    bool matched_topic_from_ref(
        uint16_t topic_id,
        const std::string& ref) const override;

    bool matched_topic_from_xml(
        uint16_t topic_id,
        const std::string& xml) const override;

    bool matched_topic_from_bin(
        uint16_t topic_id,
        const dds::xrce::OBJK_Topic_Binary& topic_xrce) const override;

    bool matched_datawriter_from_ref(
        uint16_t datawriter_id,
        const std::string& ref) const override;

    bool matched_datawriter_from_xml(
        uint16_t datawriter_id,
        const std::string& xml) const override;

    bool matched_datawriter_from_bin(
        uint16_t datawriter_id,
        const dds::xrce::OBJK_DataWriter_Binary& datawriter_xrce) const override;

    bool matched_datareader_from_ref(
        uint16_t datareader_id,
        const std::string& ref) const override;

    bool matched_datareader_from_xml(
        uint16_t datareader_id,
        const std::string& xml) const override;

    bool matched_datareader_from_bin(
        uint16_t datareader_id,
        const dds::xrce::OBJK_DataReader_Binary& datareader_xrce) const override;

    bool matched_requester_from_ref(
        uint16_t participant_id,
        const std::string& ref) const override;

    bool matched_requester_from_xml(
        uint16_t participant_id,
        const std::string& xml) const override;

    bool matched_requester_from_bin(
        uint16_t requester_id,
        const dds::xrce::OBJK_Requester_Binary& requester_xrce) const override;

    bool matched_replier_from_ref(
        uint16_t participant_id,
        const std::string& ref) const override;

    bool matched_replier_from_xml(
        uint16_t participant_id,
        const std::string& xml) const override;

    bool matched_replier_from_bin(
        uint16_t replier_id,
        const dds::xrce::OBJK_Replier_Binary& replier_xrce) const override;


    private:
    const rosidl_message_type_support_t *get_fastrtps_typesupport_handle(const rosidl_message_type_support_t* generic_handle);
    bool get_pubsub_ingredients_by_topic_id(uint16_t id, PubSubIngredients& ingredients);
    bool get_pubsub_ingredients_by_topic_id(const std::string id_str, PubSubIngredients& ingredients);

    char serialized_buffer[MAX_MSG_LENGTH];

    bool rcl_initted = false;
    rcl_context_t rcl_context;

    std::mutex mtex;

    std::unordered_map<uint16_t, std::shared_ptr<rcl_node_t>> participants_;
    std::unordered_map<uint16_t, TopicInfo> topics_;
    std::unordered_map<uint16_t, PubSubInfo<rcl_publisher_t>> datawriters_;
    std::unordered_map<uint16_t, PubSubInfo<rcl_subscription_t>> datareaders_;

    middleware::CallbackFactory& callback_factory_;
};

} // namespace uxr
} // namespace eprosima

#endif // UXR__AGENT__MIDDLEWARE__RMW__MIDDLEWARE_HPP_