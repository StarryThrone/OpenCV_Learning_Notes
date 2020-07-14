//
//  main.cpp
//  Computation
//
//  Created by chenjie on 2020/7/13.
//  Copyright © 2020 chenjie. All rights reserved.
//

#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

int main(int argc, const char * argv[]) {
    if (argc != 2) {
        std::cout << "\n// Example 13-1. Histogram computation and display" << std::endl;
        std::cout << "\nComputer Color Histogram\nUsage: " << argv[0] << " <imagename>\n" << std::endl;
        return -1;
    }
    
    // 读取原图
    cv::Mat src = cv::imread(argv[1], cv::IMREAD_COLOR);
    if (src.empty()) {
        std::cout << "Cannot load " << argv[1] << std::endl;
        return -1;
    }

    // 构建HSV颜色空间数据
    cv::Mat hsv;
    cv::cvtColor(src, hsv, cv::COLOR_BGR2HSV);

    // 设定二维直方图中的分组策略
    // HSV标准定义色调H取值区域为[0, 360)，S和V取值区域为[0, 1]，而在OpenCV中为了能够用8位统一表示，H减半处理及，OpenCV中HSV的H值仅为实际值的一半，因此其取值范围为[0, 180)，而S和V的取值为[0, 255]
    float h_ranges[] = {0, 180};
    float s_ranges[] = {0, 256};
    const float * ranges[] = {h_ranges, s_ranges};
    int histSize[] = {30, 32};
    int ch[] = {0, 1};

    // 计算二维直方图
    cv::Mat hist;
    cv::calcHist(&hsv, 1, ch, cv::noArray(), hist, 2, histSize, ranges, true);
    // 标准化处理直方图
    cv::normalize(hist, hist, 0, 255, cv::NORM_MINMAX);
    
    // 定义单个数据点表示的方块直径
    int scale = 10;
    // 创建显示二维直方图的图像
    cv::Mat hist_img(histSize[0] * scale, histSize[1] * scale, CV_8UC3);
    // 在图像hist_img中绘制scale✖️scale像素的方块表示二维直方图hist中的每个数据点
    for (int h = 0; h < histSize[0]; h++) {
        for (int s = 0; s < histSize[1]; s++) {
            float hval = hist.at<float>(h, s);
            cv::rectangle(hist_img,
                          cv::Rect(h * scale, s * scale, scale, scale),
                          cv::Scalar::all(hval), -1);
        }
    }
    
    // 显示图像
    cv::imshow("image", src);
    cv::imshow("H-S histogram", hist_img);
    
    // 挂起程序等待用户输入任意键
    cv::waitKey();

    return 0;
}
