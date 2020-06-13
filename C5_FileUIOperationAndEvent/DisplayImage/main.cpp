//
//  main.cpp
//  DisplayImage
//
//  Created by chenjie on 2020/6/13.
//  Copyright © 2020 chenjie. All rights reserved.
//

#include <iostream>
#include <opencv2/highgui.hpp>

void help(const char * argv[]) {
    std::cout << "\n"
              << "Example: Create a window and display an image\nCall:\n"
              << argv[0] << " image\n" << std::endl;
}

int main(int argc, const char * argv[]) {
    help(argv);
    if(argc != 2) {
        std::cout << "You need to supply an image path/name" << std::endl;
        return -1;
    }
    
    // 使用图片名称创建窗口
    cv::namedWindow(argv[1], 1);
    // 加载图片
    cv::Mat img = cv::imread(argv[1]);
    // 在窗口中显示图片
    cv::imshow(argv[1], img);
    // 持续挂起应用，直至输入任意键
    cv::waitKey(0);
    // 销毁窗口，释放资源
    cv::destroyWindow(argv[1]);

    return 0;
}
