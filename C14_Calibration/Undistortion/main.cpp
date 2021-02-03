//
//  main.cpp
//  Undistortion
//
//  Created by chenjie on 2021/2/1.
//

#include <iostream>
#include <dirent.h>
#include <sys/stat.h>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/highgui.hpp>


//MARK: dyld: lazy symbol binding failed: Symbol not found: ___emutls_get_address
//  Referenced from: /usr/local/opt/gcc/lib/gcc/10/libgomp.1.dylib
//  Expected in: /usr/lib/libSystem.B.dylib
//  Mac OS 11.1 缺失系统库文件，暂无法运行，后续考虑使用Framework



/// 读出文件夹内所有文件名，以.开头的除外
/// @param filenames 文件名向量
/// @param directory 文件夹路径
int readFilenames(std::vector<std::string>& filenames, const std::string& directory) {
    DIR *dir;
    class dirent *ent;
    class stat st;
    
    // 打开文件夹
    dir = opendir(directory.c_str());
    // 依次读取每个条目
    while ((ent = readdir(dir)) != NULL) {
        const std::string file_name = ent->d_name;
        // 排除隐藏文件
        if (file_name[0] == '.') {
            continue;
        }
        
        const std::string full_file_name = directory + "/" + file_name;
        // 读取文件信息失败直接跳过
        if (stat(full_file_name.c_str(), &st) == -1) {
            continue;
        }

        // 跳过文件夹
        const bool is_directory = (st.st_mode & S_IFDIR) != 0;
        if (is_directory) {
            continue;
        }

        // 保存文件名
        filenames.push_back(file_name);
    }
    
    // 关闭文件夹
    closedir(dir);
    // 对文件名排序
    std::sort(filenames.begin(), filenames.end());
    // 计算成功找到的文件数量
    int fileSize = int(filenames.size());
    return fileSize;
}

void help(const char * argv[]) {
    std::cout << "\n\n"
              << "Example:\Enter a chessboard’s width and height,\n"
              << "reading in a directory of chessboard images,\n"
              << "and calibrating the camera\n\n"
              << "Call:\n" << argv[0] << " <1board_width> <2board_height> <3number_of_boards>"
              << " <4ms_delay_framee_capture> <5image_scaling_factor> <6path_to_calibration_images>\n\n"
              << "\nExample:\n"
              << "./example 9 6 14 100 1.0 ../stereoData/\nor:\n"
              << "./example 12 12 28 100 0.5 ../calibration/\n\n"
              << " * First it reads in checker boards and calibrates itself\n"
              << " * Then it saves and reloads the calibration matricies\n"
              << " * Then it creates an undistortion map and finaly\n"
              << " * It displays an undistorted image\n"
              << std::endl;
}



int main(int argc, const char * argv[]) {
    if (argc != 7) {
        std::cout << "\nERROR: Wrong number (" << argc - 1
                  << ") of arguments, should be (6) input parameters\n";
        help(argv);
        return -1;
    }
    
    // 1. 读取关键参数
    int board_w = atoi(argv[1]);
    int board_h = atoi(argv[2]);
    // 需要读取到棋盘图像数量
    int n_boards = atoi(argv[3]);
    // 延时，单位为毫秒
    int delay = atof(argv[4]);
    // 图片缩放系数
    float image_sf = atof(argv[5]);
    // 棋盘的角点数量
    int board_n = board_w * board_h;
    // 棋盘大小
    cv::Size board_sz = cv::Size(board_w, board_h);
    
    // 图片保存的文件夹路径
    std::string folder = argv[6];
    std::cout << "Reading in directory " << folder << std::endl;
    std::vector<std::string> filenames;
    // 2. 读取所有棋盘图像的文件名
    int num_files = readFilenames(filenames, folder);
    std::cout << "   ... Done. Number of files = " << num_files << "\n" << std::endl;

    // 初始化点存储容器
    // 角点坐标，单位是像素，以图像坐标系为参考
    std::vector<std::vector<cv::Point2f>> image_points;
    // 角点索引坐标，表示第几个角点，以模型坐标系为参考
    std::vector<std::vector<cv::Point3f>> object_points;
    
    // 3. 处理用于校准的棋盘图像
    // 尽可能收集用户指定数量的原始图像，收集它们所有的角点
    cv::Size image_size;
    int board_count = 0;
    for (size_t i = 0; (i < filenames.size()) && (board_count < n_boards); ++i) {
        // 3.1 读取图像
        cv::Mat image0 = cv::imread(folder + filenames[i]);
        board_count += 1;
        if (!image0.data) {
            std::cerr << folder + filenames[i] << ", file #" << i << ", is not an image" << std::endl;
            continue;
        }
        
        // 3.2 根据指定的缩放系数，缩放输入图像
        image_size = image0.size();
        cv::Mat image;
        cv::resize(image0, image, cv::Size(), image_sf, image_sf, cv::INTER_LINEAR);

        // 3.3 寻找当前图像中的所有角点
        std::vector<cv::Point2f> corners;
        bool found = cv::findChessboardCorners(image, board_sz, corners);

        // 3.3 绘制找到的角点
        drawChessboardCorners(image, board_sz, corners, found);

        // 3.4 如果对于某个图像，找到了全部的角点，则将其纳入统计数据中
        if (found) {
            // 找出非白色（255， 255， 255，255）区域
            image ^= cv::Scalar::all(255);
            
//            cv::Mat mcorners(corners);
//            // do not copy the data
//            mcorners *= (1.0 / image_sf);

            // 保存角点像素坐标
            image_points.push_back(corners);
            // 计算并保存角点索引坐标
            object_points.push_back(std::vector<cv::Point3f>());
            std::vector<cv::Point3f> &opts = object_points.back();
            opts.resize(board_n);
            for (int j = 0; j < board_n; j++) {
                opts[j] = cv::Point3f(static_cast<float>(j / board_w),
                                      static_cast<float>(j % board_w), 0.0f);
            }
            
            std::cout << "Collected " << static_cast<int>(image_points.size())
                      << "total boards. This one from chessboard image #"
                      << i << ", " << folder + filenames[i] << std::endl;
        }
        
        // 3.5 展示处理后的图像
        cv::imshow("Calibration", image);
        
        // 3.6 适当挂起程序
        if ((cv::waitKey(delay) & 255) == 27) {
            return -1;
        }
    }
    
    // 收集完所有数据后销毁窗口
    cv::destroyWindow("Calibration");
    std::cout << "\n\n*** CALIBRATING THE CAMERA...\n" << std::endl;

    // 4. 校准图像
    cv::Mat intrinsic_matrix;
    cv::Mat distortion_coeffs;
    // 计算内参矩阵和扭曲系数
    double err = cv::calibrateCamera(object_points, image_points, image_size,
                                     intrinsic_matrix, distortion_coeffs, cv::noArray(), cv::noArray(),
                                     cv::CALIB_ZERO_TANGENT_DIST | cv::CALIB_FIX_PRINCIPAL_POINT);

    // 5. 保存内参矩阵和扭曲系数
    std::cout << " *** DONE!\n\nReprojection error is " << err
              << "\nStoring Intrinsics.xml and Distortions.xml files\n\n";
    cv::FileStorage fs("intrinsics.xml", cv::FileStorage::WRITE);
    fs << "image_width" << image_size.width << "image_height" << image_size.height
       << "camera_matrix" << intrinsic_matrix << "distortion_coefficients"
       << distortion_coeffs;
    fs.release();

    // 6. 读取内参矩阵和扭曲系数
    fs.open("intrinsics.xml", cv::FileStorage::READ);
    std::cout << "\nimage width: " << static_cast<int>(fs["image_width"]);
    std::cout << "\nimage height: " << static_cast<int>(fs["image_height"]);
    cv::Mat intrinsic_matrix_loaded;
    cv::Mat distortion_coeffs_loaded;
    fs["camera_matrix"] >> intrinsic_matrix_loaded;
    fs["distortion_coefficients"] >> distortion_coeffs_loaded;
    std::cout << "\nintrinsic matrix:" << intrinsic_matrix_loaded;
    std::cout << "\ndistortion coefficients: " << distortion_coeffs_loaded << "\n" << std::endl;

    // 7. 计算矫正映射
    cv::Mat map1;
    cv::Mat map2;
    cv::initUndistortRectifyMap(intrinsic_matrix_loaded, distortion_coeffs_loaded,
                                cv::Mat(), intrinsic_matrix_loaded,
                                image_size,
                                CV_16SC2, map1, map2);
        
    // 8. 计算并展示矫正后的图像
    std::cout << "*****************\nPRESS A KEY TO SEE THE NEXT IMAGE, ESQ TO QUIT\n"
              << "****************\n" << std::endl;
    board_count = 0;
    for (size_t i = 0; (i < filenames.size()) && (board_count < n_boards); ++i) {
        // 读取原始图像
        cv::Mat image0 = cv::imread(folder + filenames[i]);
        ++board_count;
        if (!image0.data) {
            std::cerr << folder + filenames[i] << ", file #" << i << ", is not an image" << std::endl;
            continue;
        }
        
        // 计算矫正后的图像
        cv::Mat image;
        cv::remap(image0, image, map1, map2, cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar());
        
        // 展示原图和矫正后的图像
        cv::imshow("Original", image0);
        cv::imshow("Undistorted", image);
        
        if ((cv::waitKey(0) & 255) == 27) {
            break;
        }
    }
    
    return 0;
}
