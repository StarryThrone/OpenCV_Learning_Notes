//
//  QMoviePlayer.cpp
//  Qt
//
//  Created by chenjie on 2020/6/13.
//  Copyright © 2020 chenjie. All rights reserved.
//

#include "QMoviePlayer.hpp"
#include <QTimer>
#include <QPainter>

QMoviePlayer::QMoviePlayer(QWidget *parent) : QWidget(parent) {
    ui.setupUi(this);
}

bool QMoviePlayer::open(string file) {
    // 使用OpenCV接口打开视频文件
    if (!m_cap.open(file)) {
        return false;
    }
    
    // 读取第一帧到OpenCV的图片模型中
    m_cap.read(m_cv_img);
    // 创建Qt库图像模型
    m_qt_img = QImage(QSize(m_cv_img.cols, m_cv_img.rows), QImage::Format_RGB888);
    // 设置Qt库视频播放器UI的逻辑分辨率
    ui.frame->setMinimumSize(m_qt_img.width(), m_qt_img.height());
    ui.frame->setMaximumSize(m_qt_img.width(), m_qt_img.height());
    // 将图像数据从CV模型拷贝到Qt模型
    _copyImage();

    // 创建时钟并绑定事件，每个设定的间隔结束后都会调用函数nextFrame()
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(nextFrame()));
    // 运行时钟，时间间隔为1000/视频帧率，即每帧画面的显示时间，单位为毫秒
    m_timer->start(1000. / m_cap.get(cv::CAP_PROP_FPS));

    return true;
}

void QMoviePlayer::_copyImage() {
    // 首先使用m_qt_img的数据段指针创建一个CV的图片模型
    cv::Mat cv_header_to_qt_image(cv::Size(m_qt_img.width(), m_qt_img.height()), CV_8UC3, m_qt_img.bits());
    // 使用CV的函数将m_cv_img数据转换后拷贝至cv_header_to_qt_image中，从而将其拷贝到m_qt_img中
    cv::cvtColor(m_cv_img, cv_header_to_qt_image, cv::BGR2RGB);
}

void QMoviePlayer::nextFrame() {
    if (!m_cap.isOpened()) {
        return;
    }
    // 读取当前帧图像到CV模型内
    m_cap.read(m_cv_img);
    // 将图像数据从CV模型拷贝到Qt模型
    _copyImage();
    // 通知Qt库需要刷新页面
    this->update();
}

void QMoviePlayer::paintEvent(QPaintEvent* e) {
    // 创建绘制器
    QPainter painter(this);
    // 绘制图像
    painter.drawImage(QPoint( ui.frame->x(), ui.frame->y()), m_qt_img);
}
