//
//  main.cpp
//  KMeans
//
//  Created by RichardJieChen on 2021/12/16.
//

#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

static void help(const char * argv[]) {
    std::cout << "\nThis program demonstrates kmeans clustering.\n"
                 " It generates an image with random points, then assigns a random number\n"
                 " of cluster centers and uses kmeans to move those cluster centers to their\n"
                 " representative location\n"
                 " Usage:\n"
              << argv[0] << "\n\n"
              << "ESC or 'q' or 'Q' to quit\n\n" << std::endl;
}

int main(int argc, const char * argv[]) {
    // 1 提示帮助信息
    help(argv);
    
    // 2 准备基本数据
    const int MAX_CLUSTERS = 5;
    cv::Scalar colorTab[] = {
        cv::Scalar( 0,      0,      255),
        cv::Scalar( 0,      255,    0),
        cv::Scalar( 255,    100,    100),
        cv::Scalar( 255,    0,      255),
        cv::Scalar( 0,      255,    255)
    };
    // 使用1个500*500图像限定样本的取值范围，即每个样本的x和y轴分量
    cv::Mat img(500, 500, CV_8UC3);
    // 生成1个随机种子
    cv::RNG rng(12345);
    
    // 3 循环挂起程序，监听用户键盘输入
    for (; ;) {
        // 4 使用随机函数的方式模拟采样过程，生成样本数据
        // 随机生成样本的聚类分组数
        int clusterCount = rng.uniform(2, MAX_CLUSTERS+1);
        // 随机生成所有组的总样本数量
        int sampleCount = rng.uniform(1, 1001);
        clusterCount = MIN(clusterCount, sampleCount);
        // 样本集
        cv::Mat points(sampleCount, 1, CV_32FC2);
        // 模拟样本数据
        for (int k = 0; k < clusterCount; k++) {
            // 使用均一概率分布随机生成当前分组的中心
            cv::Point center;
            center.x = rng.uniform(0, img.cols);
            center.y = rng.uniform(0, img.rows);
            // 获取当前分组的样本子集对应的子矩阵
            cv::Mat pointChunk = points.rowRange(
                    k * sampleCount / clusterCount,
                    k == clusterCount - 1 ? sampleCount : (k + 1) * sampleCount / clusterCount);
            // 使用正态分布随机生成当前分组的样本数据
            rng.fill(pointChunk,
                     cv::RNG::NORMAL,
                     cv::Scalar(center.x, center.y),
                     cv::Scalar(img.cols*0.05, img.rows*0.05));
        }
        // 打乱样本数据，模拟真实的采样结果
        cv::randShuffle(points, 1, &rng);
        
        // 5 使用k均值算法处理样本集
        // 聚类中心
        cv::Mat centers(clusterCount, 1, points.type());
        // 样本处理后的分类标签
        cv::Mat labels;
        cv::kmeans(points,
                   clusterCount,
                   labels,
                   cv::TermCriteria(
                       cv::TermCriteria::EPS | cv::TermCriteria::COUNT,
                       10,
                       1.0),
                   3,
                   cv::KMEANS_PP_CENTERS,
                   centers);
        
        // 6 显示聚类结果
        img = cv::Scalar::all(0);
        // 绘制所有的样本，并使用不同的颜色区分类簇
        for (int i = 0; i < sampleCount; i++) {
            int clusterIdx = labels.at<int>(i);
            cv::Point ipt = points.at<cv::Point2f>(i);
            cv::circle(img, ipt, 2, colorTab[clusterIdx], cv::FILLED, cv::LINE_AA);
        }
        cv::imshow("Example 16-01", img);

        // 7 挂起程序
        char key = (char)cv::waitKey();
        if (key == 27 || key == 'q' || key == 'Q') {
            break;
        }
    }
    return 0;
}
