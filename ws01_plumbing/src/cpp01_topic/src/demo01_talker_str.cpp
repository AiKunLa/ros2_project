/*
  需求：以某个固定频率发送文本"hello world!"，文本后缀编号，每发送一条消息，编号递增1。
  步骤：
    1.包含头文件；
    2.初始化 ROS2 客户端；
    3.定义节点类；
      3-1.创建发布方；
      3-2.创建定时器；
      3-3.组织消息并发布。
    4.调用spin函数，并传入节点对象指针；
    5.释放资源。
*/

// 1. 包含头文件
#include "rclcpp/rclcpp.hpp"       // ROS2 C++客户端库，提供 Node、Publisher、Timer 等核心类
#include "std_msgs/msg/string.hpp" // 标准消息类型：std_msgs::msg::String，用于传输字符串数据

// 使用 chrono 字面量（如 500ms），方便在定时器创建时指定时间间隔
using namespace std::chrono_literals;

/**
 * @brief 自定义发布者节点类
 *
 * 继承自 rclcpp::Node，封装了发布方、定时器以及消息发布逻辑。
 * 定时器每隔 500ms 触发一次回调，在回调中构建并发布一条 String 消息。
 */
class MinimalPublisher : public rclcpp::Node
{
public:
    /**
     * @brief 构造函数
     *
     * 初始化节点名称为 "minimal_publisher"，并将计数器 count_ 初始化为 0。
     * 在构造函数中完成发布方和定时器的创建。
     */
    MinimalPublisher() : Node("minimal_publisher"), count_(0)
    {
        // 3-1. 创建发布方
        //   模板参数为消息类型 std_msgs::msg::String；
        //   第一个参数 "topic" 为话题名称；
        //   第二个参数 10 为消息队列容量（QoS 深度），超出时丢弃最旧的消息。
        publicher_ = this->create_publisher<std_msgs::msg::String>("topic", 10);

        // 3-2. 创建定时器
        //   每隔 500 毫秒触发一次，绑定到 timer_callback 回调函数。
        //   std::bind 将当前对象指针 this 与成员函数绑定，确保回调在正确对象上执行。
        timer_ = this->create_wall_timer(500ms, std::bind(&MinimalPublisher::timer_callback, this));
    }

private:
    /**
     * @brief 定时器回调函数
     *
     * 每次定时器触发时执行：
     *   1. 创建 String 消息对象；
     *   2. 将 "Hello world" 与递增的编号拼接作为消息内容；
     *   3. 通过日志打印发布内容；
     *   4. 调用发布方的 publish() 方法将消息发布到话题上。
     */
    void timer_callback()
    {
        // 3-3. 组织消息并发布
        auto message = std_msgs::msg::String();                                    // 创建空消息对象
        message.data = "Hello world" + std::to_string(count_++);                   // 填充消息内容，编号自增
        RCLCPP_INFO(this->get_logger(), "发布的消息：'%s'", message.data.c_str()); // 打印日志到控制台
        publicher_->publish(message);                                              // 将消息发布到 "topic" 话题
    };

    // 成员变量
    rclcpp::TimerBase::SharedPtr timer_;                            // 定时器指针，驱动消息的周期性发布
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publicher_; // 发布方指针，负责将消息发送到指定话题
    size_t count_;                                                  // 消息计数器，每发一条消息自增 1
};

int main(int argc, char const *argv[])
{
    rclcpp::init(argc, argv);
    // 调用spin函数，传入节点对象指针
    rclcpp::spin(std::make_shared<MinimalPublisher>());
    rclcpp::shutdown();
    return 0;
}
