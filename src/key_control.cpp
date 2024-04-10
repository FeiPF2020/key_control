#include <termios.h>
#include <sys/poll.h>
#include <boost/thread/thread.hpp>
#include <ros/ros.h>
#include <std_msgs/Int32.h>
#include <std_msgs/Bool.h>

#define KEYCODE_W 0x77
#define KEYCODE_A 0x61
#define KEYCODE_S 0x73
#define KEYCODE_D 0x64

#define KEYCODE_I 0x69
#define KEYCODE_J 0x6A
#define KEYCODE_K 0x6B
#define KEYCODE_L 0x6C

#define KEYCODE_UP 0x41
#define KEYCODE_DOWN 0x42
#define KEYCODE_LEFT 0x44
#define KEYCODE_RIGHT 0x43

#define KEYCODE_X 0x78
#define KEYCODE_P 0x70

int kfd = 0;

struct termios cooked, raw;

class Key_control {
private:
    ros::NodeHandle n_;

public:
    ros::Publisher select_point_pub_;
    ros::Publisher index_pub_;

    std_msgs::Int32 key_index;
    std_msgs::Bool select_state;

    Key_control() {
        select_point_pub_ = n_.advertise<std_msgs::Bool>("/Select", 1);
        index_pub_ = n_.advertise<std_msgs::Int32>("/index_number", 1);
    }

    void keyboardLoop();

    // void stopRobot()
    // {
    //     // ROS_INFO_STREAM("stopRobot");
    // }
    ~Key_control() = default;
};

void Key_control::keyboardLoop() {
    char c;
    bool dirty = false;

    tcgetattr(kfd, &cooked);
    memcpy(&raw, &cooked, sizeof(struct termios));
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VEOL] = 1;
    raw.c_cc[VEOF] = 2;
    tcsetattr(kfd, TCSANOW, &raw);
    puts("***Reading from keyboard***");
    puts("Press 'a,w,d' key to get step distance");
    puts("Press 'up_z_up,down_z_down,i_forward,k_backward,j_y_left,l_y_right' key to get direction");
    puts("Press 'p_send,x_withdraw' key to perform action");
    puts("**************");

    struct pollfd ufd{};
    ufd.fd = kfd;
    ufd.events = POLLIN;
    int num = 0;

    while (true) {
        boost::this_thread::interruption_point();
        if ((num = poll(&ufd, 1, 250)) < 0) {
            perror("poll():");
            return;
        } else if (num > 0) {
            if (read(kfd, &c, 1) < 0) {
                perror("read():");
                return;
            }
        } else {
            if (dirty == true) {
                // stopRobot();//every key press will stop robot
                dirty = false;
            }
            continue;
        }
        switch (c) {
            case KEYCODE_S:
                dirty = true;
                select_state.data = true;
                break;
            case KEYCODE_A:
                dirty = true;
                key_index.data = 1;
                index_pub_.publish(key_index);
                break;
            case KEYCODE_W:
                dirty = true;
                key_index.data = 2;
                index_pub_.publish(key_index);
                break;
            case KEYCODE_D:
                dirty = true;
                key_index.data = 3;
                index_pub_.publish(key_index);
                break;
            case KEYCODE_I:
                dirty = true;
                key_index.data = 4;
                index_pub_.publish(key_index);
                break;
            case KEYCODE_K:
                dirty = true;
                key_index.data = 5;
                index_pub_.publish(key_index);
                break;
            case KEYCODE_J:
                dirty = true;
                key_index.data = 6;
                index_pub_.publish(key_index);
                break;
            case KEYCODE_L:
                dirty = true;
                key_index.data = 7;
                index_pub_.publish(key_index);
                break;
            case KEYCODE_X:
                dirty = true;
                key_index.data = 8;
                index_pub_.publish(key_index);
                break;
            case KEYCODE_P:
                dirty = true;
                key_index.data = 9;
                index_pub_.publish(key_index);
                break;
            case KEYCODE_UP:
                dirty = true;
                key_index.data = 10;
                index_pub_.publish(key_index);
                break;
            case KEYCODE_DOWN:
                dirty = true;
                key_index.data = 11;
                index_pub_.publish(key_index);
                break;
            default:
                dirty = false;
        }
        num = 0;
    }
}

int main(int argc, char **argv) {
    ros::init(argc, argv, "key_control_node");
    Key_control key_control;

    ros::NodeHandle nh;
//    boost::thread t = boost::thread(boost::bind(&Key_control::keyboardLoop, &key_control));
    boost::thread t = boost::thread([ObjectPtr = &key_control] { ObjectPtr->keyboardLoop(); });

    ros::Rate loop_rate(10);
    ros::Time last_request = ros::Time::now();
    while (ros::ok()) // ctrl-c notice
    {
        // If continuously posted and modified here，Detecting the rising edge
        key_control.select_point_pub_.publish(key_control.select_state);
        key_control.select_state.data = false;
        ros::spinOnce();
        loop_rate.sleep();
    }

    t.interrupt();
    t.join();
    // key_control.stopRobot();
    tcsetattr(kfd, TCSANOW, &cooked);

    return 0;
}