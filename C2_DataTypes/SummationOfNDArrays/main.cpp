//
//  main.cpp
//  SummationOfNDArrays
//
//  Created by chenjie on 2020/4/30.
//  Copyright © 2020 chenjie. All rights reserved.
//

#include <iostream>
#include <opencv2/opencv.hpp>

int main(int argc, const char * argv[]) {
    // 创建2个5*5*5的矩阵对象
    const int n_mat_size = 5;
    const int n_mat_sz[] = { n_mat_size, n_mat_size, n_mat_size };
    cv::Mat n_mat0( 3, n_mat_sz, CV_32FC1 );
    cv::Mat n_mat1( 3, n_mat_sz, CV_32FC1 );
    
    // 使用区间[0,1]的浮点型随机数填充着两个矩阵
    cv::RNG rng = cv::RNG();
    rng.fill(n_mat0, cv::RNG::UNIFORM, 0.0f, 1.0f);
    rng.fill(n_mat1, cv::RNG::UNIFORM, 0.0f, 1.0f);
    
    // 创建矩阵数组迭代器
    const cv::Mat *arrays[] = { &n_mat0, &n_mat1, 0 };
    cv::Mat my_planes[2];
    cv::NAryMatIterator it = cv::NAryMatIterator(arrays, my_planes);
    
    // 遍历所有平面并求两个矩阵所有元素的和
    float s = 0.0f;
    for (int p = 0; p < it.nplanes; p++, ++it) {
        // planes[]中每个元素依次为每个被遍历的数组当前平面的元素集合
        s += cv::sum(it.planes[0])[0];
        s += cv::sum(it.planes[1])[0];
    }
    
    std::cout << "Total across both volumes: " << s << std::endl;
    
    return 0;
}
