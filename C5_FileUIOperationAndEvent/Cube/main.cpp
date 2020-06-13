//
//  main.cpp
//  Cube
//
//  Created by chenjie on 2020/6/13.
//  Copyright © 2020 chenjie. All rights reserved.
//

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <GLUT/GLUT.h>

int rotx = 55, roty = 40;

void on_opengl(void* param) {
    cv::ogl::Texture2D *backgroundTex = (cv::ogl::Texture2D *)param;
    glEnable(GL_TEXTURE_2D);
    backgroundTex->bind();
    cv::ogl::render(*backgroundTex);
    glDisable(GL_TEXTURE_2D);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -0.5);
    glRotatef(rotx, 1, 0, 0);
    glRotatef(roty, 0, 1, 0);
    glRotatef(0, 0, 0, 1);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    static const int coords[6][4][3] = {
        { { +1, -1, -1 }, { -1, -1, -1 }, { -1, +1, -1 }, { +1, +1, -1 } },
        { { +1, +1, -1 }, { -1, +1, -1 }, { -1, +1, +1 }, { +1, +1, +1 } },
        { { +1, -1, +1 }, { +1, -1, -1 }, { +1, +1, -1 }, { +1, +1, +1 } },
        { { -1, -1, -1 }, { -1, -1, +1 }, { -1, +1, +1 }, { -1, +1, -1 } },
        { { +1, -1, +1 }, { -1, -1, +1 }, { -1, -1, -1 }, { +1, -1, -1 } },
        { { -1, -1, +1 }, { +1, -1, +1 }, { +1, +1, +1 }, { -1, +1, +1 } }
    };
    for (int i = 0; i < 6; ++i) {
        glColor3ub(i*20, 100 + i*10, i*42);
        glBegin(GL_QUADS);
        for (int j = 0; j < 4; ++j) {
            glVertex3d(0.2 * coords[i][j][0], 0.2 * coords[i][j][1], 0.2 * coords[i][j][2]);
        }
        glEnd();
    }
}

void on_trackbar(int, void *) {
    cv::updateWindow("Example Cube");
}

void help(const char * argv[]) {
    std::cout << "\n//Example. Slightly modified code from the OpenCV documentation that draws a"
              << "\n//cube every frame; this modified version uses the global variables rotx and roty that are"
              << "\n//connected to the sliders in Figure 9-6"
              << "\n// Note: This example needs OpenGL installed on your system. It doesn't build if"
              << "\n//       the OpenGL libraries cannot be found.\n//"
              << "\nCall: " << argv[0] << " <image>\n\n"
              << "\nHere OpenGL is used to render a cube on top of an image.\n"
              << "\nUser can rotate the cube with the sliders\n" << std::endl;
}

int main(int argc, const char * argv[]) {
    if (argc != 2) {
        help (argv);
        return -1;
    }

    cv::Mat img = cv::imread(argv[1]);
    if (img.empty()) {
        std::cout << "Cannot load " << argv[1] << std::endl;
        return -1;
    }
    
    cv::namedWindow("Example Cube", cv::WINDOW_OPENGL);
    cv::resizeWindow("Example Cube", img.cols, img.rows);
    cv::createTrackbar("X-rotation", "Example Cube", &rotx, 360, on_trackbar);
    cv::createTrackbar("Y-rotation", "Example Cube", &roty, 360, on_trackbar);

    cv::ogl::Texture2D backgroundTex = cv::ogl::Texture2D(img);
    cv::setOpenGlDrawCallback("Example Cube", on_opengl, &backgroundTex);
    cv::updateWindow("Example Cube");
    
    cv::waitKey(0);

    cv::setOpenGlDrawCallback("Example Cube", 0, 0);
    cv::destroyAllWindows();

    return 0;
}
