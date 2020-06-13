//
//  main.cpp
//  WXWidgets
//
//  Created by chenjie on 2020/6/13.
//  Copyright © 2020 chenjie. All rights reserved.
//

#include "wx/wx.h"
#include "WxMoviePlayer.hpp"

// 继承wxWidgets中的顶层类wxApp定义类MyApp
class MyApp : public wxApp {
    public:
    virtual bool OnInit();
};

// 创建main函数，创建MyApp实例作为应用程序，并将其和main函数绑定
DECLARE_APP(MyApp);
IMPLEMENT_APP(MyApp);

// 重写应用程序运行后调用的函数
bool MyApp::OnInit() {
    // 创建帧(窗口管理对象)
    wxFrame* frame = new wxFrame(NULL, wxID_ANY, wxT("ch4_wx"));
    frame->Show(true);
    
    // 创建我们自己编写的视频播放器，并和该窗口关联
    WxMoviePlayer *mp = new WxMoviePlayer(frame, wxPoint(-1, -1), wxSize(640, 480));
    // 打开某个视频文件
    mp->open(wxString(argv[1]));
    mp->Show(true);

    return true;
}

