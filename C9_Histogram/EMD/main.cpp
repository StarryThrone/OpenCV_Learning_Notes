//
//  main.cpp
//  EMD
//
//  Created by chenjie on 2020/7/14.
//  Copyright © 2020 chenjie. All rights reserved.
//

#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

void help(const char * argv[]) {
    std::cout << "//\nExample. Ccompare signatures from histograms with all method;" << std::endl;
    std::cout << "\nCall is:\n"
              << argv[0] << "testImage0 testImage1 testImage2 badImage2\n\n"
              << "for example: " << argv[0]
              << " ../HandIndoorColor.jpg ../HandOutdoorColor.jpg "
              << "../HandOutdoorSunColor.jpg ../fruits.jpg\n"
              << "\n" << std::endl;
}

int main(int argc, const char * argv[]) {
    if (argc != 5) {
        help(argv);
        return -1;
    }
    
    // 加载室内手掌图像HandIndoorColor.jpg
    cv::Mat tmp = cv::imread(argv[1], 1);
    if (tmp.empty()) {
        std::cerr << "Error on reading image 1," << argv[1] << "\n" << std::endl;
        help(argv);
        return(-1);
    }
    
    // 计算图像TMP的尺寸
    cv::Size size = tmp.size();
    int width = size.width;
    int height = size.height;
    int halfheight = height >> 1;
    std::cout << "Getting size [[" << tmp.cols << "] [" << tmp.rows << "]]\n" << std::endl;
    std::cout << "Got size (w,h): (" << size.width << "," << size.height << ")" << std::endl;

    // 定义带处理的图像数据
    std::vector<cv::Mat> src(5);
    // 将室内手掌图像的分割为上下半幅共两幅图像
    src[0] = cv::Mat(cv::Size(width, halfheight), CV_8UC3);
    src[1] = cv::Mat(cv::Size(width, halfheight), CV_8UC3);
    
    cv::Mat_<cv::Vec3b>::iterator tmpit = tmp.begin<cv::Vec3b>();
    cv::Mat_<cv::Vec3b>::iterator s0it = src[0].begin<cv::Vec3b>();
    // 上半幅
    for (int i = 0; i < width*halfheight; ++i, ++tmpit, ++s0it) {
        *s0it = *tmpit;
    }
    // 下半幅
    cv::Mat_<cv::Vec3b>::iterator s1it = src[1].begin<cv::Vec3b>();
    for (int i = 0; i < width*halfheight; ++i, ++tmpit, ++s1it) {
        *s1it = *tmpit;
    }

    // 加载室外的两幅手掌图像以及一副完全无关的水果图像
    for (int i = 2; i<5; ++i) {
        src[i] = cv::imread(argv[i], 1);
        if (src[i].empty()) {
            std::cerr << "Error on reading image " << i << ": " << argv[i] << "\n" << std::endl;
            help(argv);
            return(-1);
        }
    }

    // 定义原始图像转换到HSV颜色空间中的图像hsv，以及其对应的直方图hist，和表示直方图的图像hist_img
    std::vector<cv::Mat> hsv(5), hist(5), hist_img(5);
    // 定义直方图在色度H和饱和度S维度的分组数
    int h_bins = 8;
    int s_bins = 8;
    int hist_size[] = {h_bins, s_bins};
    // 定义需要处理的原始数据通道索引
    int ch[] = {0, 1};
    // 定义直方图在两个维度的分组策略，OpenCV中的色度是减半的，因此取值范围为[0, 180)
    float h_ranges[] = {0, 180};
    float s_ranges[] = {0, 255};
    const float * ranges[] = {h_ranges, s_ranges};
    // 定义直方图中单个原始需要表示的正方形直径
    int scale = 10;
    
    for (int i = 0; i < 5; ++i) {
        // 转换颜色空间
        cv::cvtColor(src[i], hsv[i], cv::COLOR_BGR2HSV);
        // 计算直方图
        cv::calcHist(&hsv[i], 1, ch, cv::noArray(), hist[i], 2, hist_size, ranges, true);
        // 直方图标准化
        cv::normalize(hist[i], hist[i], 0, 255, cv::NORM_MINMAX);
        
        // 初始化表示直方图的图像
        hist_img[i] = cv::Mat::zeros(hist_size[0] * scale, hist_size[1] * scale, CV_8UC3);
        // 绘制表示直方图的图像
        for (int h = 0; h < hist_size[0]; h++) {
            for (int s = 0; s < hist_size[1]; s++) {
                float hval = hist[i].at<float>(h, s);
                cv::rectangle(hist_img[i],
                              cv::Rect(h*scale, s*scale, scale, scale),
                              cv::Scalar::all(hval), -1);
            }
        }
    }

    // 展示这些图像
    cv::namedWindow("Source0", 1);
    cv::imshow("Source0", src[0]);
    cv::namedWindow("HS Histogram0", 1);
    cv::imshow("HS Histogram0", hist_img[0]);

    cv::namedWindow("Source1", 1);
    cv::imshow("Source1", src[1]);
    cv::namedWindow("HS Histogram1", 1);
    cv::imshow("HS Histogram1", hist_img[1]);

    cv::namedWindow("Source2", 1);
    cv::imshow("Source2", src[2]);
    cv::namedWindow("HS Histogram2", 1);
    cv::imshow("HS Histogram2", hist_img[2]);

    cv::namedWindow("Source3", 1);
    cv::imshow("Source3", src[3]);
    cv::namedWindow("HS Histogram3", 1);
    cv::imshow("HS Histogram3", hist_img[3]);

    cv::namedWindow("Source4", 1);
    cv::imshow("Source4", src[4]);
    cv::namedWindow("HS Histogram4", 1);
    cv::imshow("HS Histogram4", hist_img[4]);
    
    // 将Source1计算得到的直方图分别使用4中方式和其他的直方图比较
    std::cout << "Comparison:\n"
              << "Corr                Chi                Intersect                Bhat\n" << std::endl;
    // 处理每个待比较的直方图
    for (int i = 1; i < 5; ++i) {
        std::cout << "Hist[0] vs Hist[" << i << "]: " << std::endl;
        // 处理每种不同的比较方法
        for (int j = 0; j < 4; ++j) {
            std::cout << "method[" << j << "]: " << cv::compareHist(hist[0], hist[i], j) << " ";
        }
        std::cout << std::endl;
    }
    
    // 计算EMD距离
    std::vector<cv::Mat> sig(5);
    std::cout << "\nEMD: " << std::endl;

    // 将直方图数据转换为签名
    for (int i = 0; i < 5; ++i) {
        std::vector<cv::Vec3f> sigv;
        // 重新标准化直方图，使其所有元素的和为1
        cv::normalize(hist[i], hist[i], 1, 0, cv::NORM_L1);
        for (int h = 0; h < h_bins; h++) {
            for (int s = 0; s < s_bins; s++) {
                float bin_val = hist[i].at<float>(h, s);
                // 在计算EMD距离是，值为0的分组可以不需要统计，它并不会影响最终的结果
                if (bin_val != 0) {
                    sigv.push_back(cv::Vec3f(bin_val, (float)h, (float)s));
                }
            }
        }

        // 将向量数转换为N✖️3的矩阵，N是直方图中数值非0点组的个数
        sig[i] = cv::Mat(sigv).clone().reshape(1);
        if (i > 0) {
            std::cout << "Hist[0] vs Hist[" << i << "]: "
                      << EMD(sig[0], sig[i], cv::DIST_L2) << std::endl;
        }
    }

    cv::waitKey(0);
    
    return 0;
}
