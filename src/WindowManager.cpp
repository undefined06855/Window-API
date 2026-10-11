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

std::shared_ptr<Window> WindowManager::getWindowByID(geode::ZStringView id) {
    for (auto window : m_windows) {
        if (window.expired()) continue;
        if (window.lock()->getID() == id) return window.lock();
    }

    return nullptr;
}

std::shared_ptr<Window> WindowManager::getWindowByNode(cocos2d::CCNode* node) {
    for (auto window : m_windows) {
        if (window.expired()) continue;
        if (window.lock()->getNode() == node) return window.lock();
    }

    return nullptr;
}

std::shared_ptr<Window> WindowManager::findWindowForNode(cocos2d::CCNode* node) {
    auto parent = node;
    while (parent->getParent() != nullptr) {
        parent = parent->getParent();
    }

    // parent is now the rendernode
    auto wrapper = parent->getChildrenExt()[0];
    auto realNode = wrapper->getChildrenExt()[0];

    return this->getWindowByNode(realNode);
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
