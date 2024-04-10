cmake_minimum_required(VERSION 3.0.2)
project(key_control)
add_compile_options(-std=c++14)
find_package(catkin REQUIRED COMPONENTS
  roscpp
  rospy
  std_msgs
)
catkin_package(
)
include_directories(
  ${catkin_INCLUDE_DIRS}
)
add_executable(${PROJECT_NAME}_node src/key_control.cpp)
target_link_libraries(${PROJECT_NAME}_node 
  ${catkin_LIBRARIES}
)