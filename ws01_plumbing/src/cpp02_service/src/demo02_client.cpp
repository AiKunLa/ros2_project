#include "rclcpp/rclcpp.hpp"
#include "base_interfaces_demo/srv/add_ints.hpp"

using base_interfaces_demo::srv::AddInts;
using namespace std::chrono_literals;

class MinimalClient : public rclcpp::Node
{
public:
    MinimalClient() : Node("minimal_client")
    {
        client = this->create_client<AddInts>("add_ints");
        RCLCPP_INFO(this->get_logger(), "客户端创建，等待连接服务端！");
    }
    // 用于建立链接，循环进行尝试
    bool connect_server()
    {
        // wait_for_service() 在指定时间内去链接服务亲，链接成功了返回true，否则返回false
        while (!client->wait_for_service(1s)) // 循环去链接服务，直到连接上服务，只要没有连上就会一直去链接
        {
            /* code */
            // 手动退出链接服务器，ctrl c
            if (!rclcpp::ok())
            {
                RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "强制退出！");
                return false;
            }
            RCLCPP_INFO(this->get_logger(), "服务连接中，请稍候...");
        }
        return true;
    }
    // 想服务端发送数据，并获取结果 async_send_request
    rclcpp::Client<AddInts>::FutureAndRequestId send_request(int32_t num1, int32_t num2)
    {
        // 构建请求数据
        auto request = std::make_shared<AddInts::Request>();
        request->num1 = num1;
        request->num2 = num2;
        // 发送请求
        return client->async_send_request(request);
    }

private:
    rclcpp::Client<AddInts>::SharedPtr client;
};

int main(int argc, char const *argv[])
{
    if (argc != 3)
    {
        RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "请提交两个整型数据！");
        return 1;
    }

    // 2.初始化 ROS2 客户端；
    rclcpp::init(argc, argv);

    // 4.创建对象指针并调用其功能；
    auto client = std::make_shared<MinimalClient>();
    bool flag = client->connect_server();
    if (!flag)
    {
        RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "服务连接失败！");
        return 0;
    }

    auto response = client->send_request(atoi(argv[1]), atoi(argv[2]));

    // 处理响应
    if (rclcpp::spin_until_future_complete(client, response) == rclcpp::FutureReturnCode::SUCCESS)
    {
        RCLCPP_INFO(client->get_logger(), "请求正常处理");
        RCLCPP_INFO(client->get_logger(), "响应结果:%d!", response.get()->sum);
    }
    else
    {
        RCLCPP_INFO(client->get_logger(), "请求异常");
    }

    // 5.释放资源。
    rclcpp::shutdown();
    return 0;
}
