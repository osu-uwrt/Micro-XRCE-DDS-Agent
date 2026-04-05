#include <map>
#include <string>
#include <rcl/rcl.h>

#include <rosidl_typesupport_cpp/message_type_support.hpp>
#include <rosidl_typesupport_fastrtps_cpp/message_type_support.h> //for deserialization

#include <std_msgs/msg/u_int8.hpp>
#include <std_msgs/msg/int8.hpp>
#include <std_msgs/msg/float32.hpp>
#include <std_msgs/msg/bool.hpp>
#include <std_msgs/msg/string.hpp>
#include <std_msgs/msg/int16.hpp>
#include <std_msgs/msg/u_int16.hpp>
#include <std_msgs/msg/string.hpp>
#include <mercury_msgs/msg/firmware_status.hpp>
#include <mercury_msgs/msg/electrical_command.hpp>
#include <mercury_msgs/msg/depth.hpp>
#include <mercury_msgs/msg/kill_switch_report.hpp>
#include <mercury_msgs/msg/electrical_readings.hpp> 

class RosMessageType
{
    public:
    virtual size_t size() = 0;
    virtual const rosidl_message_type_support_t *get_typesupport_handle() = 0;
    virtual void *get_empty_as_void_ptr(size_t *sz) = 0;

    void delete_empty(void *empty)
    {
        delete empty;
    }
};

template<typename T>
class RosMessageTypeImpl : public RosMessageType
{
    public:
    RosMessageTypeImpl()
     : msg(T()),
       typesupport_handle(rosidl_typesupport_cpp::get_message_type_support_handle<T>())
    { }
    
    size_t size() override
    {
        return sizeof(msg);
    }

    const rosidl_message_type_support_t *get_typesupport_handle() override
    {
        return typesupport_handle;
    }

    void *get_empty_as_void_ptr(size_t *sz) override
    {
        // size_t sz = size();
        // if(sz > max_sz)
        // {
        //     return 0;
        // }

        // memcpy(buf, &msg, sz);
        // return sz;

        *sz = size();
        return new T();
    }

    private:
    const T msg;
    const rosidl_message_type_support_t *typesupport_handle;
};


static std::map<std::string, std::shared_ptr<RosMessageType>> ROSIDL_TYPES = {
    { "std_msgs::msg::dds_::Int8_", std::make_shared<RosMessageTypeImpl<std_msgs::msg::Int8>>() },
    { "std_msgs::msg::dds_::Float32_", std::make_shared<RosMessageTypeImpl<std_msgs::msg::Float32>>() },
    { "std_msgs::msg::dds_::Bool_", std::make_shared<RosMessageTypeImpl<std_msgs::msg::Bool>>() },
    { "mercury_msgs::msg::dds_::FirmwareStatus_", std::make_shared<RosMessageTypeImpl<mercury_msgs::msg::FirmwareStatus>>() },
    { "mercury_msgs::msg::dds_::ElectricalCommand_", std::make_shared<RosMessageTypeImpl<mercury_msgs::msg::ElectricalCommand>>() },
    { "mercury_msgs::msg::dds_::Depth_", std::make_shared<RosMessageTypeImpl<mercury_msgs::msg::Depth>>() },
    { "mercury_msgs::msg::dds_::KillSwitchReport_", std::make_shared<RosMessageTypeImpl<mercury_msgs::msg::KillSwitchReport>>() },
    { "mercury_msgs::msg::dds_::ElectricalReadings_", std::make_shared<RosMessageTypeImpl<mercury_msgs::msg::ElectricalReadings>>() }
};

