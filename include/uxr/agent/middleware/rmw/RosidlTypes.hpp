#include <map>
#include <string>
#include <rcl/rcl.h>

#include <rosidl_typesupport_cpp/message_type_support.hpp>
#include <rosidl_typesupport_fastrtps_cpp/message_type_support.h> //for deserialization

#include <std_msgs/msg/int8.hpp>
#include <std_msgs/msg/bool.hpp>
#include <riptide_msgs2/msg/firmware_status.hpp>

class RosMessageType
{
    public:
    virtual size_t size() = 0;
    virtual const rosidl_message_type_support_t *get_typesupport_handle() = 0;
    virtual void get_empty_as_void_ptr(void *buf) = 0;
};

template<typename T>
class RosMessageTypeImpl : public RosMessageType
{
    public:
    RosMessageTypeImpl()
     : typesupport_handle(rosidl_typesupport_cpp::get_message_type_support_handle<T>())
    { }
    
    size_t size() override
    {
        return sizeof(T);
    }

    const rosidl_message_type_support_t *get_typesupport_handle() override
    {
        return typesupport_handle;
    }

    void get_empty_as_void_ptr(void *buf) override
    {
        memcpy(buf, &msg, size());
    }

    private:
    const T msg;
    const rosidl_message_type_support_t *typesupport_handle;
};


static std::map<std::string, std::shared_ptr<RosMessageType>> ROSIDL_TYPES = {
    { "std_msgs::msg::dds_::Int8_", std::make_shared<RosMessageTypeImpl<std_msgs::msg::Int8>>() },
    { "riptide_msgs2::msg::dds_::FirmwareStatus_", std::make_shared<RosMessageTypeImpl<riptide_msgs2::msg::FirmwareStatus>>() },
    { "std_msgs::msg::dds_::Bool_", std::make_shared<RosMessageTypeImpl<std_msgs::msg::Bool>>() }
};

