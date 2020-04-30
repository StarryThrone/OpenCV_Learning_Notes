//
//  main.cpp
//  PrintMatrixWithTemplate
//
//  Created by chenjie on 2020/4/30.
//  Copyright © 2020 chenjie. All rights reserved.
//

#include <iostream>
#include <opencv2/opencv.hpp>

template <class T>
void print_matrix(const cv::SparseMat_<T> *sm) {
    // 创建数组迭代器
    cv::SparseMatConstIterator_<T> it = sm->begin();
    cv::SparseMatConstIterator_<T> it_end = sm->end();
    
    // 打印所有非零元素
    for (; it != it_end; ++it) {
        const typename cv::SparseMat_<T>::Node *node = it.node();
        std::cout << "(" << node->idx[0] << ", " << node->idx[1]
                  << " ) = " << *it << std::endl;
    }
}

// 使用模版稀疏矩阵
void calling_function1() {
    // 创建模版稀疏矩阵
    int ndim = 2;
    int size[] = {4, 4};
    cv::SparseMat_<float> sm = cv::SparseMat_<float>(ndim, size);
    
    // 填充数据
    cv::RNG rng = cv::RNG();
    for (int i = 0; i < 10; i++) {
        int idx[2] = {i, 0};
        sm.ref(idx) = rng.uniform(0.0f, 1.0f);
    }
    
    // 打印数据
    print_matrix(&sm);
}

// 使用稀疏矩阵
void calling_function2() {
    // 创建模版稀疏矩阵
    int ndim = 2;
    int size[] = {4, 4};
    cv::SparseMat sm = cv::SparseMat(ndim, size, CV_32FC1);
    
    // 填充数据
    cv::RNG rng = cv::RNG();
    for (int i = 0; i < 10; i++) {
        int idx[2] = {i, 0};
        sm.ref<float>(idx) = rng.uniform(0.0f, 1.0f);
    }

    // 打印数据
    print_matrix((cv::SparseMat_<float> *)&sm);
}

int main(int argc, const char * argv[]) {
    std::cout << "Case 1:" << std::endl;
    calling_function1();
    
    std::cout << "Case 2:" << std::endl;
    calling_function2();
    
    return 0;
}
