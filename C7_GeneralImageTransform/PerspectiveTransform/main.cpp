//
//  main.cpp
//  PerspectiveTransform
//
//  Created by chenjie on 2020/6/25.
//  Copyright © 2020 chenjie. All rights reserved.
//

#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

int main(int argc, const char * argv[]) {
    if (argc != 2) {
        std::cout << "Perspective Warp\nUsage: " << argv[0] << " <imagename>\n" << std::endl;
        return -1;
    }

    // 读取图像
    cv::Mat src = cv::imread(argv[1], cv::IMREAD_COLOR);
    if (src.empty()) {
        std::cout << "Can not load " << argv[1] << std::endl;
        return -1;
    }
    
    // 定义旋转之前图像轮廓顶点坐标
    cv::Point2f srcQuad[] = {
        cv::Point2f(0, 0),                   // src 左上
        cv::Point2f(src.cols - 1, 0),          // src 右上
        cv::Point2f(src.cols - 1, src.rows - 1), // src 右下
        cv::Point2f(0, src.rows - 1)           // src 左下
    };
    // 定义旋转之后图像轮廓顶点坐标
    cv::Point2f dstQuad[] = {
        cv::Point2f(src.cols * 0.05f, src.rows * 0.33f),
        cv::Point2f(src.cols * 0.9f, src.rows * 0.25f),
        cv::Point2f(src.cols * 0.8f, src.rows * 0.9f),
        cv::Point2f(src.cols * 0.2f, src.rows * 0.7f)
    };
    
    // 计算投影矩阵
    cv::Mat warp_mat = cv::getPerspectiveTransform(srcQuad, dstQuad);
    cv::Mat dst;
    // 应用投影变换
    cv::warpPerspective(src, dst, warp_mat, src.size(), cv::INTER_LINEAR,
                        cv::BORDER_CONSTANT, cv::Scalar());

    // 在仿射变换处理后的图像上目标顶点上绘制圆形
    for (int i = 0; i < 4; i++) {
        cv::circle(dst, dstQuad[i], 5, cv::Scalar(255, 0, 255), -1, cv::LINE_AA);
    }

    // 显示图像
    cv::imshow("Perspective Transform Test", dst);
    // 挂起程序直至用户输入任意键
    cv::waitKey();
    
    return 0;
}
