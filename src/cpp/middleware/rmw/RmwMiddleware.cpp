#include <uxr/agent/middleware/rmw/RmwMiddleware.hpp>
#include <uxr/agent/middleware/rmw/RosidlTypes.hpp>
#include <uxr/agent/logger/Logger.hpp>
#include <uxr/agent/utils/Conversion.hpp>
#include <uxr/agent/middleware/utils/Callbacks.hpp>

#include <rmw/validate_node_name.h>
#include <fastcdr/FastBuffer.h>
#include <fastcdr/Cdr.h>

#include <typeinfo>
#include <exception>

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
namespace {
    std::mutex g_rcl_context_mtx;
    rcl_context_t g_rcl_context;
    size_t g_rcl_context_refcount = 0;
}
namespace eprosima {
namespace uxr {

    RmwMiddleware::RmwMiddleware()
     : participants_{},
       topics_{},
       datawriters_{},
       datareaders_{},
       callback_factory_(callback_factory_.getInstance())
    {
        std::lock_guard<std::mutex> lock(g_rcl_context_mtx);

        if (g_rcl_context_refcount == 0)
        {
            rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
            RCL_RET_CHECK_UXR_NO_RET(rcl_init_options_init(&init_options, rcutils_get_default_allocator()));

            g_rcl_context = rcl_get_zero_initialized_context();
            RCL_RET_CHECK_UXR_NO_RET(rcl_init(0, NULL, &init_options, &g_rcl_context));
            RCL_RET_CHECK_UXR_NO_RET(rcl_init_options_fini(&init_options));
        }
        ++g_rcl_context_refcount;

        UXR_AGENT_LOG_INFO(
            UXR_DECORATE_GREEN("rmw plugin active"),
            "Universal RMW set as active middleware.", "");
    }


    RmwMiddleware::~RmwMiddleware()
    {
        std::lock_guard<std::mutex> lock(g_rcl_context_mtx);

        // Finalize this client's endpoints and nodes while the context is still valid
        // Holding the lock prevents another client's teardown from shutting the shared context down underneath us
        // The shared_ptr deleters call rcl_*_fini in the correct order (pubs/subs before node)
        {
            std::lock_guard<std::recursive_mutex> map_lock(mtex);
            datawriters_.clear();
            datareaders_.clear();
            participants_.clear();
        }

        if (g_rcl_context_refcount > 0 && --g_rcl_context_refcount == 0)
        {
            rcl_ret_t shutdown_ret = rcl_shutdown(&g_rcl_context);
            if (shutdown_ret != RCL_RET_OK)
            {
                UXR_AGENT_LOG_ERROR(
                    UXR_DECORATE_RED("rmw plugin error"),
                    "rcl_shutdown failed with code " + std::to_string(shutdown_ret), "");
            }
            rcl_ret_t fini_ret = rcl_context_fini(&g_rcl_context);
            if (fini_ret != RCL_RET_OK)
            {
                UXR_AGENT_LOG_ERROR(
                    UXR_DECORATE_RED("rmw plugin error"),
                    "rcl_context_fini failed with code %s", std::to_string(fini_ret).c_str());
            }
        }
    }

    /**********************************************************************************************************************
     * Create functions.
     **********************************************************************************************************************/
    bool RmwMiddleware::create_participant_by_ref(
        uint16_t participant_id,
        int16_t domain_id,
        const std::string& ref)
    {
        std::lock_guard<std::recursive_mutex> lock(mtex);

        // Don't init a second node if this participant id already exists
        if (participants_.find(participant_id) != participants_.end())
        {
            return true;
        }

        auto node_deleter = [](rcl_node_t* n) {
            rcl_ret_t fini_ret = rcl_node_fini(n);
            if (fini_ret != RCL_RET_OK)
            {
                UXR_AGENT_LOG_ERROR(
                    UXR_DECORATE_RED("rmw plugin error"),
                    "rcl_node_fini failed with code " + std::to_string(fini_ret), "");
            }
            delete n;
        };

        std::shared_ptr<rcl_node_t> node(
            new rcl_node_t(rcl_get_zero_initialized_node()), node_deleter);

        std::string part_name = "xrce_participant_" + std::to_string(next_participant_id);
        next_participant_id++;
        int ret;
        size_t invalid_index;
        RCL_RET_CHECK_UXR_RET_FALSE(rmw_validate_node_name(part_name.c_str(), &ret, &invalid_index));
        rcl_node_options_t node_options = rcl_node_get_default_options();
        RCL_RET_CHECK_UXR_RET_FALSE(rcl_node_init(node.get(), part_name.c_str(), "/", &g_rcl_context, &node_options));
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
        std::lock_guard<std::recursive_mutex> lock(mtex);

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
        std::lock_guard<std::recursive_mutex> lock(mtex);

        PubSubIngredients pub_ingredients;
        if(!get_pubsub_ingredients_by_topic_id(ref, pub_ingredients))
        {
            return false;
        }

        const rosidl_message_type_support_t *ts = ROSIDL_TYPES.at(pub_ingredients.topic_type)->get_typesupport_handle();

        // keep the node alive for the publisher's lifetime, and fini the
        // publisher (against that node) before the node is finalized
        std::shared_ptr<rcl_node_t> node = pub_ingredients.node;
        auto pub_deleter = [node](rcl_publisher_t* p) {
            rcl_ret_t fini_ret = rcl_publisher_fini(p, node.get());
            if (fini_ret != RCL_RET_OK)
            {
                UXR_AGENT_LOG_ERROR(
                    UXR_DECORATE_RED("rmw plugin error"),
                    "rcl_publisher_fini failed with code " + std::to_string(fini_ret), "");
            }
            delete p;
        };

        std::shared_ptr<rcl_publisher_t> pub(
            new rcl_publisher_t(rcl_get_zero_initialized_publisher()), pub_deleter);
        rcl_publisher_options_t pub_ops = rcl_publisher_get_default_options();
        RCL_RET_CHECK_UXR_RET_FALSE(rcl_publisher_init(pub.get(), node.get(), ts, pub_ingredients.topic_name.c_str(), &pub_ops));

        PubSubInfo<rcl_publisher_t> pn;
        pn.verified_type_name = pub_ingredients.topic_type;
        pn.node = node;
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
        std::lock_guard<std::recursive_mutex> lock(mtex);

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
        std::lock_guard<std::recursive_mutex> lock(mtex);

        PubSubIngredients sub_ingredients;

        if(!get_pubsub_ingredients_by_topic_id(ref, sub_ingredients))
        {
            return false;
        }

        const rosidl_message_type_support_t *ts = ROSIDL_TYPES.at(sub_ingredients.topic_type)->get_typesupport_handle();

        std::shared_ptr<rcl_node_t> node = sub_ingredients.node;
        auto sub_deleter = [node](rcl_subscription_t* s) {
            rcl_ret_t fini_ret = rcl_subscription_fini(s, node.get());
            if (fini_ret != RCL_RET_OK)
            {
                UXR_AGENT_LOG_ERROR(
                    UXR_DECORATE_RED("rmw plugin error"),
                    "rcl_subscription_fini failed with code " + std::to_string(fini_ret), "");
            }
            delete s;
        };

        std::shared_ptr<rcl_subscription_t> sub(
            new rcl_subscription_t(rcl_get_zero_initialized_subscription()), sub_deleter);
        rcl_subscription_options_t sub_ops = rcl_subscription_get_default_options();
        RCL_RET_CHECK_UXR_RET_FALSE(rcl_subscription_init(sub.get(), node.get(), ts, sub_ingredients.topic_name.c_str(), &sub_ops));

        PubSubInfo<rcl_subscription_t> sn;
        sn.verified_type_name = sub_ingredients.topic_type;
        sn.node = node;
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
        std::lock_guard<std::recursive_mutex> lock(mtex);

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
        std::lock_guard<std::recursive_mutex> lock(mtex);

        auto it = participants_.find(participant_id);
        if(it == participants_.end())
        {
            return false;
        }
        // node is finalized once its pubs/subs have also been released
        participants_.erase(participant_id);
        return true;
    }

    bool RmwMiddleware::delete_topic(uint16_t topic_id)
    {
        std::lock_guard<std::recursive_mutex> lock(mtex);

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
        std::lock_guard<std::recursive_mutex> lock(mtex);

        auto it = datawriters_.find(datawriter_id);
        if(it == datawriters_.end())
        {
            return false;
        }
        datawriters_.erase(datawriter_id);  // deleter calls rcl_publisher_fini
        return true;
    }

    bool RmwMiddleware::delete_datareader(uint16_t datareader_id)
    {
        std::lock_guard<std::recursive_mutex> lock(mtex);

        auto it = datareaders_.find(datareader_id);
        if(it == datareaders_.end())
        {
            return false;
        }
        datareaders_.erase(datareader_id);  // deleter calls rcl_subscription_fini
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
        std::lock_guard<std::recursive_mutex> lock(mtex);

        //find datawriter
        auto it = datawriters_.find(datawriter_id);
        if(it == datawriters_.end())
        {
            return false;
        }

        PubSubInfo<rcl_publisher_t> pn = it->second;
        std::shared_ptr<rcl_publisher_t> pub = pn.t;

        //deserialize data (https://github.com/ros2/rmw_fastrtps/blob/humble/rmw_fastrtps_cpp/src/rmw_serialize.cpp)
        //also look at this: https://github.com/ros2/rmw_fastrtps/blob/humble/rmw_fastrtps_cpp/src/type_support_common.cpp#L118

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
        std::shared_ptr<RosMessageType> msg_info = ROSIDL_TYPES.at(pn.verified_type_name);
        const rosidl_message_type_support_t 
            *generic_typesupport = msg_info->get_typesupport_handle(),
            *fastdds_typesupport = get_fastrtps_typesupport_handle(generic_typesupport);

        // check fastdds-specific type support handle
        std::string vtn = pn.verified_type_name;
        if (!fastdds_typesupport) {
            UXR_AGENT_LOG_ERROR(
                UXR_DECORATE_RED("typesupport error"),
                "" + vtn, "");

            return false;
        }

        // callbacks, includes deserialize function handle
        auto callbacks = static_cast<const message_type_support_callbacks_t *>(fastdds_typesupport->data);

        //populate a buffer with an empty message
        size_t msg_sz;
        void *buf = msg_info->get_empty_as_void_ptr(&msg_sz);

        //de-serialize data into message buffer
        try
        {
            callbacks->cdr_deserialize(deser, buf); //now msg_data contains raw unserialized msg
        }
        catch (const std::exception& e)
        {
            UXR_AGENT_LOG_ERROR(
                UXR_DECORATE_RED("deserialization error"),
                "%s", e.what());
            msg_info->delete_empty(buf);
            return false;
        }

        //publish data (rcl_publish serializes inline, so buf can be freed after it returns)
        rcl_ret_t pub_ret = rcl_publish(pub.get(), buf, nullptr);
        msg_info->delete_empty(buf);

        if (pub_ret != RCL_RET_OK)
        {
            UXR_AGENT_LOG_CRITICAL(
                UXR_DECORATE_RED("rmw plugin error"),
                "rcl_publish failed with code " + std::to_string(pub_ret), "");
            return false;
        }

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
        std::lock_guard<std::recursive_mutex> lock(mtex);

        auto it = datareaders_.find(datareader_id);
        if(it == datareaders_.end())
        {
            return false;
        }

        //get subscription handle
        PubSubInfo<rcl_subscription_t> sn = it->second;
        std::shared_ptr<rcl_subscription_t> sub = sn.t;

        std::shared_ptr<RosMessageType> msg_info = ROSIDL_TYPES.at(sn.verified_type_name);

        //get generic message into buffer
        size_t msg_size;
        void *buf = msg_info->get_empty_as_void_ptr(&msg_size);

        //take msg with rcl
        rmw_message_info_t metadata;
        rcl_ret_t ret = rcl_take(sub.get(), buf, &metadata, nullptr);

        if(ret == RCL_RET_SUBSCRIPTION_TAKE_FAILED)
        {
            msg_info->delete_empty(buf);
            return false;
        }

        if(ret != RCL_RET_OK)
        {
            UXR_AGENT_LOG_CRITICAL(
                UXR_DECORATE_RED("rmw plugin error"),
                "rcl_take failed with code " + std::to_string(ret), "");

            msg_info->delete_empty(buf);

            return false;
        }

        //see comments in write_data for how serialization is handled

        //now serialize the data using fastrtps
        const rosidl_message_type_support_t 
            *generic_typesupport = msg_info->get_typesupport_handle(),
            *fastrtps_typesupport = get_fastrtps_typesupport_handle(generic_typesupport);

        // check fastdds-specific type support handle
        std::string vtn = sn.verified_type_name;
        if (!fastrtps_typesupport) {
            UXR_AGENT_LOG_ERROR(
                UXR_DECORATE_RED("typesupport error"),
                "" + vtn, "");

            msg_info->delete_empty(buf);

            return false;
        }

        auto callbacks = static_cast<const message_type_support_callbacks_t *>(fastrtps_typesupport->data);

        // size the buffer to the actual capacity, not the in-memory struct size,
        // so variable-length messages can't overflow serialized_buffer
        eprosima::fastcdr::FastBuffer fastbuffer(serialized_buffer, sizeof(serialized_buffer));
        eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::Cdr::DDS_CDR);

        try
        {
            callbacks->cdr_serialize(buf, ser);
        }
        catch (const std::exception& e)
        {
            UXR_AGENT_LOG_ERROR(
                UXR_DECORATE_RED("serialization error"),
                "%s", e.what());
            msg_info->delete_empty(buf);
            return false;
        }

        msg_info->delete_empty(buf);

        //pack into data out, using the number of bytes actually written
        data.assign(serialized_buffer, serialized_buffer + ser.getSerializedDataLength());

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
        return get_message_typesupport_handle(
            generic_handle, rosidl_typesupport_fastrtps_cpp::typesupport_identifier);
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