//
//  main.cpp
//  VideoExporter
//
//  Created by chenjie on 2020/2/21.
//  Copyright © 2020 chenjie. All rights reserved.
//

// 运行需要注意以下几点
// 1. 如果报动态库无法加载，首先确保已经安装OpenCV，然后请在工程选项-当前Target选项-General选项-Framework and libraries中检查所依赖的动态库，在文件资源的Frameworks中删除相应的动态库，再添加自己安装好的动态库。
// 2. 如果报动态库无法加载，并提示无动态库签名，请在终端使用命令“codesign -f -s "Mac Developer: <你的苹果开发者账号邮箱>" *.dylib“对某个动态库签名，由于动态库存在依赖问题，你也可以使用命令“codesign -f -s "Mac Developer: <你的苹果开发者账号邮箱>" /usr/local/opt/*/lib/*.dylib“对该目录下的所有动态库签名。
// 3. 如果在编译时OpenCV报头文件无法找到，请确保在工程选项-当前Target选项-Build Settings选项中搜索Header Search Paths，将其改为你自己在/usr/local下安装好的包含OpenCV2头文件集合的文件夹，需要注意是要设置为OpenCV2的上级目录，否则编译时会报错。

#include <iostream>
#include <opencv2/videoio.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

const std::string c_windowName_in = "VideoExporter_In";
const std::string c_windowName_out = "VideoExporter_Out";

void help(const char * argv[]) {
    std::cout << "Demo: VideoExporter \n"
    << argv[0] <<" <path/video_in> <paht/video_out> \n"
    << "For example:\n"
    << argv[0] << " ./tree.avi ./treeout.avi \n"
    << std::endl;
}

int fourCharCode(char a, char b, char c, char d) {
    return (int)a + ((int)b << 8) + ((int)c << 16) + ((int)d << 24);
}

int main(int argc, const char * argv[]) {
    if (argc != 3) {
        help(argv);
        return 0;
    }
    
    // 创建视频捕捉对象，并采集视频文件相关信息
    cv::VideoCapture capture(argv[1]);
    cv::Size size (
        (int)capture.get(cv::CAP_PROP_FRAME_WIDTH),
        (int)capture.get(cv::CAP_PROP_FRAME_HEIGHT)
    );
    double fps = capture.get(cv::CAP_PROP_FPS);
    
    // 创建视频导出器对象，并开启视频写入会话
    cv::VideoWriter writer;
    int fcc = fourCharCode('M', 'J', 'P', 'G');
    writer.open(argv[2], fcc, fps, size);
    
    // 通过循环的方式处理视频流，并将帧数据放入导出器中
    cv::Mat original_frame, logpolar_frame;
    cv::namedWindow(c_windowName_in);
    cv::namedWindow(c_windowName_out);
    for (; ; ) {
        // 读取帧数据
        capture >> original_frame;
        if (original_frame.empty()) {
            // 数据读取失败，退出循环
            break;
        }
        
        // 得到对数极坐标转换后的图像
        cv::logPolar(
            original_frame,
            logpolar_frame,
            cv::Point2f(
                original_frame.cols/2,
                original_frame.rows/2
            ),
            40,
            cv::WARP_FILL_OUTLIERS
        );
        
        // 显示原图和处理后的图片
        cv::imshow(c_windowName_in, original_frame);
        cv::imshow(c_windowName_out, logpolar_frame);
        
        // 将处理后的帧数据放入导出器
        writer << logpolar_frame;
        
        // 暂时挂起程序，如果接受键盘输入，则退出循环
        int delay = fps <= 0 ? 40 : (int)1000/fps;
        if (cv::waitKey(delay) == 27) {
            break;
        }
    }
    
    // 释放视频捕捉器和导出器资源
    capture.release();
    writer.release();
    
    // 销毁窗口对象，释放内存资源
    cv::destroyWindow(c_windowName_in);
    cv::destroyWindow(c_windowName_out);
    
    return 0;
}
