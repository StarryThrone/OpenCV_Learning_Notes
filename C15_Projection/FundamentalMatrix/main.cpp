//
//  main.cpp
//  FundamentalMatrix
//
//  Created by 陈杰 on 2021/6/13.
//

#include <iostream>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/highgui.hpp>

void help(const char * argv[]) {
    std::cout << "\nExample FundamentalMatrix, Computing the fundamental matrix using RANSAC relating 2 images. Show the camera a checkerboard "
              << "\nCall"
              << "\n./FundamentalMatrix <1:board_w> <2:board_h> <3:# of boards> <4:delay capture this many ms between frames> <5:scale the images 0-1>"
              << "\n\nExample call:"
              << "\n./example_19-2 9 6 20 500 0.5"
              << "\n\n -- use the checkerboard9x6.png provided"
              << "\n"
              << std::endl;
}

int main(int argc, const char * argv[]) {
    if (argc != 6) {
      std::cout << "\nERROR: Wrong number of input parameters, need 5, got " << argc - 1 << "\n";
      help(argv);
      return -1;
    }

    cv::VideoCapture capture(0);
    if (!capture.isOpened()) {
        std::cout << "\nCouldn't open the camera\n";
        help(argv);
        return -1;
    }

    int board_w = atoi(argv[1]);
    int board_h = atoi(argv[2]);
    int n_boards = atoi(argv[3]);
    // 保留参数
//    float delay = atof(argv[4]);
    float image_sf = atof(argv[5]);
    int board_n = board_w * board_h;
    cv::Size board_sz = cv::Size(board_w, board_h);

    // 初始化坐标矩阵，使用向量结构保存数据
    std::vector<std::vector<cv::Point2f>> image_points;
    std::vector<std::vector<cv::Point3f>> object_points;
    
    // 从摄像头不断获取标定板的数据，直至成功获取到n_boards定义的数量
    double last_captured_timestamp = 0;
    cv::Size image_size;
    while (image_points.size() < (size_t)n_boards) {
        cv::Mat image0, image;
        capture >> image0;
        image_size = image0.size();
        // 对图像进行重采样
        cv::resize(image0, image, cv::Size(), image_sf, image_sf, cv::INTER_LINEAR);

        // 寻找角点
        std::vector<cv::Point2f> corners;
        bool found = cv::findChessboardCorners(image, board_sz, corners);
        // 绘制棋盘角点
        cv::drawChessboardCorners(image, board_sz, corners, found);
        // 如果获得足够的角点则记录数据
        double timestamp = (double)clock() / CLOCKS_PER_SEC;
        if (found && timestamp - last_captured_timestamp > 1) {
            last_captured_timestamp = timestamp;
            // 反色处理
            image ^= cv::Scalar::all(255);

            // 保留逻辑
            // do not copy the data
//            cv::Mat mcorners(corners);
            // scale corner coordinates
//            mcorners *= (1. / image_sf);
            image_points.push_back(corners);
            // 计算并保存焦点在模型坐标系下的单位坐标
            object_points.push_back(std::vector<cv::Point3f>());
            std::vector<cv::Point3f> &opts = object_points.back();
            opts.resize(board_n);
            for (int j = 0; j < board_n; j++) {
                opts[j] = cv::Point3f((float)(j / board_w), (float)(j % board_w), 0.f);
            }
            std::cout << "Collected our " << (int)image_points.size() << " of " << n_boards
                      << " needed chessboard images\n" << std::endl;
        }
        // 显示经过缩放绘制出的角点图
        cv::imshow("Calibration", image);
        if ((cv::waitKey(30) & 255) == 27) {
            return -1;
        }
    }
    // 结束数据采集
    cv::destroyWindow("Calibration");
    std::cout << "\n\n*** CALIBRATING THE CAMERA...\n" << std::endl;
    
    // 开始标定相机
    cv::Mat intrinsic_matrix, distortion_coeffs;
    double err = cv::calibrateCamera(object_points,     // 标定棋盘的角点在模型坐标系下的坐标，使用向量的向量来表示
                                     image_points,      // 标定棋盘的角点投影在图像上的坐标，和object_points中的点对应，格式相同
                                     image_size,        // 标定图像的大小
                                     intrinsic_matrix,  // 计算出的相机内参矩阵
                                     distortion_coeffs, // 计算出的相机畸变系数
                                     cv::noArray(),     // 忽略旋转向量
                                     cv::noArray(),     // 忽略平移向量
                                     cv::CALIB_ZERO_TANGENT_DIST | cv::CALIB_FIX_PRINCIPAL_POINT);
    
    // 保存相机的内在参数和扭曲系数
    std::cout << " *** DONE!\n\nReprojection error is " << err
              << "\nStoring Intrinsics.xml and Distortions.xml files\n\n";
    cv::FileStorage fs("intrinsics.xml", cv::FileStorage::WRITE);
    fs << "image_width" << image_size.width << "image_height" << image_size.height
       << "camera_matrix" << intrinsic_matrix << "distortion_coefficients"
       << distortion_coeffs;
    fs.release();

    // 加载相机的内在参数和扭曲系数
    fs.open("intrinsics.xml", cv::FileStorage::READ);
    std::cout << "\nimage width: " << (int)fs["image_width"];
    std::cout << "\nimage height: " << (int)fs["image_height"];
    cv::Mat intrinsic_matrix_loaded, distortion_coeffs_loaded;
    fs["camera_matrix"] >> intrinsic_matrix_loaded;
    fs["distortion_coefficients"] >> distortion_coeffs_loaded;
    std::cout << "\nintrinsic matrix:" << intrinsic_matrix_loaded;
    std::cout << "\ndistortion coefficients: " << distortion_coeffs_loaded << std::endl;
    
    // 假定第一帧图片和第二帧图片分别来自立体成像系统的两个相机，则计算该系统的基础矩阵F，首先对原始的图像数据进行矫正
    cv::undistortPoints(image_points[0],   // 第一帧图像上角点的位置
                        image_points[0],   // 校正后的第一帧图像上角点的位置
                        intrinsic_matrix,  // 相机的内参矩阵，通过函数cv::calibrateCamera()获取
                        distortion_coeffs, // 相机系统的扭曲系数，同样通过函数cv::calibrateCamera()获取
                        cv::Mat(),         // 这里先解决单个相机的校正问题，暂不考虑它位于立体系统中的情况，传入单位矩阵即可
                        intrinsic_matrix); // 同理先解决单个相机的校正问题，暂不考虑它位于立体系统中的情况，传入本身的内参矩阵

    cv::undistortPoints(image_points[1],   // 第二帧图像上角点的位置
                        image_points[1],   // 校正后的第二帧图像上角点的位置
                        intrinsic_matrix,  // 相机的内参矩阵，通过函数cv::calibrateCamera()获取
                        distortion_coeffs, // 相机系统的扭曲系数，同样通过函数cv::calibrateCamera()获取
                        cv::Mat(),         // 这里先解决单个相机的校正问题，暂不考虑它位于立体系统中的情况，传入单位矩阵即可
                        intrinsic_matrix); // 同理先解决单个相机的校正问题，暂不考虑它位于立体系统中的情况，传入本身的内参矩阵

    // 计算模拟的立体成像系统中的基础矩阵
    cv::Mat F = cv::findFundamentalMat(image_points[0], // 第一帧内经过校正后的角点坐标
                                       image_points[1], // 第二帧内经过校正后的角点坐标
                                       cv::FM_8POINT);  // 使用8点算法
    std::cout << "Fundamental matrix: " << F << std::endl;

    // 计算图像的校正映射矩阵
    cv::Mat map1, map2;
    cv::initUndistortRectifyMap(intrinsic_matrix_loaded,  // 相机的内参矩阵
                                distortion_coeffs_loaded, // 相机的畸变系数
                                cv::Mat(),                // 相机坐标系的校正矩阵即旋转矩阵，这里只解决单个相机问题，不考虑立体成像，传入单位矩阵
                                intrinsic_matrix_loaded,  // 相机坐标系校正后的内参矩阵，这里不考虑相机校正，传入原始内参矩阵即可
                                image_size,               // 待校准图像的大小
                                CV_16SC2,                 // 计算的校正映射矩阵格式
                                map1,                     // 计算得到的校正映射矩阵整数部分的数据
                                map2);                    // 计算得到的校正映射矩阵定点Fixed-Point(表示小数)部分的数据

    // 在此运行摄像机，捕捉图像并显示校正后的图像
    for (;;) {
        cv::Mat image, image0;
        capture >> image0;
        if (image0.empty()) {
            break;
        }
        cv::remap(image0,           // 未校正的图像
                  image,            // 校正后的图像
                  map1,             // 校正映射矩阵整数部分的数据
                  map2,             // 校正映射矩阵定点Fixed-Point(表示小数)部分的数据
                  cv::INTER_LINEAR, cv::BORDER_CONSTANT,
                  cv::Scalar());    // 边界区外颜色设置为黑色
        cv::imshow("Undistorted", image);
        if ((cv::waitKey(30) & 255) == 27) {
            break;
        }
    }
    return 1;
}
