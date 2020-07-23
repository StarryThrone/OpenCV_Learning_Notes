//
//  main.cpp
//  ConnectedComponents
//
//  Created by chenjie on 2020/7/23.
//  Copyright © 2020 chenjie. All rights reserved.
//

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

int main(int argc, const char * argv[]) {
    // 加载原始图片
    cv::Mat img = cv::imread(argv[1], cv::IMREAD_GRAYSCALE);
    if ((argc != 2) || img.empty()) {
        std::cout << "\nERROR: You need 2 parameters, you had " << argc << "\n" << std::endl;
        std::cout << "\nExample: Drawing labeled connected componnents\n"
                  << "Call is:\n" <<argv[0] << " <path/image>\n"
                  << "\nExample:\n" << argv[0] << " ../HandIndoorColor.jpg\n" << std::endl;
        return -1;
    }
    cv::imshow("Source Image", img);
    
    // 生成阈值图
    cv::Mat img_edge;
    cv::threshold(img, img_edge, 128, 255, cv::THRESH_BINARY);
    cv::imshow("Image after threshold", img_edge);
    
    // 分析连通区域
    cv::Mat labels, stats, centroids;
    int nccomps = cv::connectedComponentsWithStats(img_edge, labels, stats, centroids);
    std::cout << "Total Connected Components Detected: " << nccomps << std::endl;

    // 为每个连通区域分配一个随机颜色，labels中的标记对应为颜色表内的索引
    std::vector<cv::Vec3b> colors(nccomps + 1);
    // label为0的连通区域是背景区域（即在待分析图像中就是黑色部分），设置为黑色
    colors[0] = cv::Vec3b(0,0,0);
    for (int i = 1; i <= nccomps; i++) {
        // 面积如果小于100，则设置为黑色
        if (stats.at<int>(i-1, cv::CC_STAT_AREA) < 100) {
            colors[i] = cv::Vec3b(0,0,0);
        } else {
            colors[i] = cv::Vec3b(rand()%256, rand()%256, rand()%256);
        }
    }

    // 绘制连通区域分析结果图像
    cv::Mat img_color = cv::Mat::zeros(img.size(), CV_8UC3);
    for (int y = 0; y < img_color.rows; y++) {
      for (int x = 0; x < img_color.cols; x++) {
          int label = labels.at<int>(y, x);
          CV_Assert(0 <= label && label <= nccomps);
          img_color.at<cv::Vec3b>(y, x) = colors[label];
      }
    }

    // 展示连通区域分析结果
    cv::imshow("Labeled map", img_color);
    
    // 挂起程序，等待用户输入
    cv::waitKey();

    return 0;
}
