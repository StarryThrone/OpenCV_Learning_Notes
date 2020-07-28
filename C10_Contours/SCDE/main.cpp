//
//  main.cpp
//  SCDE
//
//  Created by chenjie on 2020/7/25.
//  Copyright © 2020 chenjie. All rights reserved.
//

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/shape.hpp>
#include <opencv2/highgui.hpp>
#include <algorithm>
#include <random>

/// 提取图片轮廓，并随机采样顶点
/// - Parameters:
///   - image: 待采样的图片
///   - n: 采样顶点数
static std::vector<cv::Point> sampleContour(const cv::Mat& image, int n = 300) {
    // 查找所有的轮廓
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(image, contours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);
    
    // 这里提取出第一条轮廓的所有顶点，由于准备的图片只能提取出一张轮廓，因此得到的就是想要寻找的轮廓
    std::vector<cv::Point> all_points;
    for (size_t j = 0; j < contours[0].size(); j++) {
        all_points.push_back(contours[0][j]);
    }

    // 如果单条轮廓的顶点数量小于n，则重复该条轮廓已有的顶点，直至轮廓数量等于N
    int dummy = 0;
    for (int add = (int)all_points.size(); add < n; add++) {
        all_points.push_back(all_points[dummy++]);
    }

    // 使用随机顺序排列所有的顶点
    unsigned seed = (unsigned)std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(all_points.begin(), all_points.end(), std::default_random_engine (seed));
    
    // 随机采样轮廓的n个顶点
    std::vector<cv::Point> sampled;
    for (int i = 0; i < n; i++) {
        sampled.push_back(all_points[i]);
    }
    return sampled;
}

void help(const char * argv[]) {
    std::cout << "\nExample. Using the shape context distance extractor"
              << "\nNOTE: See images to try in ../shape_sample/\n"
              << "\nCall:\n" << argv[0] << " <path/image_1> <path/image2>\n"
              << "\nMISSMATCH Example:\n" << argv[0] << "  ../shape_sample/1.png ../shape_sample/3.png\n"
              << "\nMATCH Example:\n" << argv[0] << "  ../shape_sample/3.png ../shape_sample/4.png\n"
              << std::endl;
}

int main(int argc, const char * argv[]) {
    help(argv);
    if (argc != 3) {
        std::cout << "\nERROR: you need 2 parameters, you had " << argc << " parameters.\n" << std::endl;
        return -1;
    }

    // 读取待比较的两个图片
    cv::Mat img1 = imread(argv[1], cv::IMREAD_GRAYSCALE);
    cv::Mat img2 = imread(argv[2], cv::IMREAD_GRAYSCALE);
    // 分别计算两个图片的随机采样轮廓顶点
    std::vector<cv::Point> c1 = sampleContour(img1);
    std::vector<cv::Point> c2 = sampleContour(img2);
    
    // 比较两个形状的距离
    cv::Ptr<cv::ShapeContextDistanceExtractor> mysc = cv::createShapeContextDistanceExtractor();
    // 可能是由于XCode使用的编译器是Clang + LLVM，使得程序运行时报动态库符号绑定错误：Symbol not found: ___emutls_get_address
    float dis = mysc->computeDistance(c1, c2);
    std::cout << "shape context distance between "
              << argv[1] << " and " << argv[2] << " is: " << dis << std::endl;
    
    // 显示两个形状
    cv::imshow("SHAPE #1", img1);
    cv::imshow("SHAPE #2", img2);
    
    // 挂起程序等待用户输入
    cv::waitKey();

    return 0;
}
