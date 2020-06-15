//
//  main.cpp
//  ADThreshold
//
//  Created by chenjie on 2020/6/15.
//  Copyright © 2020 chenjie. All rights reserved.
//

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

int main(int argc, const char * argv[]) {
    if (argc != 7) {
        std::cout << "\nExample. Threshold versus adaptive threshold\n Usage:\n"
                  << argv[0] << " fixed_threshold invert(0=off|1=on) "
                  "adaptive_type(0=mean|1=gaussian) block_size offset image\n"
                  "Example:\n" << argv[0]
                  << " 100 1 0 15 10 ../faces.png\n";
        return -1;
    }

    // 解析命令行参数
    double fixed_threshold = (double)atof(argv[1]);
    int threshold_type = atoi(argv[2]) ? cv::THRESH_BINARY : cv::THRESH_BINARY_INV;
    int adaptive_method = atoi(argv[3]) ? cv::ADAPTIVE_THRESH_MEAN_C : cv::ADAPTIVE_THRESH_GAUSSIAN_C;
    int block_size = atoi(argv[4]);
    double offset = (double)atof(argv[5]);
    // 读取灰度图
    cv::Mat Igray = cv::imread(argv[6], cv::IMREAD_GRAYSCALE);
    if (Igray.empty()) {
        std::cout << "Can not load " << argv[6] << std::endl;
        return -1;
    }

    // 声明处理后的图像对象
    cv::Mat It, Iat;
    // 阈值处理
    cv::threshold(Igray, It, fixed_threshold, 255, threshold_type);
    cv::adaptiveThreshold(Igray, Iat, 255, adaptive_method, threshold_type, block_size, offset);
        
    // 显示结果
    cv::imshow("Raw", Igray);
    cv::imshow("Threshold", It);
    cv::imshow("Adaptive Threshold", Iat);
    // 挂起程序
    cv::waitKey(0);

    return 0;
}
