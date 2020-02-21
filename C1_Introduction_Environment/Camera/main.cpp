//
//  main.cpp
//  Camera
//
//  Created by chenjie on 2020/2/20.
//  Copyright © 2020 chenjie. All rights reserved.
//

// 运行需要注意以下几点
// 1. 如果报动态库无法加载，首先确保已经安装OpenCV，然后请在工程选项-当前Target选项-General选项-Framework and libraries中检查所依赖的动态库，在文件资源的Frameworks中删除相应的动态库，再添加自己安装好的动态库。
// 2. 如果报动态库无法加载，并提示无动态库签名，请在终端使用命令“codesign -f -s "Mac Developer: <你的苹果开发者账号邮箱>" *.dylib“对某个动态库签名，由于动态库存在依赖问题，你也可以使用命令“codesign -f -s "Mac Developer: <你的苹果开发者账号邮箱>" /usr/local/opt/*/lib/*.dylib“对该目录下的所有动态库签名。
// 3. 如果在编译时OpenCV报头文件无法找到，请确保在工程选项-当前Target选项-Build Settings选项中搜索Header Search Paths，将其改为你自己在/usr/local下安装好的包含OpenCV2头文件集合的文件夹，需要注意是要设置为OpenCV2的上级目录，否则编译时会报错。
// 4. 如果运行时无法加载图片，请检查Edit Schem中的Argument选项设置正确，这是主函数的第二个参数，需要注意的是这里使用的是绝对路径， 你需要将其替换成自己环境中的绝对路径

#include <iostream>
#include <opencv2/videoio.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

// Attention: 该执行直接运行会报错《OpenCV: not authorized to capture video (status 0), requesting...》
// 这是因为XCode命令行工程未获取系统相机权限，可以选择使用系统终端（注意不能是item）运行编译好的可执行文件即可

const std::string c_window_name = "Camera";

void help(const char * argv[]) {
    std::cout << "\n"
    << "Demo: Load video from a camera or a file \n"
    << argv[0]
    << "If with parameter [video_path], will load video from file \n"
    << "If not, will from camera \n"
    << std::endl;
}

int main(int argc, const char * argv[]) {
    help(argv);
    
    // 根据参数从相机或者文件读取图像数据
    cv::VideoCapture cap;
    if (argc == 1) {
        cap.open(0);
    } else {
        cap.open(argv[1]);
    }
    
    // 判断数据是否获取成功
    if (!cap.isOpened()) {
        std::cerr << "Couldn't open video stream." << std::endl;
        return -1;
    }
    
    // 循环显示获得的数据
    cv::Mat frame;
    cv::namedWindow(c_window_name);
    for (; ; ) {
        cap >> frame;
        if (frame.empty()) {
            // 如果解析数据错误，则退出循环
            break;
        }
        
        // 显示图片
        cv::imshow(c_window_name, frame);
        
        // 挂起程序33毫秒，在此期间如果监听到任何键盘输入事件则退出循环
        if ((char)cv::waitKey(33) >= 0) {
            break;
        }
    }
    
    // 摧毁窗口，并释放相关的内存资源
    cv::destroyWindow(c_window_name);
    
    return 0;
}
