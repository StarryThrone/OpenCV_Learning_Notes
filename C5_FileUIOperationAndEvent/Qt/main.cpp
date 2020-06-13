//
//  main.cpp
//  Qt
//
//  Created by chenjie on 2020/6/13.
//  Copyright © 2020 chenjie. All rights reserved.
//

#include <iostream>
#include <QApplication>
#include "QMoviePlayer.hpp"

int main(int argc, const char * argv[]) {
    std::cout << "\n\nExample. An example program ch4_qt.cpp, which takes a single argument"
              << "\nindicating a video file; that video file will be replayed inside of a Qt object"
              << "\nthat we will define, called QMoviePlayer"
              << "\nCall:\n" << argv[0] << " <path/movie>"
              << "\nExample:\n" << argv[0] << " ../tree.avi\n" << std::endl;

    if (argc != 2) {
        return -1;
    }

    QApplication app(argc, argv);
    QMoviePlayer mp;
    mp.open(argv[1]);
    mp.show();
    return app.exec();
}
