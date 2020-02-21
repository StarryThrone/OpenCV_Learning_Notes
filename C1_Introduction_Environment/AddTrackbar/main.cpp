//
//  main.cpp
//  AddTrackbar
//
//  Created by chenjie on 2020/2/19.
//  Copyright © 2020 chenjie. All rights reserved.
//

// 运行需要注意以下几点
// 1. 如果报动态库无法加载，首先确保已经安装OpenCV，然后请在工程选项-当前Target选项-General选项-Framework and libraries中检查所依赖的动态库，在文件资源的Frameworks中删除相应的动态库，再添加自己安装好的动态库。
// 2. 如果报动态库无法加载，并提示无动态库签名，请在终端使用命令“codesign -f -s "Mac Developer: <你的苹果开发者账号邮箱>" *.dylib“对某个动态库签名，由于动态库存在依赖问题，你也可以使用命令“codesign -f -s "Mac Developer: <你的苹果开发者账号邮箱>" /usr/local/opt/*/lib/*.dylib“对该目录下的所有动态库签名。
// 3. 如果在编译时OpenCV报头文件无法找到，请确保在工程选项-当前Target选项-Build Settings选项中搜索Header Search Paths，将其改为你自己在/usr/local下安装好的包含OpenCV2头文件集合的文件夹，需要注意是要设置为OpenCV2的上级目录，否则编译时会报错。
// 4. 如果运行时无法加载图片，请检查Edit Schem中的Argument选项设置正确，这是主函数的第二个参数，需要注意的是这里使用的是绝对路径， 你需要将其替换成自己环境中的绝对路径

#include <iostream>
#include <fstream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

// 两种播放模式
// A. 单步模式，加载下一帧
// B. 播放模式，持续加载剩余帧，正常播放

// 继续播放的帧数，<= 0时表示持续播放即播放模式，=1时表示单步模式
int g_run = 1;
// 视频捕捉对象
cv::VideoCapture g_cap;

void onTrackbarSlide(int pos, void *) {
    // 设置当前待解析的帧位置
    g_cap.set(cv::CAP_PROP_POS_FRAMES, pos);
    if (g_run != 1) {
        // 切换到单步模式
        g_run = 1;
    }
}

void help(const char * argv[]) {
    std::cout << "\n"
    << "Demo: Addeing a trackbar to a basic viewer for moving w/in the video file \n"
    << argv[0] <<" <path/video>\n"
    << "For example:\n"
    << argv[0] << " ../tree.avi\n"
    << std::endl;
}

int main(int argc, const char * argv[]) {
    if (argc != 2) {
        help(argv);
        return 0;
    }
    
    // 创建窗口
    cv::namedWindow("Example-AddTrackbar", cv::WINDOW_AUTOSIZE);
    
    // 打开视频文件
    g_cap.open(argv[1]);
    
    // 读取基本的视频信息
    int frames = (int)g_cap.get(cv::CAP_PROP_FRAME_COUNT);
    int tmpw = (int)g_cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int tmph = (int)g_cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    std::cout << "Video has " << frames << " frames of dimensions("
    << tmpw << ", " << tmph << ")." << std::endl;
    
    // 创建进度条，并将油标位置设置在进度条起点处
    int g_slider_position = 0;
    cv::createTrackbar("Position", "Example-AddTrackbar", &g_slider_position, frames, onTrackbarSlide);
    
    cv::Mat frame;
    for ( ; ; ) {
        if (g_run != 0) {
            // 非暂停模式下读取下一帧数据
            g_cap >> frame;
            // 如果数据获取失败则退出循环
            if (frame.empty()) {
                break;
            }
            
            // 更新进度条位置
            int current_pos = (int)g_cap.get(cv::CAP_PROP_POS_FRAMES);
            cv::setTrackbarPos("Position", "Example-AddTrackbar", current_pos);
            
            // 显示当前帧
            cv::imshow("Example-AddTrackbar", frame);
            
            g_run -= 1;
        }
        
        // 挂起程序10毫秒，等待键盘输入
        char c = (char)cv::waitKey(10);
        if (c == 's') {
            // 切换到单步模式，即只加载下一帧画面
            g_run = 1;
            std::cout << "Single step, run =" << g_run << std::endl;
        } else if (c == 'r') {
            // 切换到播放模式
            g_run = -1;
            std::cout << "Run mode, run = " << g_run << std::endl;
        } else if (c == 27) {
            // 点击ESC键时退出循环
            break;
        }
    }
    
    cv::destroyWindow("Example-AddTrackbar");

    return 0;
}
