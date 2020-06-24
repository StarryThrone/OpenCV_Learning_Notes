//
//  main.cpp
//  AffineTransform
//
//  Created by chenjie on 2020/6/24.
//  Copyright © 2020 chenjie. All rights reserved.
//

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

int main(int argc, const char * argv[]) {
    if (argc != 2) {
      std::cout << "Warp affine\nUsage: " << argv[0] << " <imagename>\n" << std::endl;
      return -1;
    }
    
    // 读取原图
    cv::Mat src = cv::imread(argv[1], cv::IMREAD_COLOR);
    if (src.empty()) {
        std::cout << "Can not load " << argv[1] << std::endl;
        return -1;
    }

    // 定义旋转之前图像轮廓顶点坐标
    cv::Point2f srcTri[] = {
        cv::Point2f(0,0),               // 左上
        cv::Point2f(src.cols - 1, 0),   // 右上
        cv::Point2f(0, src.rows - 1)    // 左下
    };
    // 定义旋转之后图像轮廓顶点坐标
    cv::Point2f dstTri[] = {
        cv::Point2f(src.cols * 0.f, src.rows * 0.33f),   // 左上
        cv::Point2f(src.cols * 0.85f, src.rows * 0.25f), // 右上
        cv::Point2f(src.cols * 0.15f, src.rows * 0.7f)   // 左下
    };

    // 计算仿射矩阵
    cv::Mat warp_mat = cv::getAffineTransform(srcTri, dstTri);
    cv::Mat dst, dst2;
    // 应用仿射变换
    cv::warpAffine(src, dst, warp_mat, src.size(),
                   cv::INTER_LINEAR,
                   cv::BORDER_CONSTANT, cv::Scalar());

    // 在仿射变换处理后的图像上目标顶点上绘制圆形
    for (int i = 0; i < 3; ++i) {
        cv::circle(dst, dstTri[i], 5, cv::Scalar(255, 0, 255), -1, cv::LINE_AA);
    }

    // 显示第一次仿射变换的结果
    cv::imshow("Affine Transform Test", dst);
    // 刮起程序直至输入任意键
    cv::waitKey();

    // 定义旋转中心
    cv::Point2f center(src.cols * 0.5f, src.rows * 0.5f);
    // 不断旋转缩放图像
    for (int frame = 0; ; ++frame) {
        // 定义旋转角度和在xy轴上的缩放系数
        double angle = frame * 3 % 360;
        double scale = (cos((angle - 60) * CV_PI/180) + 1.05) * 0.8;
        // 计算旋转缩放矩阵
        cv::Mat rot_mat = cv::getRotationMatrix2D(center, angle, scale);
        
        // 应用仿射变换
        cv::warpAffine(src, dst, rot_mat, src.size(),
                       cv::INTER_LINEAR,
                       cv::BORDER_CONSTANT, cv::Scalar());
        // 显示旋转后的图像
        cv::imshow("Rotated Image", dst);
        // 等待用户输入任意键结束程序
        if (cv::waitKey(30) >= 0) {
            break;
        }
    }
    
    return 0;
}
