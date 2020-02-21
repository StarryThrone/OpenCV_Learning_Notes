//
//  main.cpp
//  PyramidImage
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
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

const std::string c_window_name_in = "Pyramid_Image_In";
const std::string c_window_name_out = "Pyramid_Image_Out";

void help(const char * argv[]) {
    std::cout << "\n"
    << "Demo: Downsample image by half size \n"
    << argv[0] << " <path/image>\n"
    << "For example:\n"
    << argv[0] << " ./faces.png\n"
    << std::endl;
}

int main(int argc, const char * argv[]) {
    if (argc != 2) {
        help(argv);
        return 0;
    }
    
    // 显示原始图片
    cv::namedWindow(c_window_name_in, cv::WINDOW_AUTOSIZE);
    cv::Mat imageIn = cv::imread(argv[1]);
    cv::imshow(c_window_name_in, imageIn);
    
    // 显示使用一半尺寸下采样处理后的图片
    cv::namedWindow(c_window_name_out, cv::WINDOW_NORMAL);
    cv::Mat imageOut;
    cv::pyrDown(imageIn, imageOut);
    cv::imshow(c_window_name_out, imageOut);
        
    // 挂起程序，等待用户键盘输入
    cv::waitKey(0);
    
    // 摧毁窗口对象，释放内存资源
    cv::destroyWindow(c_window_name_in);
    cv::destroyWindow(c_window_name_out);
    
    return 0;
}
