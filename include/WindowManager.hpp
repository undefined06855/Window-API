#pragma once
#include "Window.hpp"

namespace window_api {

class WINDOW_API_DLL WindowManager {
    WindowManager();
    ~WindowManager();
public:
    static WindowManager& get();

    std::deque<std::weak_ptr<Window>> m_windows;

    void addWindow(std::shared_ptr<Window> window);
    void visit();
};

}

