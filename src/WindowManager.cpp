#include "WindowManager.hpp"
#include "robglfw.hpp"

using namespace window_api;

WindowManager::WindowManager()
    : m_windows() {
    if (!glfwInit()) {
        geode::log::error("Could not initialise GLFW!!");
        return;
    }
}

WindowManager::~WindowManager() {
    glfwTerminate();
}

WindowManager& WindowManager::get() {
    static WindowManager instance;
    return instance;
}

void WindowManager::addWindow(std::shared_ptr<Window> window) {
    m_windows.push_back(window);
}

void WindowManager::visit() {
    auto director = cocos2d::CCDirector::get();
    auto origWinSize = director->m_obWinSizeInPoints;

    for (auto it = m_windows.begin(); it != m_windows.end();) {
        if (it->expired()) {
            it = m_windows.erase(it);
            continue;
        }

        auto window = it->lock();
        window->visit();
        it++;
    }

    robtop::glfwMakeContextCurrent(cocos2d::CCEGLView::get()->m_pMainWindow);
}

$on_mod(Loaded) {
    WindowManager::get();
}
