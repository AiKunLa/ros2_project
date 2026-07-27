#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using std::placeholders::_1;

class MinimalSubscriber : public rclcpp::Node
{
public:
    MinimalSubscriber() : Node("minimal_subscriber")
    {
        // 这里是的_1是占位符，表示将来收到的那个消息的位置
        subscription_ = this->create_subscription<std_msgs::msg::String>("topic", 10, std::bind(&MinimalSubscriber::topic_callback, this, _1));
    }

private:
    // 这里的const表示的是不会修感类里面的任何成员变量
    void topic_callback(const std_msgs::msg::String &msg) const
    {
        RCLCPP_INFO(this->get_logger(), "订阅消息： '%s'", msg.data.c_str());
    }
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
};

int main(int argc, char const *argv[])
{
    /* code */

    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<MinimalSubscriber>());
    rclcpp::shutdown();

    return 0;
}
