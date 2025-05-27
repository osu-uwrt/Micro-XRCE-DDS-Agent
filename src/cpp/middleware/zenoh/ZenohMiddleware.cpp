#include <uxr/agent/middleware/zenoh/ZenohMiddleware.hpp>
#include <uxr/agent/logger/Logger.hpp>
#include <uxr/agent/utils/Conversion.hpp>
#include <uxr/agent/middleware/utils/Callbacks.hpp>

namespace eprosima {
namespace uxr {

    ZenohMiddleware::ZenohMiddleware()
     : callback_factory_(callback_factory_.getInstance()),
       participants_{},
       topics_{},
       publishers_{},
       subscribers_{},
       datawriters_{},
       datareaders_{}
    {
        UXR_AGENT_LOG_INFO(
            UXR_DECORATE_GREEN("zenoh plugin active"),
            "Zenoh set as active middleware.", "");
    }

    /**********************************************************************************************************************
     * Create functions.
     **********************************************************************************************************************/
    bool ZenohMiddleware::create_participant_by_ref(
        uint16_t participant_id,
        int16_t domain_id,
        const std::string& ref)
    {
        auto it = participants_.find(participant_id);
        if(it == participants_.end())
        {
            try
            {
                // zenoh::Config zconf = zenoh::Config::create_default(); //TODO: load from file using env var
                zenoh::Config zconf = zenoh::Config::from_file("/home/brach/zenohcfg/DEFAULT_RMW_ZENOH_SESSION_CONFIG.json5");
                participants_.emplace( participant_id, std::make_shared<zenoh::Session>(std::move(zconf)) );
                return true;
            } catch(zenoh::ZException& e)
            {
                UXR_AGENT_LOG_CRITICAL(
                    UXR_DECORATE_RED("Zenoh init error"),
                    "what(): " + std::string(e.what()) +
                    ", participant_id: " + std::to_string(participant_id), 0);
            }
        }

        return false;
    }

    bool ZenohMiddleware::create_participant_by_xml(
        uint16_t participant_id,
        int16_t domain_id,
        const std::string& xml)
    {
        return create_participant_by_ref(participant_id, domain_id, "");
    }

    bool ZenohMiddleware::create_participant_by_bin(
        uint16_t participant_id,
        const dds::xrce::OBJK_DomainParticipant_Binary& participant_xrce)
    {
        return create_participant_by_ref(participant_id, participant_xrce.domain_id(), "");
    }

    bool ZenohMiddleware::create_topic_by_ref(
        uint16_t topic_id,
        uint16_t participant_id,
        const std::string& ref)
    {
        auto it = topics_.find(topic_id);
        if(it == topics_.end())
        {
            topics_.emplace(topic_id, ref);
            return true;
        }

        return false;
    }

    bool ZenohMiddleware::create_topic_by_xml(
        uint16_t topic_id,
        uint16_t participant_id,
        const std::string& xml)
    {
        return create_topic_by_ref(topic_id, participant_id, xml);
    }

    bool ZenohMiddleware::create_topic_by_bin(
        uint16_t topic_id,
        uint16_t participant_id,
        const dds::xrce::OBJK_Topic_Binary& topic_xrce)
    {
        return create_topic_by_ref(topic_id, participant_id, topic_xrce.topic_name());
    }

    bool ZenohMiddleware::create_publisher_by_xml(
        uint16_t publisher_id,
        uint16_t participant_id,
        const std::string&)
    {
        auto it = publishers_.find(publisher_id);
        if(it == publishers_.end())
        {
            publishers_.emplace(publisher_id, participant_id);
            return true;
        }

        return false;
    }

    bool ZenohMiddleware::create_publisher_by_bin(
        uint16_t publisher_id,
        uint16_t participant_id,
        const dds::xrce::OBJK_Publisher_Binary& publisher_xrce)
    {
        std::cout << "publisher by bin with name " << publisher_xrce.publisher_name() << std::endl;
        return create_publisher_by_xml(publisher_id, participant_id, "");
    }

    bool ZenohMiddleware::create_subscriber_by_xml(
        uint16_t subscriber_id,
        uint16_t participant_id,
        const std::string&)
    {
        auto it = subscribers_.find(subscriber_id);
        if(it == subscribers_.end())
        {
            subscribers_.emplace(subscriber_id, participant_id);
            return true;
        }

        return false;
    }

    bool ZenohMiddleware::create_subscriber_by_bin(
        uint16_t subscriber_id,
        uint16_t participant_id,
        const dds::xrce::OBJK_Subscriber_Binary& subscriber_xrce)
    {
        return create_subscriber_by_xml(subscriber_id, participant_id, "");
    }

    bool ZenohMiddleware::create_datawriter_by_ref(
        uint16_t datawriter_id,
        uint16_t publisher_id,
        const std::string& ref)
    {
        //make sure publisher exists and look up participant id
        auto pubit = publishers_.find(publisher_id);
        if(pubit == publishers_.end())
        {
            return false;
        }

        uint16_t participant_id = pubit->second;

        //make sure participant exists
        auto partit = participants_.find(participant_id);
        if(partit == participants_.end())
        {
            return false;
        }

        std::shared_ptr<zenoh::Session> sess = partit->second;

        //now create datawriter if it does not already exist
        auto dwit = datawriters_.find(datawriter_id);
        if(dwit == datawriters_.end())
        {
            datawriters_.emplace(datawriter_id, sess->declare_publisher(zenoh::KeyExpr(ref)));
            return true;
        }

        return false;
    }

    bool ZenohMiddleware::create_datawriter_by_xml(
        uint16_t datawriter_id,
        uint16_t publisher_id,
        const std::string& xml)
    {
        return create_datawriter_by_ref(datawriter_id, publisher_id, xml);
    }

    bool ZenohMiddleware::create_datawriter_by_bin(
        uint16_t datawriter_id,
        uint16_t publisher_id,
        const dds::xrce::OBJK_DataWriter_Binary& datawriter_xrce)
    {
        //look up topic
        uint16_t topic_id = conversion::objectid_to_raw(datawriter_xrce.topic_id());
        auto it = topics_.find(topic_id);
        if(it != topics_.end())
        {
            return create_datawriter_by_ref(datawriter_id, publisher_id, topics_[topic_id]);
        }

        return false;
    }

    bool ZenohMiddleware::create_datareader_by_ref(
        uint16_t datareader_id,
        uint16_t subscriber_id,
        const std::string& ref)
    {
        std::cout << "reader to topic " << ref << std::endl;
        //make sure publisher exists and look up participant id
        auto subit = subscribers_.find(subscriber_id);
        if(subit == subscribers_.end())
        {
            return false;
        }

        uint16_t participant_id = subit->second;

        //make sure participant exists
        auto partit = participants_.find(participant_id);
        if(partit == participants_.end())
        {
            return false;
        }

        std::shared_ptr<zenoh::Session> sess = partit->second;

        //now create datawriter if it does not already exist
        auto drit = datareaders_.find(datareader_id);
        if(drit == datareaders_.end())
        {
            datareaders_.emplace(datareader_id, sess->declare_subscriber(
                zenoh::KeyExpr(ref), 
                [](const zenoh::Sample& sample) {
                    std::cout << "Received: " << sample.get_payload().as_string() << std::endl;
                },
                zenoh::closures::none));

            return true;
        }

        return false;
    }

    bool ZenohMiddleware::create_datareader_by_xml(
        uint16_t datareader_id,
        uint16_t subscriber_id,
        const std::string& xml)
    {
        return create_datareader_by_ref(datareader_id, subscriber_id, xml);
    }

    bool ZenohMiddleware::create_datareader_by_bin(
        uint16_t datareader_id,
        uint16_t subscriber_id,
        const dds::xrce::OBJK_DataReader_Binary& datareader_xrce)
    {
        //look up topic
        uint16_t topic_id = conversion::objectid_to_raw(datareader_xrce.topic_id());
        auto it = topics_.find(topic_id);
        if(it != topics_.end())
        {
            return create_datareader_by_ref(datareader_id, subscriber_id, topics_[topic_id]);
        }

        return false;
    }

    bool ZenohMiddleware::create_requester_by_ref(
        uint16_t requester_id,
        uint16_t participant_id,
        const std::string& ref)
    {
        std::cout << "req ref" << std::endl;
        return true;
    }

    bool ZenohMiddleware::create_requester_by_xml(
        uint16_t requester_id,
        uint16_t participant_id,
        const std::string& xml)
    {
        std::cout << "req xml" << std::endl;
        return true;
    }

    bool ZenohMiddleware::create_requester_by_bin(
        uint16_t requester_id,
        uint16_t participant_id,
        const dds::xrce::OBJK_Requester_Binary& requester_xrce)
    {
        std::cout << "req bin" << std::endl;
        return true;
    }

    bool ZenohMiddleware::create_replier_by_ref(
        uint16_t replier_id,
        uint16_t participant_id,
        const std::string& ref)
    {
        std::cout << "rep ref" << std::endl;
        return true;
    }

    bool ZenohMiddleware::create_replier_by_xml(
        uint16_t replier_id,
        uint16_t participant_id,
        const std::string& xml)
    {
        std::cout << "rep xml" << std::endl;
        return true;
    }

    bool ZenohMiddleware::create_replier_by_bin(
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
    bool ZenohMiddleware::delete_participant(uint16_t participant_id)
    {
        std::cout << "del part" << std::endl;

        auto it = participants_.find(participant_id);
        if(it != participants_.end())
        {
            participants_.erase(participant_id);
            return true;
        }

        return false;
    }

    bool ZenohMiddleware::delete_topic(uint16_t topic_id)
    {
        std::cout << "del top" << std::endl;
        return true;
    }

    bool ZenohMiddleware::delete_publisher(uint16_t publisher_id)
    {
        std::cout << "del pub" << std::endl;
        return true;
    }

    bool ZenohMiddleware::delete_subscriber(uint16_t subscriber_id)
    {
        std::cout << "del sub" << std::endl;
        return true;
    }

    bool ZenohMiddleware::delete_datawriter(uint16_t datawriter_id)
    {
        std::cout << "del dw" << std::endl;
        return true;
    }

    bool ZenohMiddleware::delete_datareader(uint16_t datareader_id)
    {
        std::cout << "del dr" << std::endl;
        return true;
    }

    bool ZenohMiddleware::delete_requester(uint16_t requester_id)
    {
        std::cout << "del req" << std::endl;
        return true;
    }

    bool ZenohMiddleware::delete_replier(uint16_t replier_id)
    {
        std::cout << "del req" << std::endl;
        return true;
    }

    /**********************************************************************************************************************
     * Write/Read functions.
     **********************************************************************************************************************/
    bool ZenohMiddleware::write_data(
        uint16_t datawriter_id,
        const std::vector<uint8_t>& data)
    {
        std::cout << "wr dat" << std::endl;
        return true;
    }

    bool ZenohMiddleware::write_request(
        uint16_t requester_id,
        uint32_t sequence_number,
        const std::vector<uint8_t>& data)
    {
        std::cout << "wr req" << std::endl;
        return true;
    }

    bool ZenohMiddleware::write_reply(
        uint16_t replier_id,
        const std::vector<uint8_t>& data)
    {
        std::cout << "wr rep" << std::endl;
        return true;
    }

    bool ZenohMiddleware::read_data(
        uint16_t datareader_id,
        std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout)
    {
        // std::cout << "r dat" << std::endl;
        return true;
    }

    bool ZenohMiddleware::read_request(
        uint16_t replier_id,
        std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout)
    {
        std::cout << "r req" << std::endl;
        return true;
    }

    bool ZenohMiddleware::read_reply(
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
    bool ZenohMiddleware::matched_participant_from_ref(
        uint16_t participant_id,
        int16_t domain_id,
        const std::string& ref) const
    {
        std::cout << "mat part ref" << std::endl;
        return true;
    }

    bool ZenohMiddleware::matched_participant_from_xml(
        uint16_t participant_id,
        int16_t domain_id,
        const std::string& xml) const
    {
        std::cout << "mat part xml" << std::endl;
        return true;
    }

    bool ZenohMiddleware::matched_participant_from_bin(
        uint16_t participant_id,
        int16_t domain_id,
        const dds::xrce::OBJK_DomainParticipant_Binary& participant_xrce ) const
    {
        std::cout << "mat part bin" << std::endl;
        return true;
    }

    bool ZenohMiddleware::matched_topic_from_ref(
        uint16_t topic_id,
        const std::string& ref) const
    {
        std::cout << "mat top ref" << std::endl;
        return true;
    }

    bool ZenohMiddleware::matched_topic_from_xml(
        uint16_t topic_id,
        const std::string& xml) const
    {
        std::cout << "mat top xml" << std::endl;
        return true;
    }

    bool ZenohMiddleware::matched_topic_from_bin(
        uint16_t topic_id,
        const dds::xrce::OBJK_Topic_Binary& topic_xrce) const
    {
        std::cout << "mat top bin" << std::endl;
        return true;
    }

    bool ZenohMiddleware::matched_datawriter_from_ref(
        uint16_t datawriter_id,
        const std::string& ref) const
    {
        std::cout << "mat dw ref" << std::endl;
        return true;
    }

    bool ZenohMiddleware::matched_datawriter_from_xml(
        uint16_t datawriter_id,
        const std::string& xml) const
    {
        std::cout << "mat dw xml" << std::endl;
        return true;
    }

    bool ZenohMiddleware::matched_datawriter_from_bin(
        uint16_t datawriter_id,
        const dds::xrce::OBJK_DataWriter_Binary& datawriter_xrce) const
    {
        std::cout << "mat dw bin" << std::endl;
        return true;
    }

    bool ZenohMiddleware::matched_datareader_from_ref(
        uint16_t datareader_id,
        const std::string& ref) const
    {
        std::cout << "mat dr ref" << std::endl;
        return true;
    }

    bool ZenohMiddleware::matched_datareader_from_xml(
        uint16_t datareader_id,
        const std::string& xml) const
    {
        std::cout << "mat dr xml" << std::endl;
        return true;
    }

    bool ZenohMiddleware::matched_datareader_from_bin(
        uint16_t datareader_id,
        const dds::xrce::OBJK_DataReader_Binary& datareader_xrce) const
    {
        std::cout << "mat dr bin" << std::endl;
        return true;
    }

    bool ZenohMiddleware::matched_requester_from_ref(
        uint16_t participant_id,
        const std::string& ref) const
    {
        std::cout << "mat req ref" << std::endl;
        return true;
    }

    bool ZenohMiddleware::matched_requester_from_xml(
        uint16_t participant_id,
        const std::string& xml) const
    {
        std::cout << "mat req xml" << std::endl;
        return true;
    }

    bool ZenohMiddleware::matched_requester_from_bin(
        uint16_t requester_id,
        const dds::xrce::OBJK_Requester_Binary& requester_xrce) const
    {
        std::cout << "mat req bin" << std::endl;
        return true;
    }

    bool ZenohMiddleware::matched_replier_from_ref(
        uint16_t participant_id,
        const std::string& ref) const
    {
        std::cout << "mat rep ref" << std::endl;
        return true;
    }

    bool ZenohMiddleware::matched_replier_from_xml(
        uint16_t participant_id,
        const std::string& xml) const
    {
        std::cout << "mat rep xml" << std::endl;
        return true;
    }

    bool ZenohMiddleware::matched_replier_from_bin(
        uint16_t replier_id,
        const dds::xrce::OBJK_Replier_Binary& replier_xrce) const
    {
        std::cout << "mat rep bin" << std::endl;
        return true;
    }

} // namespace eprosima
} // namespace uxr
