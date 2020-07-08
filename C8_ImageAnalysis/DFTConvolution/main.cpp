//
//  main.cpp
//  C8_ImageAnalysis
//
//  Created by chenjie on 2020/7/8.
//  Copyright © 2020 chenjie. All rights reserved.
//

#include <iostream>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

int main(int argc, const char * argv[]) {
    if (argc != 2) {
        std::cout << "\nExample. Using cv::dft() and cv::idft() to accelerate the"
                  << "\n computation of convolutions"
                  << "\nFourier Transform\nUsage: "
                  << argv[0] << " <path/imagename>\n" << std::endl;
        return -1;
    }
    
    // 读取图像
    cv::Mat A = cv::imread(argv[1], 0);
    if (A.empty()) {
        std::cout << "Cannot load " << argv[1] << std::endl;
        return -1;
    }
    
    cv::Size patchSize(100, 100);
    cv::Point topleft(A.cols / 2, A.rows /2);
    cv::Rect roi(topleft.x, topleft.y, patchSize.width, patchSize.height);
    // 选中其中的一小块图像，需要注意实际的数据段仍然是共用的
    cv::Mat B = A(roi);
    
    // 获取离散傅立叶变换的最佳向量长度
    int dft_M = cv::getOptimalDFTSize(A.rows + B.rows - 1);
    int dft_N = cv::getOptimalDFTSize(A.cols + B.cols - 1);
    // 使用最佳尺寸创建待处理的样本矩阵
    cv::Mat dft_A = cv::Mat::zeros(dft_M, dft_N, CV_32F);
    cv::Mat dft_B = cv::Mat::zeros(dft_M, dft_N, CV_32F);
    // 映射内部实际有效数据矩阵，共用数据段内存
    cv::Mat dft_A_part = dft_A(cv::Rect(0, 0, A.cols, A.rows));
    cv::Mat dft_B_part = dft_B(cv::Rect(0, 0, B.cols, B.rows));
    // 将原图A和选择的区块B映射到共用的数据段内存，即此时dft_A和dft_B被填充对应数据
    // 此次平移各自矩阵中的均值单位
    A.convertTo(dft_A_part, dft_A_part.type(), 1, -mean(A)[0]);
    B.convertTo(dft_B_part, dft_B_part.type(), 1, -mean(B)[0]);
    
    // 执行傅立叶变换
    cv::dft(dft_A, dft_A, 0, A.rows);
    // 需要注意dft_B在执行傅立叶变换之前有效的数据数为100*100，在执行完成后整个矩阵都包含有效数据，推测内部对原始的频域值做了插值运算，以便接下来方便计算卷积
    cv::dft(dft_B, dft_B, 0, B.rows);
    
    // 将最后一个参数设置伟false则计算卷积，否则计算相关性
    cv::mulSpectrums(dft_A, dft_B, dft_A, 0, true);
    // 此处卷积并未使用前文常见的任何滤波器，如高斯滤波器，暂不讨论具体含义
//    cv::mulSpectrums(dft_A, dft_B, dft_A, 0, false);
    cv::idft(dft_A, dft_A, cv::DFT_SCALE, A.rows + B.rows - 1);

    // 获取相关性矩阵的有效数据区域
    cv::Mat corr = dft_A(cv::Rect(0, 0, A.cols + B.cols - 1, A.rows + B.rows - 1));
    // 标准话矩阵原始
    cv::normalize(corr, corr, 0, 1, cv::NORM_MINMAX, corr.type());
    // 取三次幂，提高对比度便于查看
    cv::pow(corr, 3.0, corr);

    // 对分割出的小块区域执行异或运算，等效于黑白反色
    B ^= cv::Scalar::all(255);
    
    // 显示处理结果
    cv::imshow("Image", A);
    // 显示相关性，相关性在后面章节中会详细介绍，新增可以简单的理解为函数的相似程度，即图像的相似程度
    cv::imshow("Correlation", corr);
    
    // 挂起程序等待用户输入任意键
    cv::waitKey();
    
    return 0;
}
