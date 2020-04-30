//
//  main.cpp
//  PrintSparseArray
//
//  Created by chenjie on 2020/4/30.
//  Copyright © 2020 chenjie. All rights reserved.
//

#include <iostream>
#include <opencv2/opencv.hpp>

int main(int argc, const char * argv[]) {
    // 创建稀疏矩阵
    int size[] = {10, 10};
    cv::SparseMat sm = cv::SparseMat(2, size, CV_32FC1);
    
    // 为稀疏矩阵填充数据
    cv::RNG rng = cv::RNG();
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            int idx[2] = {i, j};
            // 通过条件控制仅仅填充部分数据
            if (j == 0) {
                sm.ref<float>(idx) = rng.uniform(0.0f, 1.0f);
            }
        }
    }
    
    // 创建矩阵迭代器
    cv::SparseMatConstIterator_<float> it = sm.begin<float>();
    cv::SparseMatConstIterator_<float> it_end = sm.end<float>();
    
    for (; it != it_end; ++it) {
        const cv::SparseMat::Node *node = it.node();
        printf(" (%d, %d) %f\n", node->idx[0], node->idx[1], *it);
    }
    
    return 0;
}
