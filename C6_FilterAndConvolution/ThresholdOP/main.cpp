//
//  main.cpp
//  ThresholdOP
//
//  Created by chenjie on 2020/6/15.
//  Copyright © 2020 chenjie. All rights reserved.
//

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

void sum_rgb(const cv::Mat& src, cv::Mat& dst) {
    // 将图片数据分割到三个独立的矩阵中
    std::vector<cv::Mat> planes;
    cv::split(src, planes);
    cv::Mat b = planes[0], g = planes[1], r = planes[2];

    // 计算三个通道的和
    cv::Mat s = cv::Mat::zeros(b.size(), CV_32F);
    cv::accumulate(b, s);
    cv::accumulate(g, s);
    cv::accumulate(r, s);

    // 将像素值截取到100
    cv::threshold(s, s, 100, 100, cv::THRESH_TRUNC);
    s.convertTo(dst, b.type());
}

void help(const char * argv[]) {
    std::cout << "\nExample. Alternative method to combine and threshold image planes\n" << std::endl;
    std::cout << "\nCall:\n" << argv[0] << " ../faces.jpg\n" << std::endl;
}

int main(int argc, const char * argv[]) {
    help(argv);
    if (argc < 2) {
        std::cout << "specify input image" << std::endl;
        return -1;
    }

    // 载入图像
    cv::Mat src = cv::imread(argv[1]);
    cv::Mat dst;
    if (src.empty()) {
        std::cout << "can not load " << argv[1] << std::endl;
        return -1;
    }
    sum_rgb(src, dst);

    // 使用文件名创窗口，并在其中显示图片
    cv::imshow(argv[1], dst);

    // 挂起程序直至输入任意键
    cv::waitKey(0);

    return 0;
}
