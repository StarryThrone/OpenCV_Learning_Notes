//
//  main.cpp
//  DrawBoxes
//
//  Created by chenjie on 2020/6/13.
//  Copyright © 2020 chenjie. All rights reserved.
//

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

// 定义鼠标事件的回调函数
void my_mouse_callback (int event, int x, int y, int flags, void * param);

cv::Rect box;
bool drawing_box = false;

// 矩形绘制函数
void draw_box(cv::Mat& img, cv::Rect box) {
    cv::rectangle(img, box.tl(), box.br(), cv::Scalar(0x00,0x00,0xff));
}

void help(const char * argv[]) {
    std::cout << "Example. Program for using a mouse to draw boxes on the screen"
              << "\n Call: \n" << argv[0]
              << "\n\nshows how to use a mouse to draw regions in an image. Esc to quit\n"
              << std::endl;
}

int main(int argc, const char * argv[]) {
    help(argv);
    
    box = cv::Rect(-1, -1, 0, 0);
    // 准备两个图像实例，image用于保存历史绘制的多个矩形，temp用于记录当前鼠标的移动绘制临时矩形
    cv::Mat image = cv::Mat(200, 200, CV_8UC3);
    cv::Mat temp;
    image.copyTo(temp);
    // 使用全为0的cv::Scalar初始化矩阵的每个元素
    image = cv::Scalar::all(0);
    // 创建窗口
    cv::namedWindow("Box Example");

    // 注册鼠标事件的监听汗函数，透传参数设置为原图，这样在鼠标事件的回调函数中就可以完成图像绘制操作
    cv::setMouseCallback("Box Example", my_mouse_callback, (void *)&image);
    
    // 程序主循环
    for(;;) {
        // 将历史图片数据拷贝至临时图片中
        image.copyTo(temp);
        if (drawing_box) {
            // 如果用户正在绘制图片，即鼠标键位处于按下位置，则将当前记录的矩形绘制在临时图片中
            draw_box(temp, box);
        }
        // 显示临时图片
        cv::imshow("Box Example", temp);
        // 挂起程序，间隔为15毫米，即刷新图片的帧率约66FPS
        if (cv::waitKey(15) == 27) {
            // 如果用户输入ESC键，则退出程序
            break;
        }
    }

    return 0;
}


void my_mouse_callback(int event, int x, int y, int flags, void * param) {
    // 声明工作图像的引用变量
    switch (event) {
        // 如果按下鼠标左键，则记录用户开始绘图
        case cv::EVENT_LBUTTONDOWN: {
            drawing_box = true;
            box = cv::Rect(x, y, 0, 0);
        }
            break;
        // 鼠标移动时，如果用户正在绘制矩形，则根据用户当前的位置和开始绘图时的位置计算矩形的宽高
        case cv::EVENT_MOUSEMOVE: {
            if (drawing_box) {
                box.width = x-box.x;
                box.height = y-box.y;
            }
        }
            break;
        // 当用户松开鼠标左键，则记录用户停止绘图，并将当前记录的矩形绘制到工作图像中
        case cv::EVENT_LBUTTONUP: {
            drawing_box = false;
            // 如果矩形宽或者高小于0，则表示用户结束绘图时鼠标在开始绘图时的左侧及下侧，需要重新计算矩形
            if (box.width < 0) {
                box.x += box.width;
                box.width *= -1;
            }
            if (box.height < 0) {
                box.y += box.height;
                box.height *= -1;
            }
            cv::Mat & image = *(cv::Mat *)param;
            draw_box(image, box);
        }
            break;
    }
}
