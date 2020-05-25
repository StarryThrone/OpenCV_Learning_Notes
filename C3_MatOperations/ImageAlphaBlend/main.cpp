//
//  main.cpp
//  ImageAlphaBlend
//
//  Created by chenjie on 2020/5/25.
//  Copyright © 2020 chenjie. All rights reserved.
//

#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

void help(const char * argv[]) {
    std::cout << "\n\n"
    << "This program alpha blends the first image onto the second and needs six parameters.\n"
    << "First: The path of source image.\n"
    << "Second: The path of destinate image.\n"
    << "Third and fourth: The original location of blend area in destinate image as x and y respectively.\n"
    << "Fifth and sixth: The blending coefficients of source and destinate image. \n"
    << std::endl;
}

int main(int argc, const char * argv[]) {
    help(argv);
    if (argc != 7) {
        std::cout << "Error: You have to set 7 parameters." << std::endl;
        return -1;
    }
    
    // 图像Src1的数据会被混合到图像Src2指定中
    cv::Mat src = cv::imread(argv[1], cv::IMREAD_COLOR);
    cv::Mat dst = cv::imread(argv[2], cv::IMREAD_COLOR);
    if (src.empty() || dst.empty()) {
        std::cout << "Error: Source image is empty." << std::endl;
        return -2;
    }
    // 计算原始图像宽高
    int src_w = src.size().width;
    int src_h = src.size().height;
    int dst_w = dst.size().width;
    int dst_h = dst.size().height;

    // 获取在Src2中混合的起点位置
    int x = atoi(argv[3]);
    int y = atoi(argv[4]);
    if (x < 0 || y < 0 || x > dst_w - 1 || y > dst_h - 1 || x + src_w > dst_w - 1 || y + src_h > dst_h) {
        std::cout << "Error: Blend area is invalid." << std::endl;
        return -3;
    }

    // 获取混合的参数
    double alpha = atof(argv[5]);
    double beta = atof(argv[6]);

    // 混合图像
    cv::Mat target = cv::Mat(dst, cv::Rect(x, y, src_w, src_h));
    cv::addWeighted(src, alpha, target, beta, 0, target);

    // 显示图像
    cv::namedWindow("Alpha Blend", 1);
    cv::imshow("Alpha Blend", dst);
    
    // 等待任意键盘事件
    cv::waitKey(0);
    
    return 0;
}
