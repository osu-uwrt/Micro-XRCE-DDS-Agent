#include <uxr/agent/middleware/rmw/RmwMiddleware.hpp>
#include <uxr/agent/middleware/rmw/RosidlTypes.hpp>
#include <uxr/agent/logger/Logger.hpp>
#include <uxr/agent/utils/Conversion.hpp>
#include <uxr/agent/middleware/utils/Callbacks.hpp>

#include <rmw/validate_node_name.h>
#include <fastcdr/FastBuffer.h>
#include <fastcdr/Cdr.h>

#include <rosidl_typesupport_fastrtps_cpp/identifier.hpp>

#define RCL_RET_CHECK_UXR(expr, ret) \
    do \
    { \
        if(rcl_ret_t __ret = expr != RCL_RET_OK) \
        { \
            UXR_AGENT_LOG_CRITICAL( \
                UXR_DECORATE_RED("rmw plugin error"), \
                #expr " failed with code " + std::to_string(__ret), ""); \
            return ret; \
        } \
    } while(0)

#define RCL_RET_CHECK_UXR_NO_RET(expr) RCL_RET_CHECK_UXR(expr,)
#define RCL_RET_CHECK_UXR_RET_FALSE(expr) RCL_RET_CHECK_UXR(expr, false)

namespace eprosima {
namespace uxr {

    RmwMiddleware::RmwMiddleware()
     : callback_factory_(callback_factory_.getInstance()),
       participants_{},
       topics_{},
       datawriters_{},
       datareaders_{}
    {
        rcl_init_options_t init_options;
        RCL_RET_CHECK_UXR_NO_RET(rcl_init_options_init(&init_options, rcutils_get_default_allocator()));

        rcl_context = rcl_get_zero_initialized_context();
        RCL_RET_CHECK_UXR_NO_RET(rcl_init(0, NULL, &init_options, &rcl_context));
        RCL_RET_CHECK_UXR_NO_RET(rcl_init_options_fini(&init_options));
        
        UXR_AGENT_LOG_INFO(
            UXR_DECORATE_GREEN("rmw plugin active"),
            "Universal RMW set as active middleware.", "");
    }


    RmwMiddleware::~RmwMiddleware()
    {
        RCL_RET_CHECK_UXR_NO_RET(rcl_shutdown(&rcl_context));
        Middleware::~Middleware();
    }

    /**********************************************************************************************************************
     * Create functions.
     **********************************************************************************************************************/
    bool RmwMiddleware::create_participant_by_ref(
        uint16_t participant_id,
        int16_t domain_id,
        const std::string& ref)
    {
        //referenced from here: https://github.com/ros2/rclcpp/blob/humble/rclcpp/src/rclcpp/node_interfaces/node_base.cpp
        std::shared_ptr<rcl_node_t> node(new rcl_node_t(rcl_get_zero_initialized_node()));
        std::string part_name = "xrce_participant_" + std::to_string(participant_id);
        int ret;
        size_t invalid_index;
        RCL_RET_CHECK_UXR_RET_FALSE(rmw_validate_node_name(part_name.c_str(), &ret, &invalid_index));
        rcl_node_options_t node_options = rcl_node_get_default_options();
        RCL_RET_CHECK_UXR_RET_FALSE(rcl_node_init(node.get(), part_name.c_str(), "/", &rcl_context, &node_options));
        participants_.insert({participant_id, node});
        return true;
    }

    bool RmwMiddleware::create_participant_by_xml(
        uint16_t participant_id,
        int16_t domain_id,
        const std::string& xml)
    {
        return create_participant_by_ref(participant_id, domain_id, "");
    }

    bool RmwMiddleware::create_participant_by_bin(
        uint16_t participant_id,
        const dds::xrce::OBJK_DomainParticipant_Binary& participant_xrce)
    {
        return create_participant_by_ref(participant_id, participant_xrce.domain_id(), participant_xrce.domain_referente());
    }

    bool RmwMiddleware::create_topic_by_ref(
        uint16_t topic_id,
        uint16_t participant_id,
        const std::string& ref)
    {
        return false;
    }

    bool RmwMiddleware::create_topic_by_xml(
        uint16_t topic_id,
        uint16_t participant_id,
        const std::string& xml)
    {
        return false;
    }

    bool RmwMiddleware::create_topic_by_bin(
        uint16_t topic_id,
        uint16_t participant_id,
        const dds::xrce::OBJK_Topic_Binary& topic_xrce)
    {
        if(topics_.find(topic_id) == topics_.end())
        {
            TopicInfo tinfo;
            tinfo.participant_id = participant_id;
            tinfo.topic_name = topic_xrce.topic_name();
            tinfo.topic_type = topic_xrce.type_name();

            topics_.insert({topic_id, tinfo});
            return true;
        }

        return false;
    }

    bool RmwMiddleware::create_publisher_by_xml(
        uint16_t publisher_id,
        uint16_t participant_id,
        const std::string&)
    {
        return true;
    }

    bool RmwMiddleware::create_publisher_by_bin(
        uint16_t publisher_id,
        uint16_t participant_id,
        const dds::xrce::OBJK_Publisher_Binary& publisher_xrce)
    {
        return create_publisher_by_xml(publisher_id, participant_id, "");
    }

    bool RmwMiddleware::create_subscriber_by_xml(
        uint16_t subscriber_id,
        uint16_t participant_id,
        const std::string&)
    {
        return true;
    }

    bool RmwMiddleware::create_subscriber_by_bin(
        uint16_t subscriber_id,
        uint16_t participant_id,
        const dds::xrce::OBJK_Subscriber_Binary& subscriber_xrce)
    {
        return create_subscriber_by_xml(subscriber_id, participant_id, "");
    }

    bool RmwMiddleware::create_datawriter_by_ref(
        uint16_t datawriter_id,
        uint16_t publisher_id,
        const std::string& ref)
    {
        PubSubIngredients pub_ingredients;
        if(!get_pubsub_ingredients_by_topic_id(ref, pub_ingredients))
        {
            return false;
        }

        const rosidl_message_type_support_t *ts = ROSIDL_TYPES[pub_ingredients.topic_type]->get_typesupport_handle();

        //now create publisher
        std::shared_ptr<rcl_publisher_t> pub(new rcl_publisher_t(rcl_get_zero_initialized_publisher()));
        rcl_publisher_options_t pub_ops = rcl_publisher_get_default_options();
        RCL_RET_CHECK_UXR_RET_FALSE(rcl_publisher_init(pub.get(), pub_ingredients.node.get(), ts, pub_ingredients.topic_name.c_str(), &pub_ops));

        //if we get here then init succeeded, add publisher to map
        PubSubInfo<rcl_publisher_t> pn;
        pn.verified_type_name = pub_ingredients.topic_type;
        pn.node = pub_ingredients.node;
        pn.t = pub;
        datawriters_.insert({datawriter_id, pn});
        return true;
    }

    bool RmwMiddleware::create_datawriter_by_xml(
        uint16_t datawriter_id,
        uint16_t publisher_id,
        const std::string& xml)
    {
        return create_datawriter_by_ref(datawriter_id, publisher_id, xml);
    }

    bool RmwMiddleware::create_datawriter_by_bin(
        uint16_t datawriter_id,
        uint16_t publisher_id,
        const dds::xrce::OBJK_DataWriter_Binary& datawriter_xrce)
    {
        //look up topic
        uint16_t topic_id = conversion::objectid_to_raw(datawriter_xrce.topic_id());
        auto it = topics_.find(topic_id);
        if(it != topics_.end())
        {
            return create_datawriter_by_ref(datawriter_id, publisher_id, std::to_string(topic_id));
        }

        return false;
    }

    bool RmwMiddleware::create_datareader_by_ref(
        uint16_t datareader_id,
        uint16_t subscriber_id,
        const std::string& ref)
    {
        PubSubIngredients sub_ingredients;

        if(!get_pubsub_ingredients_by_topic_id(ref, sub_ingredients))
        {
            return false;
        }

        const rosidl_message_type_support_t *ts = ROSIDL_TYPES[sub_ingredients.topic_type]->get_typesupport_handle();

        //now create subscription
        std::shared_ptr<rcl_subscription_t> sub(new rcl_subscription_t(rcl_get_zero_initialized_subscription()));
        rcl_subscription_options_t sub_ops = rcl_subscription_get_default_options();
        RCL_RET_CHECK_UXR_RET_FALSE(rcl_subscription_init(sub.get(), sub_ingredients.node.get(), ts, sub_ingredients.topic_name.c_str(), &sub_ops));
        
        //if we got here then init succeeded, so add to datareaders
        PubSubInfo<rcl_subscription_t> sn;
        sn.verified_type_name = sub_ingredients.topic_type;
        sn.node = sub_ingredients.node;
        sn.t = sub;
        datareaders_.insert({datareader_id, sn});
        return true;
    }

    bool RmwMiddleware::create_datareader_by_xml(
        uint16_t datareader_id,
        uint16_t subscriber_id,
        const std::string& xml)
    {
        return create_datareader_by_ref(datareader_id, subscriber_id, xml);
    }

    bool RmwMiddleware::create_datareader_by_bin(
        uint16_t datareader_id,
        uint16_t subscriber_id,
        const dds::xrce::OBJK_DataReader_Binary& datareader_xrce)
    {
        //look up topic
        uint16_t topic_id = conversion::objectid_to_raw(datareader_xrce.topic_id());
        auto it = topics_.find(topic_id);
        if(it != topics_.end())
        {
            return create_datareader_by_ref(datareader_id, subscriber_id, std::to_string(topic_id));
        }

        return false;
    }

    bool RmwMiddleware::create_requester_by_ref(
        uint16_t requester_id,
        uint16_t participant_id,
        const std::string& ref)
    {
        std::cout << "req ref" << std::endl;
        return true;
    }

    bool RmwMiddleware::create_requester_by_xml(
        uint16_t requester_id,
        uint16_t participant_id,
        const std::string& xml)
    {
        std::cout << "req xml" << std::endl;
        return true;
    }

    bool RmwMiddleware::create_requester_by_bin(
        uint16_t requester_id,
        uint16_t participant_id,
        const dds::xrce::OBJK_Requester_Binary& requester_xrce)
    {
        std::cout << "req bin" << std::endl;
        return true;
    }

    bool RmwMiddleware::create_replier_by_ref(
        uint16_t replier_id,
        uint16_t participant_id,
        const std::string& ref)
    {
        std::cout << "rep ref" << std::endl;
        return true;
    }

    bool RmwMiddleware::create_replier_by_xml(
        uint16_t replier_id,
        uint16_t participant_id,
        const std::string& xml)
    {
        std::cout << "rep xml" << std::endl;
        return true;
    }

    bool RmwMiddleware::create_replier_by_bin(
        uint16_t replier_id,
        uint16_t participant_id,
        const dds::xrce::OBJK_Replier_Binary& replier_xrce)
    {
        std::cout << "rep bin" << std::endl;
        return true;
    }

    /**********************************************************************************************************************
     * Delete functions.
     **********************************************************************************************************************/
    bool RmwMiddleware::delete_participant(uint16_t participant_id)
    {
        auto it = participants_.find(participant_id);
        if(it == participants_.end())
        {
            return false;
        }

        //destroy node
        std::shared_ptr<rcl_node_t> node = it->second;
        RCL_RET_CHECK_UXR_RET_FALSE(rcl_node_fini(node.get()));

        //if we get here then destroy good, remove from map
        participants_.erase(participant_id);

        return true;
    }

    bool RmwMiddleware::delete_topic(uint16_t topic_id)
    {
        auto it = topics_.find(topic_id);
        if(it == topics_.end())
        {
            return false;
        }

        //no destroying needed, just remove from map
        topics_.erase(topic_id);

        return true;
    }

    bool RmwMiddleware::delete_publisher(uint16_t publisher_id)
    {
        //no xrce publishers (rcl pubs are datawriters) so nothing to do here
        return true;
    }

    bool RmwMiddleware::delete_subscriber(uint16_t subscriber_id)
    {
        //ditto ^ ^ (see delete_publisher)
        return true;
    }

    bool RmwMiddleware::delete_datawriter(uint16_t datawriter_id)
    {
        auto it = datawriters_.find(datawriter_id);
        if(it == datawriters_.end())
        {
            return false;
        }

        //destroy publisher
        PubSubInfo<rcl_publisher_t> pn = it->second;
        RCL_RET_CHECK_UXR_RET_FALSE(rcl_publisher_fini(pn.t.get(), pn.node.get()));

        //if we get here, then remove from the map
        datawriters_.erase(datawriter_id);

        return true;
    }

    bool RmwMiddleware::delete_datareader(uint16_t datareader_id)
    {
        auto it = datareaders_.find(datareader_id);
        if(it == datareaders_.end())
        {
            return false;
        }

        //destroy subscription
        PubSubInfo<rcl_subscription_t> sn = it->second;
        RCL_RET_CHECK_UXR_RET_FALSE(rcl_subscription_fini(sn.t.get(), sn.node.get()));

        //if we get here, then remove from the map
        datareaders_.erase(datareader_id);

        return true;
    }

    bool RmwMiddleware::delete_requester(uint16_t requester_id)
    {
        std::cout << "del req" << std::endl;
        return true;
    }

    bool RmwMiddleware::delete_replier(uint16_t replier_id)
    {
        std::cout << "del req" << std::endl;
        return true;
    }

    /**********************************************************************************************************************
     * Write/Read functions.
     **********************************************************************************************************************/
    bool RmwMiddleware::write_data(
        uint16_t datawriter_id,
        const std::vector<uint8_t>& data)
    {
        //find datawriter
        auto it = datawriters_.find(datawriter_id);
        if(it == datawriters_.end())
        {
            return false;
        }

        PubSubInfo<rcl_publisher_t> pn = it->second;
        std::shared_ptr<rcl_publisher_t> pub = pn.t;

        //deserialize data (https://github.com/ros2/rmw_fastrtps/blob/humble/rmw_fastrtps_cpp/src/rmw_serialize.cpp)
        //also look at this: https://github.com/ros2/rmw_fastrtps/blob/rolling/rmw_fastrtps_cpp/src/type_support_common.cpp#L118

        if(data.size() > sizeof(serialized_buffer))
        {
            UXR_AGENT_LOG_ERROR(
                UXR_DECORATE_RED("deserialization error"),
                "Message too large: " + std::to_string(data.size()) + " > " + std::to_string(sizeof(serialized_buffer)), "");
            
            return false;
        }

        memcpy(serialized_buffer, data.data(), data.size());

        eprosima::fastcdr::FastBuffer fastbuffer(serialized_buffer, data.size());
        eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::Cdr::DDS_CDR);
        
        // get generic type support handle
        std::shared_ptr<RosMessageType> msg_info = ROSIDL_TYPES[pn.verified_type_name];
        const rosidl_message_type_support_t 
            *generic_typesupport = msg_info->get_typesupport_handle(),
            *fastdds_typesupport = get_fastrtps_typesupport_handle(generic_typesupport);

        // check fastdds-specific type support handle
        if (!fastdds_typesupport) {
            UXR_AGENT_LOG_ERROR(
                UXR_DECORATE_RED("typesupport error"),
                "Typesupport is not supported.", "");

            return false;
        }

        // callbacks, includes deserialize function handle
        auto callbacks = static_cast<const message_type_support_callbacks_t *>(fastdds_typesupport->data);

        //populate a buffer with an empty message
        size_t msg_sz = msg_info->get_empty_as_void_ptr(unserialized_buffer, sizeof(unserialized_buffer));
        
        if(msg_sz == 0)
        {
            UXR_AGENT_LOG_ERROR(
                UXR_DECORATE_RED("message error"),
                "Message too large (" + std::to_string(msg_sz) + " > " + std::to_string(sizeof(unserialized_buffer)), "");
            
            return false;
        }

        //de-serialize data into message buffer
        callbacks->cdr_deserialize(deser, static_cast<void *>(unserialized_buffer)); //now msg_data contains raw unserialized msg

        //publish data
        RCL_RET_CHECK_UXR_RET_FALSE(rcl_publish(pub.get(), unserialized_buffer, nullptr));

        return true;
    }

    bool RmwMiddleware::write_request(
        uint16_t requester_id,
        uint32_t sequence_number,
        const std::vector<uint8_t>& data)
    {
        std::cout << "wr req" << std::endl;
        return true;
    }

    bool RmwMiddleware::write_reply(
        uint16_t replier_id,
        const std::vector<uint8_t>& data)
    {
        std::cout << "wr rep" << std::endl;
        return true;
    }

    bool RmwMiddleware::read_data(
        uint16_t datareader_id,
        std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout)
    {
        auto it = datareaders_.find(datareader_id);
        if(it == datareaders_.end())
        {
            return false;
        }

        //get subscription handle
        PubSubInfo<rcl_subscription_t> sn = it->second;
        std::shared_ptr<rcl_subscription_t> sub = sn.t;

        std::shared_ptr<RosMessageType> msg_info = ROSIDL_TYPES[sn.verified_type_name];

        //get generic message into buffer
        size_t msg_size = msg_info->get_empty_as_void_ptr(unserialized_buffer, sizeof(unserialized_buffer));
        if(msg_size == 0)
        {
            UXR_AGENT_LOG_ERROR(
                UXR_DECORATE_RED("message error"),
                "Message too large (" + std::to_string(msg_size) + " > " + std::to_string(sizeof(unserialized_buffer)), "");
            
            return false;
        }

        //take msg with rcl
        rmw_message_info_t metadata;
        rcl_ret_t ret = rcl_take(sub.get(), unserialized_buffer, &metadata, nullptr);

        if(ret == RCL_RET_SUBSCRIPTION_TAKE_FAILED)
        {
            return false;
        }

        if(ret != RCL_RET_OK)
        {
            UXR_AGENT_LOG_CRITICAL(
                UXR_DECORATE_RED("rmw plugin error"),
                "rcl_take failed with code " + std::to_string(ret), "");
            
            return false;
        }

        //now serialize the data using fastrtps
        const rosidl_message_type_support_t 
            *generic_typesupport = msg_info->get_typesupport_handle(),
            *fastrtps_typesupport = get_fastrtps_typesupport_handle(generic_typesupport);

        // check fastdds-specific type support handle
        if (!fastrtps_typesupport) {
            UXR_AGENT_LOG_ERROR(
                UXR_DECORATE_RED("typesupport error"),
                "Typesupport is not supported.", "");

            return false;
        }

        auto callbacks = static_cast<const message_type_support_callbacks_t *>(fastrtps_typesupport->data);

        eprosima::fastcdr::FastBuffer fastbuffer(serialized_buffer, msg_size);
        eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::Cdr::DDS_CDR);
        
        callbacks->cdr_serialize(static_cast<void *>(unserialized_buffer), deser);

        //pack into data out
        data.assign(fastbuffer.getBuffer(), fastbuffer.getBuffer() + fastbuffer.getBufferSize());

        return true;
    }

    bool RmwMiddleware::read_request(
        uint16_t replier_id,
        std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout)
    {
        std::cout << "r req" << std::endl;
        return true;
    }

    bool RmwMiddleware::read_reply(
        uint16_t reply_id,
        uint32_t& sequence_number,
        std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout)
    {
        std::cout << "r rep" << std::endl;
        return true;
    }

    /**********************************************************************************************************************
     * Matched functions.
     **********************************************************************************************************************/
    bool RmwMiddleware::matched_participant_from_ref(
        uint16_t participant_id,
        int16_t domain_id,
        const std::string& ref) const
    {
        std::cout << "mat part ref" << std::endl;
        return true;
    }

    bool RmwMiddleware::matched_participant_from_xml(
        uint16_t participant_id,
        int16_t domain_id,
        const std::string& xml) const
    {
        std::cout << "mat part xml" << std::endl;
        return true;
    }

    bool RmwMiddleware::matched_participant_from_bin(
        uint16_t participant_id,
        int16_t domain_id,
        const dds::xrce::OBJK_DomainParticipant_Binary& participant_xrce ) const
    {
        std::cout << "mat part bin" << std::endl;
        return true;
    }

    bool RmwMiddleware::matched_topic_from_ref(
        uint16_t topic_id,
        const std::string& ref) const
    {
        std::cout << "mat top ref" << std::endl;
        return true;
    }

    bool RmwMiddleware::matched_topic_from_xml(
        uint16_t topic_id,
        const std::string& xml) const
    {
        std::cout << "mat top xml" << std::endl;
        return true;
    }

    bool RmwMiddleware::matched_topic_from_bin(
        uint16_t topic_id,
        const dds::xrce::OBJK_Topic_Binary& topic_xrce) const
    {
        std::cout << "mat top bin" << std::endl;
        return true;
    }

    bool RmwMiddleware::matched_datawriter_from_ref(
        uint16_t datawriter_id,
        const std::string& ref) const
    {
        std::cout << "mat dw ref" << std::endl;
        return true;
    }

    bool RmwMiddleware::matched_datawriter_from_xml(
        uint16_t datawriter_id,
        const std::string& xml) const
    {
        std::cout << "mat dw xml" << std::endl;
        return true;
    }

    bool RmwMiddleware::matched_datawriter_from_bin(
        uint16_t datawriter_id,
        const dds::xrce::OBJK_DataWriter_Binary& datawriter_xrce) const
    {
        std::cout << "mat dw bin" << std::endl;
        return true;
    }

    bool RmwMiddleware::matched_datareader_from_ref(
        uint16_t datareader_id,
        const std::string& ref) const
    {
        std::cout << "mat dr ref" << std::endl;
        return true;
    }

    bool RmwMiddleware::matched_datareader_from_xml(
        uint16_t datareader_id,
        const std::string& xml) const
    {
        std::cout << "mat dr xml" << std::endl;
        return true;
    }

    bool RmwMiddleware::matched_datareader_from_bin(
        uint16_t datareader_id,
        const dds::xrce::OBJK_DataReader_Binary& datareader_xrce) const
    {
        std::cout << "mat dr bin" << std::endl;
        return true;
    }

    bool RmwMiddleware::matched_requester_from_ref(
        uint16_t participant_id,
        const std::string& ref) const
    {
        std::cout << "mat req ref" << std::endl;
        return true;
    }

    bool RmwMiddleware::matched_requester_from_xml(
        uint16_t participant_id,
        const std::string& xml) const
    {
        std::cout << "mat req xml" << std::endl;
        return true;
    }

    bool RmwMiddleware::matched_requester_from_bin(
        uint16_t requester_id,
        const dds::xrce::OBJK_Requester_Binary& requester_xrce) const
    {
        std::cout << "mat req bin" << std::endl;
        return true;
    }

    bool RmwMiddleware::matched_replier_from_ref(
        uint16_t participant_id,
        const std::string& ref) const
    {
        std::cout << "mat rep ref" << std::endl;
        return true;
    }

    bool RmwMiddleware::matched_replier_from_xml(
        uint16_t participant_id,
        const std::string& xml) const
    {
        std::cout << "mat rep xml" << std::endl;
        return true;
    }

    bool RmwMiddleware::matched_replier_from_bin(
        uint16_t replier_id,
        const dds::xrce::OBJK_Replier_Binary& replier_xrce) const
    {
        std::cout << "mat rep bin" << std::endl;
        return true;
    }

    const rosidl_message_type_support_t *RmwMiddleware::get_fastrtps_typesupport_handle(const rosidl_message_type_support_t* generic_handle)
    {
        const rosidl_message_type_support_t * fastdds_typesupport = get_message_typesupport_handle(
            generic_handle, rosidl_typesupport_fastrtps_cpp::typesupport_identifier);

        if (!fastdds_typesupport) {
            fastdds_typesupport = get_message_typesupport_handle(
                generic_handle, rosidl_typesupport_fastrtps_cpp::typesupport_identifier);

            return nullptr;
        }
    }

    bool RmwMiddleware::get_pubsub_ingredients_by_topic_id(uint16_t id, PubSubIngredients& ingredients)
    {
        auto topicit = topics_.find(id);
        if(topicit == topics_.end())
        {
            return false;
        }

        TopicInfo tinfo = topicit->second;

        //remove "rt" from ref string to derive topic name
        ingredients.topic_name = tinfo.topic_name;
        if(ingredients.topic_name.find("rt") == 0)
        {
            ingredients.topic_name = ingredients.topic_name.substr(2);
        }

        //validate message type support handle
        auto tsit = ROSIDL_TYPES.find(tinfo.topic_type);
        if(tsit == ROSIDL_TYPES.end())
        {
            UXR_AGENT_LOG_ERROR(
                UXR_DECORATE_RED("type error"),
                "Message type " + tinfo.topic_type + " is not supported.", "");

            return false;
        }

        //ros message type exists, populate ingredients
        ingredients.topic_type = tinfo.topic_type;

        //find ros node
        uint16_t participant_id = tinfo.participant_id;
        
        auto partit = participants_.find(participant_id);
        if(partit == participants_.end())
        {
            return false;
        }

        ingredients.node = partit->second;

        return true;
    }

    bool RmwMiddleware::get_pubsub_ingredients_by_topic_id(const std::string id_str, PubSubIngredients& ingredients)
    {
        //figure out the topic id and call the other overload
        uint16_t topic_id = 0;
        try
        {
            topic_id = (uint16_t) std::stoi(id_str);
        } catch(std::invalid_argument& e)
        {
            return false;
        }

        return get_pubsub_ingredients_by_topic_id(topic_id, ingredients);
    }

} // namespace eprosima
} // namespace uxr
