//
//  main.cpp
//  DisplayAPicture
//
//  Created by chenjie on 2020/2/18.
//  Copyright © 2020 chenjie. All rights reserved.
//

// 运行需要注意以下几点
// 1. 如果报动态库无法加载，首先确保已经安装OpenCV，然后请在工程选项-当前Target选项-General选项-Framework and libraries中检查所依赖的动态库，在文件资源的Frameworks中删除相应的动态库，再添加自己安装好的动态库。
// 2. 如果报动态库无法加载，并提示无动态库签名，请在终端使用命令“codesign -f -s "Mac Developer: <你的苹果开发者账号邮箱>" *.dylib“对某个动态库签名，由于动态库存在依赖问题，你也可以使用命令“codesign -f -s "Mac Developer: <你的苹果开发者账号邮箱>" /usr/local/opt/*/lib/*.dylib“对该目录下的所有动态库签名。
// 3. 如果在编译时OpenCV报头文件无法找到，请确保在工程选项-当前Target选项-Build Settings选项中搜索Header Search Paths，将其改为你自己在/usr/local下安装好的包含OpenCV2头文件集合的文件夹，需要注意是要设置为OpenCV2的上级目录，否则编译时会报错。
// 4. 如果运行时无法加载图片，请检查Edit Schem中的Argument选项设置正确，这是主函数的第二个参数，需要注意的是这里使用的是绝对路径， 你需要将其替换成自己环境中的绝对路径

#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

// 如果在使用CV库里的类或者函数时不想显示添加命名空间，可以解开下面这行代码注释
//using namespace cv;

void help(const char * argv[]) {
    std::cout << "\n"
    << "A simple OpenCV program that loads and displays an image from disk\n"
    << "Usecase as follows\n"
    << argv[0] <<" <path/filename>\n"
    << "For example:\n"
    << argv[0] << " ./fruits.jpg\n"
    << std::endl;
}

int main(int argc, const char * argv[]) {
    if (argc != 2) {
        help(argv);
    }
    
    // 从磁盘中读取图片数据到内存
    cv::Mat img = cv::imread(argv[1], cv::IMREAD_UNCHANGED);
    if (img.empty()) {
        return -1;
    }
    
    // 创建展示图片的窗口
    cv::namedWindow("Example-DisplayAPicture", cv::WINDOW_AUTOSIZE);
    // 在创建好的窗口上展示图片，如果不存在已经建立好的窗口，会隐式创建
    cv::imshow("Example-DisplayAPicture", img);
    // 等待用户敲击键盘上任意键
    cv::waitKey(0);
    // 摧毁窗口
    cv::destroyWindow("Example-DisplayAPicture");
    // 退出程序
    return 0;
}
