#include <termios.h>
#include <signal.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <tf2/utils.h>
#include <tf/transform_datatypes.h>
#include <boost/thread/thread.hpp>
#include <ros/ros.h>
#include <std_msgs/Float64.h>
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

#define PI 3.1415926
double inx=0,iny=0,inz=0,ori_x=0,ori_y=0,ori_z=0,ori_w=0,yaw=0,theta=0;
bool flag=false;

class KeyboardTeleopNode
{
    private:
        double move_x_=0.1;
        double move_y_=0.1;
        double move_z_=0.1;

        ros::NodeHandle n_;

    public:
        ros::Publisher pub_;
        std_msgs::Bool sel;
        ros::Publisher dist_pub_; // Distance publisher
        std_msgs::Float64 dist_msg; // Distance message
        std_msgs::Int32 K_info;

        KeyboardTeleopNode()
        {
            pub_ = n_.advertise<std_msgs::Bool>("/Select", 1);//
            dist_pub_ = n_.advertise<std_msgs::Int32>("/control_number", 1); // Initialize distance publisher
            ros::NodeHandle n_private("~");
        }

        ~KeyboardTeleopNode() { }
        void keyboardLoop();
 
        void stopRobot()
        {

        }
};

void poseCallback(const geometry_msgs::PoseStamped::ConstPtr& msg)
{
    if(!flag){
        inx=msg->pose.position.x;
        iny=msg->pose.position.y;
        inz=msg->pose.position.z;
        ori_x=msg->pose.orientation.x;
        ori_y=msg->pose.orientation.y;
        ori_z=msg->pose.orientation.z;
        ori_w=msg->pose.orientation.w;
	yaw=tf2::getYaw(msg->pose.orientation);
    }
    flag=true;
    theta=tf2::getYaw(msg->pose.orientation);
}


KeyboardTeleopNode* tbk;
int kfd = 0;
struct termios cooked, raw;
bool done;

int main(int argc, char** argv)
{
    // ros::init(argc,argv,"tbk", ros::init_options::AnonymousName | ros::init_options::NoSigintHandler);
    ros::init(argc,argv,"kcontrol_node");
    KeyboardTeleopNode tbk;

    ros::NodeHandle nh;
    boost::thread t = boost::thread(boost::bind(&KeyboardTeleopNode::keyboardLoop, &tbk));

    ros::Rate loop_rate(10);

    ros::Time last_request = ros::Time::now();
    while(ros::ok()){

        tbk.pub_.publish(tbk.sel);
        tbk.sel.data = false;
        // tbk.dist_pub_.publish(tbk.dist_msg);

        ros::spinOnce();

        loop_rate.sleep();
    }

    t.interrupt();
    t.join();
    tbk.stopRobot();
    tcsetattr(kfd, TCSANOW, &cooked);

    return(0);
}

void KeyboardTeleopNode::keyboardLoop()
{
    char c;
    bool dirty = false;

    tcgetattr(kfd, &cooked);
    memcpy(&raw, &cooked, sizeof(struct termios));
    raw.c_lflag &=~ (ICANON | ECHO);
    raw.c_cc[VEOL] = 1;
    raw.c_cc[VEOF] = 2;
    tcsetattr(kfd, TCSANOW, &raw);

    puts("Reading from keyboard");
    puts("Press 'S' key to select waypoints");

    struct pollfd ufd;
    ufd.fd = kfd;
    ufd.events = POLLIN;
    int times=0;
    int num;

    for(;;)
    {
        boost::this_thread::interruption_point();

        if ((num = poll(&ufd, 1, 250)) < 0)
        {
            perror("poll():");
            return;
        }
        else if(num > 0)
        {
            if(read(kfd, &c, 1) < 0)
            {
                perror("read():");
                return;
            }
        }
        else
        {
            if (dirty == true)
            {
                stopRobot();
                dirty = false;
            }
            continue;
        }
/*******
#define KEYCODE_W 0x77 // 步进1
#define KEYCODE_A 0x61 // 步进2
#define KEYCODE_S 0x73 // 选点
#define KEYCODE_D 0x64 // 步进3

#define KEYCODE_I 0x69 //删除一个点
#define KEYCODE_J 0x6A // 确认发布（发布的是pose/array）
#define KEYCODE_K 0x6B // 上
#define KEYCODE_L 0x6C // 下

#define KEYCODE_UP 0x41 // 前
#define KEYCODE_DOWN 0x42 // 后
#define KEYCODE_LEFT 0x44 // 左
#define KEYCODE_RIGHT 0x43 // 右
*/
        switch(c)
        {
            /*******选点一个键*************/
            case KEYCODE_S:
                dirty = true;
                sel.data = true;
                // K_info.data = 10;
                // dist_pub_.publish(K_info);
                break;
                /*************步进三个键**************/
            case KEYCODE_A:
                dirty = true;
                K_info.data = 1;

                // dist_msg.data = 1.0; 
                dist_pub_.publish(K_info);
                break;
            case KEYCODE_W:
                dirty = true;
                K_info.data = 2;
                // dist_msg.data = 2.0; 
                dist_pub_.publish(K_info);
                break;
            case KEYCODE_D:
                dirty = true;
                K_info.data = 3;
                // dist_msg.data = 3.0; 
                dist_pub_.publish(K_info);
                break;
                /***********上下左右四个键************/
            case KEYCODE_UP:
                dirty = true;
                K_info.data = 4;
                // dist_msg.data = 3.0; 
                dist_pub_.publish(K_info);
                break;
            case KEYCODE_DOWN:
                dirty = true;
                K_info.data = 5;
                // dist_msg.data = 3.0; 
                dist_pub_.publish(K_info);
                break;
            case KEYCODE_LEFT:
                dirty = true;
                K_info.data = 6;
                // dist_msg.data = 3.0; 
                dist_pub_.publish(K_info);
                break;
            case KEYCODE_RIGHT:
                dirty = true;
                K_info.data = 7;
                // dist_msg.data = 3.0; 
                dist_pub_.publish(K_info);
                break;
                /*********删除一个点********/
            case KEYCODE_I:
                dirty = true;
                K_info.data = 8;
                // dist_msg.data = 3.0; 
                dist_pub_.publish(K_info);
                break;
                /********确认发布一个点*/
            case KEYCODE_J:
                dirty = true;
                K_info.data = 9;
                // dist_msg.data = 3.0; 
                dist_pub_.publish(K_info);
                break;
                // 上
            case KEYCODE_K:
                dirty = true;
                K_info.data = 10;
                // dist_msg.data = 3.0; 
                dist_pub_.publish(K_info);
                break;
                // 下
            case KEYCODE_L:
                dirty = true;
                K_info.data = 11;
                // dist_msg.data = 3.0; 
                dist_pub_.publish(K_info);
                break;
            default:
                dirty = false;
        }
        // ROS_INFO("TP: [%f][%f][%f] ;  CP: [%f][%f]",pose_.pose.position.x,pose_.pose.position.y,pose_.pose.position.z,inx,iny);
        num = 0;
    }
}
