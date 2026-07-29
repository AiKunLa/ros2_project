/*
  需求：编写动作服务端实习，可以提取客户端请求提交的整型数据，并累加从1到该数据之间的所有整数以求和，
       每累加一次都计算当前运算进度并连续反馈回客户端，最后，在将求和结果返回给客户端。
  步骤：
    1.包含头文件；
    2.初始化 ROS2 客户端；
    3.定义节点类；
      3-1.创建动作服务端；
      3-2.处理请求数据；
      3-3.处理取消任务请求；
      3-4.生成连续反馈。
    4.调用spin函数，并传入节点对象指针；
    5.释放资源。

*/

/*
    引入ros2核心库、动作库以及接口库
*/
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "base_interfaces_demo/action/progress.hpp"

using namespace std::placeholders;                                    // 引入命名空间中所有的占位符
using base_interfaces_demo::action::Progress;                         // 将Progress这个动作类型从完整的命名空间中拉到当前作用域中
using GoalHandleProgress = rclcpp_action::ServerGoalHandle<Progress>; // 命名别名，为rclcpp_action::ServerGoalHandle<Progress>这个去一个别名，
// ServerGoalHandle 对象是用于获取目标值、检查取消状态、发布反馈、设置结果的方法。

class MinimalActionServer : public rclcpp::Node
{
public:
    // 接受一个NodeOptions的常引用，避免拷贝。如果不传递参数就默认构造一个NodeOptions对象
    explicit MinimalActionServer(const rclcpp::NodeOptions &options = rclcpp::NodeOptions()) : Node("minimal_action_server", options)
    {
        this->action_server_ = rclcpp_action::create_server<Progress>(
            this,
            "get_sum",
            std::bind(&MinimalActionServer::handle_goal, this, _1, _2),
            std::bind(&MinimalActionServer::handle_cancel, this, _1),
            std::bind(&MinimalActionServer::handle_accepted, this, _1));
        RCLCPP_INFO(this->get_logger(), "动作服务端创建，等待请求...");
    }

private:
    rclcpp_action::Server<Progress>::SharedPtr action_server_;

    // 处理请求数据
    /**
     * handle_goal 函数收到了两个参数：uuid 和 goal，但这个函数只用到了 goal，uuid 没有被使用。编译器会报一个警告：
        warning: unused parameter 'uuid'
        (void)uuid; 这行做了两件事：

        用了一下 uuid（转换成 void 算是一种"使用"）
        实际上什么事都没干（void 转换是空操作）
        编译器看到 uuid 被"使用"了，就不再警告未使用参数了。
        因为 handle_goal 是作为回调函数绑定给 action server 的（第 38 行）
        ，ROS 2 要求这个回调的签名必须包含 uuid 和 goal 两个参数，即使你的业务逻辑不需要 uuid，也必须按约定接收它。

        std::shared_ptr<const Progress::Goal> goal
            类型：指向 Progress::Goal 的常共享指针
            含义：客户端发来的目标值，也就是实际的数据
            作用：告诉服务端"你要算什么"。在你的动作接口定义中，Progress::Goal 里面应该有一个 int32 num 之类的字段，表示客户端想求和的终点数字
     */
    rclcpp_action::GoalResponse handle_goal(const rclcpp_action::GoalUUID &uuid, std::shared_ptr<const Progress::Goal> goal)
    {
        (void)uuid;
        RCLCPP_INFO(this->get_logger(), "接收到动作客户端请求，请求数字为 %ld", goal->num);

        // 传递过来的数字小于num则拒绝执行
        if (goal->num < 1)
        {
            return rclcpp_action::GoalResponse::REJECT;
        }
        // 返回接受并执行，服务端开始从1累加到num
        return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
    }
    // 处理取消任务请求 ，
    /*
        std::shared_ptr<GoalHandleProgress>，这个是共享的智能指针，指向ROS2题动的用于操纵服务的
    */
    rclcpp_action::CancelResponse handle_cancel(const std::shared_ptr<GoalHandleProgress> goal_handle)
    {
        (void)goal_handle;
        RCLCPP_INFO(this->get_logger(), "接收到任务取消请求");
        return rclcpp_action::CancelResponse::ACCEPT;
    }
    // 执行函数
    void execute(const std::shared_ptr<GoalHandleProgress> goal_handle)
    {
        RCLCPP_INFO(this->get_logger(), "");
        rclcpp::Rate loop_rate(10.0);
        const auto goal = goal_handle.get_goal();

        // 创建一个 Progress::Feedback 对象，并返回一个管理它的共享指针。 通常紧接着就用来填充反馈数据，比如：
        auto feedbacek = std::make_shared<Progress::Feedback>();
        auto result = std::make_shared<Progress ::Result>();
        int64_t sum = 0;

        for (int i = 1; (i <= goal->num) && rclcpp::ok(); i++)
        {
            sum += i;
            if (goal_handle->is_canceling())
            {
                result->sum = sum;
                goal_handle->canceled(result);
                RCLCPP_INFO(this->get_logger(), "");
                return;
            }
            feedbacek->progress = (double_t)i / goal->num;
            goal_handle.publish_feedback(feedbacek);
            RCLCPP_INFO(this->get_logger(), "");
            loop_rate.sleep();
        }
        if (rclcpp::ok())
        {
            result->sum = sum;
            goal_handle->succeed(result);
            RCLCPP_INFO(this->get_logger(), "");
        }
    }
    // 生成连续反馈
    void handle_accepted(const std::shared_ptr<GoalHandleProgress> goal_handle)
    {
        //
        std::thread{std::bind(&MinimalActionServer::execute, this, _1), goal_handle}.detach();
    }
};

int main(int argc, char const *argv[])
{
    rclcpp::init(argc, argv);
    auto action_server = std::make_shared<MinimalActionServer>();
    rclcpp::spin(action_server);
    rclcpp::shutdown();
    return 0;
}
