//
//  main.cpp
//  LineFitting
//
//  Created by 陈杰 on 2021/7/15.
//

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>



void help(const char * argv[]) {
    std::cout << "\nExample, two dimensional line fitting"
              << "\nCall"
              << "\n" << argv[0] << "\n"
              << "\n 'q', 'Q' or ESC to quit"
              << "\n" << std::endl;
}



int main(int argc, const char * argv[]) {
    help(argv);
    
    // 用于存放绘制结果的图像，也就是画布
    cv::Mat img(500, 500, CV_8UC3);
    // 创建随机种子
    cv::RNG rng(-1);
    while (1) {
        // 定义样本数量的随机值，最小值为3
        int count = rng.uniform(0, 100) + 3;
        // 定义离群样本的数量
        int outliers = count / 5;
        // 存储所有的样本
        std::vector<cv::Point> points(count);
        
        // x方向偏移的最大取值
        float a = (float)rng.uniform(0., 200.);
        // y方向偏移的最大取值
        float b = (float)rng.uniform(0., 40.);
        // 定义0-180度的随机角度，未预定义线的倾角
        float angle = (float)rng.uniform(0., CV_PI);
        float cos_a = cos(angle);
        float sin_a = sin(angle);
        // 在预先定义好的线两侧生成一系列的噪声样本
        for (int i = 0; i < count - outliers; i++) {
            float x = (float)rng.uniform(-1., 1.) * a;
            float y = (float)rng.uniform(-1., 1.) * b;
            points[i].x = cvRound(x * cos_a - y * sin_a + img.cols / 2);
            points[i].y = cvRound(x * sin_a + y * cos_a + img.rows / 2);
        }
        
        // 填充离群样本
        for (int i = count - outliers - 1; i < count; i++) {
            points[i].x = rng.uniform(0, img.cols);
            points[i].y = rng.uniform(0, img.rows);
        }

        // 使用准备的样本拟合直线
        cv::Vec4f line;
        cv::fitLine(points, line, cv::DIST_L1, 1, 0.001, 0.001);
        
        // 绘制所有的点
        img = cv::Scalar::all(0);
        for (int i = 0; i < count; i++) {
            cv::Scalar pointColor = (i < count - outliers ? cv::Scalar(0, 0, 255) : cv::Scalar(0, 255, 255));
            cv::circle(img, points[i], 2, pointColor, cv::FILLED, cv::LINE_AA, 0);
        }
        
        // 计算拟合线上的两个端点确保它们穿越了整个图像
        float d = sqrt((double)line[0] * line[0] + (double)line[1] * line[1]);
        line[0] /= d;
        line[1] /= d;
        cv::Point pt1, pt2;
        float t = (float)(img.cols + img.rows);
        pt1.x = cvRound(line[2] - line[0] * t);
        pt1.y = cvRound(line[3] - line[1] * t);
        pt2.x = cvRound(line[2] + line[0] * t);
        pt2.y = cvRound(line[3] + line[1] * t);
        // 绘制拟合线条
        cv::line(img, pt1, pt2, cv::Scalar(0, 255, 0), 3, cv::LINE_AA, 0);
        // 显示绘制结果
        cv::imshow("Fit Line", img);
        
        char key = (char)cv::waitKey(0);
        // 阻塞当前线程等待键盘输入事件
        if (key == 27 || key == 'q' || key == 'Q') {
            // 用户退出时结束程序
            break;
        }
    }
    return 0;
}
