//
//  main.cpp
//  TrackBarContour
//
//  Created by chenjie on 2020/7/17.
//  Copyright © 2020 chenjie. All rights reserved.
//

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

// 原图灰度图
cv::Mat g_gray;
// 阈值处理后的二值图，用于查询轮廓
cv::Mat g_binary;
// 阈值处理使用的阈值
int g_thresh = 100;

// 滚动条的事件回调函数
void on_trackbar(int, void *) {
    // 生成二值图
    cv::threshold(g_gray, g_binary, g_thresh, 255, cv::THRESH_BINARY);
    cv::imshow("Binary", g_binary);

    // 获取轮廓
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(g_binary, contours, cv::noArray(), cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
    
    // 清空二值图
    g_binary = cv::Scalar::all(0);
    // 绘制轮廓
    cv::drawContours(g_binary, contours, -1, cv::Scalar::all(255));
    cv::imshow("Contours", g_binary);
}

int main(int argc, const char * argv[]) {
    // 读取原始图像
    g_gray = cv::imread(argv[1], cv::IMREAD_GRAYSCALE);
    cv::imshow("Original", g_gray);
    if (argc != 2 || g_gray.empty()) {
        std::cout << "\nExample: Find threshold dependent contours\nUsage:\n" << argv[0]
                  << " ../fruits.jpg\n" << std::endl;
        return -1;
    }
    
    // 创建UI控件
    cv::namedWindow("Contours", 1);
    cv::createTrackbar("Threshold", "Contours", &g_thresh, 255, on_trackbar);
    // 手动调用滑动条触发函数
    on_trackbar(g_thresh, nullptr);
    // 挂起程序，等待用户输入事件
    cv::waitKey();
    
    return 0;
}
