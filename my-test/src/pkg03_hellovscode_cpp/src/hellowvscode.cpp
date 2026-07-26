#include "rclcpp/rclcpp.hpp"

int main(int argc, char const *argv[])
{
  /* code */

  rclcpp::init(argc, argv);
  auto node = rclcpp::Node::make_shared("helloword");
  RCLCPP_INFO(node->get_logger(), "hello vscode");
  rclcpp::shutdown();
  return 0;
}
