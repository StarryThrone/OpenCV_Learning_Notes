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

//#include <opencv2/opencv.hpp>

// Read Me First
//
// This example has not been tested by author due to a lack of time. But these code ara still valuble
// for learning OpenCV with its rich annotaions, or you can vist origianl version on follow site:
// https://github.com/oreillymedia/Learning-OpenCV-3_examples/blob/master/example_20-01.cpp
//
// I am working on make this example more reliable and available on various platforms - so stay tuned!

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
    cv::Mat img(500, 500, CV_8UC3);
    cv::RNG rng(12345);
    
    // 3 循环挂起程序，监听用户键盘输入
    for (; ;) {
        // 4 准备均值算法需要的一些样本限定变量
        // 随机生成样本的聚类分组数
        int clusterCount = rng.uniform(2, MAX_CLUSTERS+1);
        // 随机生成所有组的总样本数量
        int sampleCount = rng.uniform(1, 1001);
        clusterCount = MIN(clusterCount, sampleCount);
        
        // 5 准备存储数据的矩阵
        // 样本
        cv::Mat points(sampleCount, 1, CV_32FC2);
        // 聚类中心
        cv::Mat centers(clusterCount, 1, points.type());
        // 样本处理后的分类标签
        cv::Mat labels;

        // Generate random sample from multiguassian distribution
        for (int k = 0; k < clusterCount; k++) {
            cv::Point center;
            center.x = rng.uniform(0, img.cols);
            center.y = rng.uniform(0, img.rows);
            // 获取到指定行的子矩阵引用
            cv::Mat pointChunk = points.rowRange(
                    k * sampleCount / clusterCount,
                    k == clusterCount - 1 ? sampleCount : (k + 1) * sampleCount / clusterCount);
            rng.fill(pointChunk,
                     cv::RNG::NORMAL,
                     cv::Scalar(center.x, center.y),
                     cv::Scalar(img.cols*0.05, img.rows*0.05));
        }
        cv::randShuffle(points, 1, &rng);
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
        
        img = cv::Scalar::all(0);
        for (int i = 0; i < sampleCount; i++) {
            int clusterIdx = labels.at<int>(i);
            cv::Point ipt = points.at<cv::Point2f>(i);
            cv::circle(img, ipt, 2, colorTab[clusterIdx], cv::FILLED, cv::LINE_AA);
        }
        cv::imshow("Example 20-01", img);
        char key = (char)cv::waitKey();
        // 'ESC'
        if (key == 27 || key == 'q' || key == 'Q') {
            break;
        }
    }
    return 0;
}
