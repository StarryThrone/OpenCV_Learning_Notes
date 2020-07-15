//
//  main.cpp
//  Template
//
//  Created by chenjie on 2020/7/15.
//  Copyright © 2020 chenjie. All rights reserved.
//

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

void help(const char * argv[]) {
    std::cout << "\n"
                << "\nExample: using matchTemplate(). The call is:\n"
                << "\n"
                << argv [0] << " <template> <image_to_be_searched?\n"
                << "\nExample:\n" << argv[0] << "../BlueCup.jpg ../adrian.jpg"
                << "\n"
                << " This routine will search using all methods:\n"
                << " cv::TM_SQDIFF 0\n"
                << " cv::TM_SQDIFF_NORMED 1\n"
                << " cv::TM_CCORR 2\n"
                << " cv::TM_CCORR_NORMED 3\n"
                << " cv::TM_CCOEFF 4\n"
                << " cv::TM_CCOEFF_NORMED 5\n"
                << "\n" << std::endl;
}

int main(int argc, const char * argv[]) {
    if (argc != 3) {
        help(argv);
        return -1;
    }
    
    // 读取匹配模版图像
    cv::Mat templ = cv::imread(argv[1], 1);
    if (templ.empty()) {
        std::cout << "Error on reading template " << argv[1] << std::endl;
        help(argv);
        return -1;
    }

    // 读取待查找图像
    cv::Mat src = cv::imread(argv[2], 1);
    if (src.empty()) {
        std::cout << "Error on reading src image " << argv[2] << std::endl;
        help(argv);
        return -1;
    }

    // 使用6种不同的方法执行模版匹配操作
    cv::Mat ftmp[6];
    for (int i = 0; i < 6; ++i) {
        cv::matchTemplate( src, templ, ftmp[i], i);
        cv::normalize(ftmp[i],ftmp[i],1,0,cv::NORM_MINMAX);
    }
    
    // 显示匹配结果
    cv::imshow("Template", templ);
    cv::imshow("Image", src);
    cv::imshow("SQDIFF", ftmp[0]);
    cv::imshow("SQDIFF_NORMED", ftmp[1]);
    cv::imshow("CCORR", ftmp[2]);
    cv::imshow("CCORR_NORMED", ftmp[3]);
    cv::imshow("CCOEFF", ftmp[4]);
    cv::imshow("CCOEFF_NORMED", ftmp[5]);

    // 挂起程序等待用户输入事件
    cv::waitKey(0);

    return 0;
}
