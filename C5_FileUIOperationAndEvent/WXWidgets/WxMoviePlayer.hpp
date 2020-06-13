//
//  WxMoviePlayer.hpp
//  WXWidgets
//
//  Created by chenjie on 2020/6/13.
//  Copyright © 2020 chenjie. All rights reserved.
//

#ifndef WxMoviePlayer_hpp
#define WxMoviePlayer_hpp

#include "wx/wx.h"
#include <string>
#define TIMER_ID 0

// 需要显示在窗口上的视图类都应该继承于wxWindow
class WxMoviePlayer : public wxWindow {
    public:
    WxMoviePlayer(wxWindow* parent, const wxPoint& pos, const wxSize& size);
    virtual ~WxMoviePlayer() {};
    bool open(wxString file);

    private:
    cv::VideoCapture m_cap;
    cv::Mat m_cv_img;
    // 和设备无关的图片
    wxImage m_wx_img;
    // 和设备相关的图片
    wxBitmap m_wx_bmp;
    wxTimer *m_timer;
    wxWindow *m_parent;

    void _copyImage();

    // 绘制事件，用于绘制视频帧
    void OnPaint(wxPaintEvent& e);
    // 时钟事件，用于刷新视频帧
    void OnTimer(wxTimerEvent& e);
    // 键盘事件，用于监听ESC输入退出程序
    void OnKey(wxKeyEvent& e);

    protected:
    DECLARE_EVENT_TABLE();
};

#endif /* WxMoviePlayer_hpp */
