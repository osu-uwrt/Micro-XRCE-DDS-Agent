#include <uxr/agent/middleware/rmw/RmwMiddleware.hpp>
#include <uxr/agent/logger/Logger.hpp>
#include <uxr/agent/utils/Conversion.hpp>
#include <uxr/agent/middleware/utils/Callbacks.hpp>

namespace eprosima {
namespace uxr {

    RmwMiddleware::RmwMiddleware()
     : callback_factory_(callback_factory_.getInstance()),
       participants_{},
       topics_{},
       publishers_{},
       subscribers_{},
       datawriters_{},
       datareaders_{}
    {
        UXR_AGENT_LOG_INFO(
            UXR_DECORATE_GREEN("rmw plugin active"),
            "Universal RMW set as active middleware.", "");
    }

    /**********************************************************************************************************************
     * Create functions.
     **********************************************************************************************************************/
    bool RmwMiddleware::create_participant_by_ref(
        uint16_t participant_id,
        int16_t domain_id,
        const std::string& ref)
    {
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
        return create_participant_by_ref(participant_id, participant_xrce.domain_id(), "");
    }

    bool RmwMiddleware::create_topic_by_ref(
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

    bool RmwMiddleware::create_topic_by_xml(
        uint16_t topic_id,
        uint16_t participant_id,
        const std::string& xml)
    {
        return create_topic_by_ref(topic_id, participant_id, xml);
    }

    bool RmwMiddleware::create_topic_by_bin(
        uint16_t topic_id,
        uint16_t participant_id,
        const dds::xrce::OBJK_Topic_Binary& topic_xrce)
    {
        return create_topic_by_ref(topic_id, participant_id, topic_xrce.topic_name());
    }

    bool RmwMiddleware::create_publisher_by_xml(
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

    bool RmwMiddleware::create_publisher_by_bin(
        uint16_t publisher_id,
        uint16_t participant_id,
        const dds::xrce::OBJK_Publisher_Binary& publisher_xrce)
    {
        std::cout << "publisher by bin with name " << publisher_xrce.publisher_name() << std::endl;
        return create_publisher_by_xml(publisher_id, participant_id, "");
    }

    bool RmwMiddleware::create_subscriber_by_xml(
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
            return create_datawriter_by_ref(datawriter_id, publisher_id, topics_[topic_id]);
        }

        return false;
    }

    bool RmwMiddleware::create_datareader_by_ref(
        uint16_t datareader_id,
        uint16_t subscriber_id,
        const std::string& ref)
    {
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
            return create_datareader_by_ref(datareader_id, subscriber_id, topics_[topic_id]);
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
        std::cout << "del part" << std::endl;

        auto it = participants_.find(participant_id);
        if(it != participants_.end())
        {
            participants_.erase(participant_id);
            return true;
        }

        return false;
    }

    bool RmwMiddleware::delete_topic(uint16_t topic_id)
    {
        std::cout << "del top" << std::endl;
        return true;
    }

    bool RmwMiddleware::delete_publisher(uint16_t publisher_id)
    {
        std::cout << "del pub" << std::endl;
        return true;
    }

    bool RmwMiddleware::delete_subscriber(uint16_t subscriber_id)
    {
        std::cout << "del sub" << std::endl;
        return true;
    }

    bool RmwMiddleware::delete_datawriter(uint16_t datawriter_id)
    {
        std::cout << "del dw" << std::endl;
        return true;
    }

    bool RmwMiddleware::delete_datareader(uint16_t datareader_id)
    {
        std::cout << "del dr" << std::endl;
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
        std::cout << "wr dat" << std::endl;
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
        // std::cout << "r dat" << std::endl;
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

} // namespace eprosima
} // namespace uxr
