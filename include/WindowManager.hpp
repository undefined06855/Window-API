#pragma once
#include "Window.hpp"

namespace window_api {

/**
 * Handles all windows. That's it!
 */
class WINDOW_API_DLL WindowManager {
    WindowManager();
    ~WindowManager();
public:
    static WindowManager& get();

    std::deque<std::weak_ptr<Window>> m_windows;

    /**
     * Gets a window by its window ID.
     */
    std::shared_ptr<Window> getWindowByID(geode::ZStringView id);

    /**
     * Gets a window by the node that it is rendering. This will return nullptr if the node is being rendered by the
     * vanilla GD window!
     */
    std::shared_ptr<Window> getWindowByNode(cocos2d::CCNode* node);

    /**
     * Gets a window by traversing this node's parents until it finds one without a parent, then calls getWindowByNode
     * on it, so this can be used to find what window this node is rendering on. This will return nullptr if the node
     * is in the vanilla GD window!
     */
    std::shared_ptr<Window> findWindowForNode(cocos2d::CCNode* node);

    /**
     * Adds a window. This is called for you in Window::create, you do not have to call it yourself.
     */
    void addWindow(std::shared_ptr<Window> window);

    /**
     * Visits all windows and removes invalid windows from m_windows.
     */
    void visit();
};

}

