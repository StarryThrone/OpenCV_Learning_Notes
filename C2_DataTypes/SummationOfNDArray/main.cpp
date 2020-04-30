//
//  main.cpp
//  SummationOfNDArray
//
//  Created by chenjie on 2020/4/29.
//  Copyright © 2020 chenjie. All rights reserved.
//

#include <iostream>
#include <opencv2/opencv.hpp>

int main(int argc, const char * argv[]) {
    // 创建5*5*5的矩阵，每个元素类型为单通道32位浮点型
    const int n_mat_size = 5;
    const int n_mat_sz[] = { n_mat_size, n_mat_size, n_mat_size };
    cv::Mat n_mat = cv::Mat(3, n_mat_sz, CV_32FC1);

    // 使用均匀分布类型随机数填充矩阵，随机数取值区间为[0.0f, 1.0f]
    cv::RNG rng = cv::RNG();
    rng.fill(n_mat, cv::RNG::UNIFORM, 0.0f, 1.0f);
    
    // 创建数组矩阵迭代器
    const cv::Mat* arrays[] = {&n_mat, 0};
    // 这里的参数my_planes是用于迭代器保存多个数组当前被迭代的平面数组，即每次迭代
    // 后该数组内部的值都会更新
    cv::Mat my_planes[1];
    cv::NAryMatIterator it = cv::NAryMatIterator(arrays, my_planes);
    
    // 求矩阵中所有元素的和
    float s = 0.0f;
    for (int p = 0; p < it.nplanes; p++, ++it) {
        // 这里使用it.planes和my_planes等价，均表示多个数组的当前平面组成的数组
        // 由于只迭代了1个数组，因此取索引值0的平面
        s += cv::sum(it.planes[0])[0];
    }
    
    std::cout << "Total across entire volume: " << s << std::endl;
}
