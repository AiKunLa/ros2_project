// 1.包含头文件；
#include "rclcpp/rclcpp.hpp"
#include "base_interfaces_demo/msg/student.hpp"

using namespace std::chrono_literals;
using base_interfaces_demo::msg::Student;

class MinimalPublisher : public rclcpp::Node
{
public:
    MinimalPublisher() : Node("studdent_publisher"), count_(0)
    {
        publicher_ = this->create_publisher<Student>("topic_stu", 10);
        timer_ = this->create_wall_timer(500ms, std::bind(&MinimalPublisher::timer_callback, this));
    }

private:
    void timer_callback()
    {
        auto stu = Student();
        stu.name = "zhang shan";
        stu.age = count_++;
        stu.height = 1.65;
        RCLCPP_INFO(this->get_logger(), "学生信息:name=%s,age=%d,height=%.2f",
                    stu.name.c_str(), stu.age, stu.height);
        publicher_->publish(stu);
    }
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Publisher<Student>::SharedPtr publicher_;
    size_t count_;
};

int main(int argc, char const *argv[])
{

    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<MinimalPublisher>());
    rclcpp::shutdown();
    return 0;
}
