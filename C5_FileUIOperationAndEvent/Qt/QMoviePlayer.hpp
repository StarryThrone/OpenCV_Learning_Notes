//
//  QMoviePlayer.hpp
//  Qt
//
//  Created by chenjie on 2020/6/13.
//  Copyright © 2020 chenjie. All rights reserved.
//

#ifndef QMoviePlayer_hpp
#define QMoviePlayer_hpp

#include "ui_QMoviePlayer.h"
#include <string>
#include <opencv2/videoio.hpp>

class QMoviePlayer : public QWidget {
  Q_OBJECT;
  public:
  QMoviePlayer(QWidget *parent = NULL);
  virtual ~QMoviePlayer() {};
  bool open(std::string file);
  private:
  Ui::QMoviePlayer ui;
  cv::VideoCapture m_cap;
  QImage m_qt_img;
  cv::Mat m_cv_img;
  QTimer* m_timer;
  void paintEvent(QPaintEvent *q);
  void _copyImage();
  public slots:
  void nextFrame();
};

#endif /* QMoviePlayer_hpp */
