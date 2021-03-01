//
//  main.cpp
//  BirdEyeView
//
//  Created by chenjie on 2021/2/15.
//

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

void help() {
    std::cout << "\nExample, using homography to get a bird's eye view."
              << "\nThis file relies on you having created an intrinsic file via C15_Projection"
              << "\nbut here, that file is already stored in ../birdseye/intrinsics.xml"
              << "\nCall:"
              << "\n./example <chessboard_width> <chessboard_height> <path/camera_calib_filename> <path/chessboard_image>"
              << "\n\nExample:"
              << "\n./example 12 12 ../birdseye/intrinsics.xml ../birdseye/IMG_0215L.jpg\n"
              << "\nPress 'd' for lower birdseye view, and 'u' for higher (it adjusts the apparent 'Z' height), Esc to exit\n"
              << std::endl;
}

int main(int argc, const char * argv[]) {
    // 1 校验参数合法性
    if (argc != 5) {
        std::cout << "\nERROR: too few parameters\n";
        help();
        return -1;
    }
    
    // 2 读取相机的内参矩阵和扭曲系数
    cv::FileStorage fs(argv[3], cv::FileStorage::READ);
    cv::Mat intrinsic, distortion;
    fs["camera_matrix"] >> intrinsic;
    fs["distortion_coefficients"] >> distortion;
    if (!fs.isOpened() || intrinsic.empty() || distortion.empty()) {
        std::cout << "Error: Couldn't load intrinsic parameters from " << argv[3]
                  << std::endl;
        return -1;
    }
    fs.release();
    
    // 3 读取棋盘图
    cv::Mat image0 = cv::imread(argv[4], 1);
    if (image0.empty()) {
        std::cout << "Error: Couldn't load image " << argv[4] << std::endl;
        return -1;
    }

    // 4 获取矫正后的灰度图像
    cv::Mat gray_image, image;
    //TODO: 疑似两个关键矩阵内部数据错误，因此矫正后的图像仍然扭曲
    cv::undistort(image0, image, intrinsic, distortion, intrinsic);
    cv::cvtColor(image, gray_image, cv::COLOR_BGRA2GRAY);


    // 5 寻找棋盘图像内角点
    // 获取输入参数
    int board_w = atoi(argv[1]);
    int board_h = atoi(argv[2]);
    // 获取棋盘内角点数量
    cv::Size board_sz(board_w, board_h);
    std::vector<cv::Point2f> corners;
    bool found = cv::findChessboardCorners(image,
                                           board_sz,
                                           corners,
                                           cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_FILTER_QUADS);
    if (!found) {
        // 如果未找到全部的角点
        int board_n = board_w * board_h;
        std::cout << "Couldn't acquire checkerboard on " << argv[4] << ", only found "
                  << corners.size() << " of " << board_n << " corners\n";
        return -1;
    }
    
    // 6 计算亚像素级别精度的角点坐标
    cv::cornerSubPix(gray_image,
                     corners,
                     cv::Size(11, 11),
                     cv::Size(-1, -1),
                     cv::TermCriteria(cv::TermCriteria::EPS | cv::TermCriteria::COUNT, 30, 0.1));

    // 7 准备4组角点模型坐标，及在图像中对应点点图像坐标
    // 前者以模型坐标系为参考，单位为行和列，取值范围为(0,0), (board_w-1,0), (0,board_h-1), (board_w-1,board_h-1)
    // 后者以图像坐标系为参考，单位为像素
    cv::Point2f objPts[4], imgPts[4];
    objPts[0].x = 0;
    objPts[0].y = 0;
    objPts[1].x = board_w - 1;
    objPts[1].y = 0;
    objPts[2].x = 0;
    objPts[2].y = board_h - 1;
    objPts[3].x = board_w - 1;
    objPts[3].y = board_h - 1;
    imgPts[0] = corners[0];
    imgPts[1] = corners[board_w - 1];
    imgPts[2] = corners[(board_h - 1) * board_w];
    imgPts[3] = corners[(board_h - 1) * board_w + board_w - 1];

    // 8 使用蓝、绿、红、黄四种颜色在图像中绘制出选择的参考角点
    cv::circle(image, imgPts[0], 9, cv::Scalar(255, 0, 0), 3);
    cv::circle(image, imgPts[1], 9, cv::Scalar(0, 255, 0), 3);
    cv::circle(image, imgPts[2], 9, cv::Scalar(0, 0, 255), 3);
    cv::circle(image, imgPts[3], 9, cv::Scalar(0, 255, 255), 3);
    // 绘制所有的角点
    cv::drawChessboardCorners(image, board_sz, corners, found);
    cv::imshow("Checkers", image);

    // 9 计算单应矩阵
    cv::Mat H = cv::getPerspectiveTransform(objPts, imgPts);

    // LET THE USER ADJUST THE Z HEIGHT OF THE VIEW
    // 10 计算鸟瞰图，并支持用户调整视角高度
    std::cout << "\nPress 'd' for lower birdseye view, and 'u' for higher (it adjusts the apparent 'Z' height), Esc to exit" << std::endl;
    double Z = 15;
    cv::Mat birds_image;
    for (;;) {
        // 按ESC键退出
        H.at<double>(2, 2) = Z;
        // 使用单应性计算鸟瞰图
        cv::warpPerspective(image,
                            birds_image,
                            H,
                            image.size(),
                            cv::WARP_INVERSE_MAP | cv::INTER_LINEAR,
                            cv::BORDER_CONSTANT, cv::Scalar::all(0));
        cv::imshow("Birds_Eye", birds_image);
        int key = cv::waitKey() & 255;
        if (key == 'u') {
            Z += 0.5;
        }
        if (key == 'd') {
            Z -= 0.5;
        }
        if (key == 27) {
            break;
        }
    }

    // 11 计算旋转和平移向量
    std::vector<cv::Point2f> image_points;
    std::vector<cv::Point3f> object_points;
    for (int i = 0; i < 4; ++i) {
        image_points.push_back(imgPts[i]);
        object_points.push_back(cv::Point3f(objPts[i].x, objPts[i].y, 0));
    }
    cv::Mat rvec, tvec, rmat;
    cv::solvePnP(object_points,
                 image_points,
                 intrinsic,
                 cv::Mat(),
                 rvec,
                 tvec);
    cv::Rodrigues(rvec, rmat);

    // 12 打印结果
    std::cout << "rotation matrix: " << rmat << std::endl;
    std::cout << "translation vector: " << tvec << std::endl;
    std::cout << "homography matrix: " << H << std::endl;
    std::cout << "inverted homography matrix: " << H.inv() << std::endl;

    return 1;
}
