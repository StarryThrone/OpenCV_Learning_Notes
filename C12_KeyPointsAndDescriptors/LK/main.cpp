//
//  main.cpp
//  LK
//
//  Created by chenjie on 2020/10/5.
//

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>



static const int MAX_CORNERS = 500;



void help(const char * argv[]) {
    std::cout << "\nExample: Pyramid L-K optical flow example.\n" << std::endl;
    std::cout << "Call: " <<argv[0] << "[image1] [image2]\n" << std::endl;
    std::cout << "\nExample:\n" << argv[0] << " ../example_16-01-imgA.png ../example_16-01-imgB.png\n" << std::endl;
    std::cout << "Demonstrates Pyramid Lucas-Kanade optical flow.\n" << std::endl;
}



int main(int argc, const char * argv[]) {
    if (argc != 3) {
        help(argv);
        exit(-1);
    }
    

    // 1 The first thing we need to do is get the features we want to track.
    cv::Mat imgA = cv::imread(argv[1], cv::IMREAD_GRAYSCALE);
    std::vector<cv::Point2f> cornersA;
    cv::goodFeaturesToTrack(
        imgA,                         // Image to track
        cornersA,                     // Vector of detected corners (output)
        MAX_CORNERS,                  // Keep up to this many corners
        0.01,                         // Quality level (percent of maximum)
        5,                            // Min distance between corners
        cv::noArray(),                // Mask
        3,                            // Block size
        false,                        // true: Harris, false: Shi-Tomasi
        0.04                          // method specific parameter
    );

    int win_size = 10;
    cv::cornerSubPix(
        imgA,                           // Input image
        cornersA,                       // Vector of corners (input and output)
        cv::Size(win_size, win_size),   // Half side length of search window
        cv::Size(-1, -1),               // Half side length of dead zone (-1=none)
        cv::TermCriteria(
            cv::TermCriteria::MAX_ITER | cv::TermCriteria::EPS,
            20,                         // Maximum number of iterations
            0.03                        // Minimum change per iteration
        )
    );

    // 2 Track features
    cv::Mat imgB = cv::imread(argv[2], cv::IMREAD_GRAYSCALE);
    std::vector<cv::Point2f> cornersB;
    std::vector<uchar> features_found;
    cv::calcOpticalFlowPyrLK(
        imgA,                         // Previous image
        imgB,                         // Next image
        cornersA,                     // Previous set of corners (from imgA)
        cornersB,                     // Next set of corners (from imgB)
        features_found,               // Output vector, each is 1 for tracked
        cv::noArray(),                // Output vector, lists errors (optional)
        cv::Size(win_size * 2 + 1, win_size * 2 + 1),  // Search window size
        5,                            // Maximum pyramid level to construct
        cv::TermCriteria(
            cv::TermCriteria::MAX_ITER | cv::TermCriteria::EPS,
            20,                         // Maximum number of iterations
            0.3                         // Minimum change per iteration
        )
    );
    
    // 3 Visualize optical flow
    // Now make some image of what we are looking at:
    // Note that if you want to track cornersB further, i.e.
    // pass them as input to the next calcOpticalFlowPyrLK,
    // you would need to "compress" the vector, i.e., exclude points for which
    // features_found[i] == false.
    cv::Mat imgC = cv::imread(argv[2], cv::IMREAD_UNCHANGED);
    for (int i = 0; i < static_cast<int>(cornersA.size()); ++i) {
        if (!features_found[i]) {
            continue;
        }
        line(
            imgC,                        // Draw onto this image
            cornersA[i],                 // Starting here
            cornersB[i],                 // Ending here
            cv::Scalar(0, 255, 0),       // This color
            1,                           // This many pixels wide
            cv::LINE_AA                  // Draw line in this style
        );
    }

    // 4 Show results
    cv::imshow("ImageA", imgA);
    cv::imshow("ImageB", imgB);
    cv::imshow("LK Optical Flow Example", imgC);
    
    // 5 Suspend Application
    cv::waitKey(0);
    
    return 0;
}
