//
//  WxMoviePlayer.cpp
//  WXWidgets
//
//  Created by chenjie on 2020/6/13.
//  Copyright © 2020 chenjie. All rights reserved.
//

#include "WxMoviePlayer.hpp"

// 注册事件
BEGIN_EVENT_TABLE(WxMoviePlayer, wxWindow)
    EVT_PAINT(WxMoviePlayer::OnPaint)
    EVT_TIMER(TIMER_ID, WxMoviePlayer::OnTimer)
    EVT_CHAR(WxMoviePlayer::OnKey)
END_EVENT_TABLE()

// 构造函数
WxMoviePlayer::WxMoviePlayer(wxWindow* parent, const wxPoint& pos, const wxSize& size)
: wxWindow(parent, -1, pos, size, wxSIMPLE_BORDER) {
    // 打开视频文件时再设置时钟
    m_timer = NULL;
    // 记录父框架
    m_parent = parent;
}

// 绘制函数
void WxMoviePlayer::OnPaint(wxPaintEvent& event) {
    wxPaintDC dc(this);
    if (!dc.Ok()) {
        return;
    }

    int x,y,w,h;
    dc.BeginDrawing();
    dc.GetClippingBox(&x, &y, &w, &h);
    // m_wx_bmp是wxBitmap实例，它和设备相关，可以直接展示
    dc.DrawBitmap(m_wx_bmp, x, y);
    dc.EndDrawing();

    return;
}

// 拷贝图像
void WxMoviePlayer::_copyImage() {
    m_wx_bmp = wxBitmap(m_wx_img);
    // 标记窗口需要重绘
    Refresh(FALSE);
    // 强制执行窗口刷新检查，如果检查到窗口需要重会，则调用重绘函数
    Update();
}

// 打开视频文件
bool WxMoviePlayer::open(wxString file) {
    // 使用CV接口打开一个视频文件
    if (!m_cap.open(std::string(file.mb_str()))) {
        return false;
    }
    
    // 使用CV接口读取当前视频帧
    m_cap.read(m_cv_img);
    // 使用CV的图片数据创建wxWidgets的设备无关图片实例，需要注意这里不存在数据拷贝，指向的都是同一份图像数据
    m_wx_img = wxImage(m_cv_img.cols, m_cv_img.rows, m_cv_img.data, TRUE);

    // 将设备无关图像转换为设备相关位图
    _copyImage();

    // 创建时钟
    m_timer = new wxTimer(this, TIMER_ID);
    // 按指定间隔运行时钟，每个时间间隔结束后都会产生一个wxTimerEvent事件，最后会调用回调函数
    m_timer->Start(1000. / m_cap.get(cv::CAP_PROP_FPS));

    return true;
}

// 时钟的回调函数
void WxMoviePlayer::OnTimer(wxTimerEvent& event) {
    if (!m_cap.isOpened()) {
        return;
    }

    // 读取当前视频帧
    m_cap.read(m_cv_img);
    // 转换颜色格式
    cv::cvtColor(m_cv_img, m_cv_img, cv::BGR2RGB);
    // 从设备无关图像转换为设备相关位图
    _copyImage();
}

// 键盘事件的回调函数
void WxMoviePlayer::OnKey(wxKeyEvent& e) {
    if (e.GetKeyCode() == WXK_ESCAPE) {
        // 关闭框架，结束程序
        m_parent->Close();
    }
}
