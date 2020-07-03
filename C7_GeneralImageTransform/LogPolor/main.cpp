//
//  main.cpp
//  LogPolor
//
//  Created by chenjie on 2020/7/3.
//  Copyright © 2020 chenjie. All rights reserved.
//

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

int main(int argc, const char * argv[]) {
    if(argc != 3) {
        std::cout << "LogPolar\nUsage: " << argv[0] << " <imagename> <M value>\n"
                  << "<M value>~30 is usually good enough\n" << std::endl;
        return -1;
    }
    
    // 加载图像
    cv::Mat src = cv::imread(argv[1], 1);
    if (src.empty()) {
        std::cout << "Can not load " << argv[1] << std::endl;
        return -1;
    }

    // 读取缩放系数，该系数越大，则得到的对数极坐标表示水平拉伸更大
    double M = atof(argv[2]);
    cv::Mat dst(src.size(), src.type());
    cv::Mat src2(src.size(), src.type());

    // 计算对数极坐标变换
    cv::logPolar(src, dst, cv::Point2f(src.cols * 0.5f, src.rows * 0.5f),
                 M, cv::INTER_LINEAR | cv::WARP_FILL_OUTLIERS);
    // 计算对数极坐标逆变换
    cv::logPolar(dst, src2, cv::Point2f(src.cols*0.5f, src.rows*0.5f),
                 M, cv::INTER_LINEAR | cv::WARP_INVERSE_MAP);
    
    // 显示图像处理的结果
    cv::imshow("source", src);
    cv::imshow("log-polar", dst);
    cv::imshow("inverse log-polar", src2);

    // 挂起程序直至用户输入任意键盘
    cv::waitKey();
    
    return 0;
}
