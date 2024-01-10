#include <chrono>
#include <functional>
#include <memory>
#include <string>

#include "geometry_msgs/msg/detail/twist__struct.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "geometry_msgs/msg/transform_stamped.hpp"
#include "rclcpp/logging.hpp"
#include "tf2_ros/buffer.h"
#include "rclcpp/rclcpp.hpp"
#include "tf2_ros/transform_listener.h"
#include "tf2/exceptions.h"

using namespace std::chrono_literals;
class RobotChase: public rclcpp::Node{
  public:
    RobotChase(): Node("turtle_tf2_frame_listener"){
    
    
    tf_buffer_ = std::make_unique<tf2_ros::Buffer>(this->get_clock());
    tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);

    publisher_ = 
      this->create_publisher<geometry_msgs::msg::Twist>("/rick/cmd_vel",1);
    timer_ = this->create_wall_timer(1s,std::bind(&RobotChase::on_timer, this));
    kp_yaw = 5.0;
    kp_distance = 0.5;
    }

  private:
    void on_timer(){
      geometry_msgs::msg::TransformStamped t;
      try { 
        t = tf_buffer_->lookupTransform(fromFrameRel, toFrameRel,tf2::TimePointZero);}
      catch (const tf2::TransformException &ex){
        RCLCPP_INFO(
            this->get_logger(), "Could not transform %s to %s: %s",
            toFrameRel.c_str(), fromFrameRel.c_str(), ex.what());
          return;
      }
    geometry_msgs::msg::Twist speed;
    //Calculate the distance (error_distance) and angular error (error_yaw) between the ricks and mortys refrence frames
    error_distance = sqrt(pow(t.transform.translation.x, 2) + pow(t.transform.translation.y, 2));
    
    error_yaw = atan2(t.transform.translation.y,t.transform.translation.x);
    RCLCPP_INFO(this->get_logger(), "this is the error angle %f",error_yaw);
    //Define the angular velocity by multiplying a fixed value (kp_yaw) and the error variable error_yaw
    speed.angular.z = kp_yaw * error_yaw;
    RCLCPP_INFO(this->get_logger(), "this is the angular speed %f",speed.angular.z);
    //Define the linear velocity by multiplying a fixed value (kp_distance) and the error variable error_distance
    speed.linear.x = kp_distance * error_distance;
    //Create a Twist message with the linear and angular velocities an publish the Twist message
    publisher_->publish(speed);
    }

    rclcpp::TimerBase::SharedPtr timer_{nullptr};
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_{nullptr};
    std::shared_ptr<tf2_ros::TransformListener> tf_listener_{nullptr};
    std::unique_ptr<tf2_ros::Buffer> tf_buffer_;
    std::string fromFrameRel = "rick/base_link";
    std::string toFrameRel = "morty/base_link";
    float kp_yaw;
    float kp_distance;
    float error_distance;
    float error_yaw;
    
};

int main(int argc, char **argv){
 
 rclcpp::init(argc, argv);
 rclcpp::spin(std::make_shared<RobotChase>());
 rclcpp::shutdown();
 return 0;

}