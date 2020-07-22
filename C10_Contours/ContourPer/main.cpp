//
//  main.cpp
//  ContourPer
//
//  Created by chenjie on 2020/7/21.
//  Copyright © 2020 chenjie. All rights reserved.
//

#include <iostream>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

// 用于比较两个轮廓的结构体
struct AreaCmp {
public:
    // 构造函数
    AreaCmp(const std::vector<float>& _areas) : areas(&_areas) {}

    // 重载std::sort()函数需要使用到的运算符
    bool operator()(int a, int b) const {
        return (*areas)[a] > (*areas)[b];
    }

private:
    // 保存所有轮廓的区域
    const std::vector<float>* areas;
};


int main(int argc, const char * argv[]) {
    // 加载图片
    cv::Mat img = cv::imread(argv[1], cv::IMREAD_GRAYSCALE);
    if (argc != 2 || img.empty()) {
        std::cout << "\nERROR: You need 2 parameters, you had " << argc << "\n" << std::endl;
        std::cout << "\nExample: Drawing Contours\nCall is:\n" << argv[0] << " <image>\n\n"
                  << "Example:\n" << argv[0] << " ../box.png\n" << std::endl;
        return -1;
    }
    
    // 查找图像中的边缘
    cv::Mat img_edge;
    cv::threshold(img, img_edge, 128, 255, cv::THRESH_BINARY);
    cv::imshow("Image after threshold", img_edge);
    
    // 查找轮廓
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(img_edge, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
    std::cout << "\n\nHit any key to draw the next contour, ESC to quit\n\n";
    std::cout << "Total Contours Detected: " << contours.size() << std::endl;

    // 根据轮廓的面积降序排序
    std::vector<int> sortIdx(contours.size());
    std::vector<float> areas(contours.size());
    for (int n = 0; n < (int)contours.size(); n++) {
        sortIdx[n] = n;
        areas[n] = cv::contourArea(contours[n], false);
    }
    std::sort(sortIdx.begin(), sortIdx.end(), AreaCmp(areas));
    
    // 绘制单条轮廓
    cv::Mat img_color;
    for (int n = 0; n < (int)sortIdx.size(); n++) {
        int idx = sortIdx[n];
        cv::cvtColor(img, img_color, cv::COLOR_GRAY2BGR);
        // Try different values of max_level, and see what happens
        cv::drawContours(img_color, contours, idx,
                         cv::Scalar(0,0,255), 2, 8, hierarchy, 0);
        std::cout << "Contour #" << idx << ": area=" << areas[idx]
                  << ", nvertices=" << contours[idx].size() << std::endl;
        cv::imshow(argv[0], img_color);
        int key = cv::waitKey();
        // 如果输入ESC键，则退出循环
        if ((key & 255) == 27) {
            break;
        }
    }
    
    std::cout << "Finished all contours\n";

    return 0;
}
