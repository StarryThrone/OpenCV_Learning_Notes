//
//  main.cpp
//  HoughCircles
//
//  Created by chenjie on 2020/7/10.
//  Copyright © 2020 chenjie. All rights reserved.
//

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

void help(const char * argv[]) {
    std::cout << "\nExample. Using cv::dft() and cv::idft() to accelerate the computation of convolutions"
              << "\nHough Circle detect\nUsage: " << argv[0] <<" <path/imagename>\n"
              << "Example:\n" << argv[0] << " ../stuff.jpg\n" << std::endl;
}

int main(int argc, const char * argv[]) {
    help(argv);
    if (argc != 2) {
        return -1;
    }

    // 读取原始图像
    cv::Mat src = cv::imread(argv[1], cv::IMREAD_COLOR);
    if (src.empty()) {
        std::cout << "Cannot load " << argv[1] << std::endl;
        return -1;
    }
    
    cv::Mat image;
    // 转换为灰度图像
    cv::cvtColor(src, image, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(image, image, cv::Size(5, 5), 0, 0);
    
    // 应用霍夫圆变换
    std::vector<cv::Vec3f> circles;
    cv::HoughCircles(image, circles, cv::HOUGH_GRADIENT, 2, image.cols/4);

    // 使用霍夫圆变换的结果在原图中绘制圆形
    for (size_t i = 0; i < circles.size(); ++i) {
        cv::circle(src,
                   cv::Point(cvRound(circles[i][0]), cvRound(circles[i][1])),
                   cvRound(circles[i][2]),
                   cv::Scalar(0, 0, 255, 1),
                   2, cv::LINE_AA);
    }
    
    // 显示霍夫圆变换的结果
    cv::imshow("Hough Circles", src);
    
    // 挂起程序等待用户输入任意键
    cv::waitKey(0);

    return 0;
}
