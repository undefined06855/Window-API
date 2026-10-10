#pragma once
#include "includes.hpp"

namespace window_api {

/**
 * A window. When this class is constructed, the window is made, and when it is destructed, the window is closed.
 * Everything in this can only be called from the main thread safely! Do not call this from threads!
 */
class WINDOW_API_DLL Window {
private:
    Window(cocos2d::CCNode* node, geode::ZStringView title);
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    struct Impl;
    std::unique_ptr<Impl> m_impl;

    void makeRenderNode();

    void swapContexts();
    void unswapContexts();
public:
    /**
     * Creates a window with a size and title. This will create the actual window itself and call glfwCreateWindow, if
     * this ever gets destructed, the real window will close.
     */
    static std::shared_ptr<Window> create(cocos2d::CCNode* node, geode::ZStringView title = "Geometry Dash");

    /**
     * Destructs the window and removes it from WindowManager.
     */
    ~Window();

    /**
     * Gets the pointer to the raw GLFWwindow object, though it is likely you won't be able to do much with this.
     * If you want more features, make a PR to the GitHub repo!
     */
    GLFWwindow* getHandle();

    /**
     * Sets the ID for this window, for inter-mod window communications.
     */
    void setID(geode::ZStringView id);

    /**
     * Sets whether the window should copy the position of the node it holds.
     */
    void setCopyPosition(bool copy);

    /**
     * Sets whether the window should copy the size of the node it holds.
     */
    void setCopySize(bool copy);

    /**
     * Sets the position of the window. Does nothing if the window is copying the position of the node it holds.
     */
    void setPosition(cocos2d::CCPoint pos);

    /**
     * Sets the size of the window. Does nothing if the window is copying the size of the node it holds.
     */
    void setSize(cocos2d::CCSize size);

    /**
     * Sets the scale factor of the window from cocos units to window pixels. Will be CCScene height / GD Window height
     * by default.
     */
    void setScale(float scale);

    /**
     * Gets the node that this window is rendering. Be warned when storing this, since it may not be tied to the
     * lifetime of the window and may just be WeakRef'd!
     */
    cocos2d::CCNode* getNode();

    /**
     * Gets the ID of this window.
     */
    geode::ZStringView getID();

    /**
     * Gets whether the window should copy the position of the node it holds.
     */
    bool getCopyPosition();

    /**
     * Gets whether the window should copy the size of the node it holds.
     */
    bool getCopySize();

    /**
     * Gets the real position of the window.
     */
    cocos2d::CCPoint getRealPos();

    /**
     * Gets the position of the window.
     */
    cocos2d::CCPoint getPos();

    /**
     * Gets the real size of the window (including any scaling).
     */
    cocos2d::CCSize getRealSize();

    /**
     * Gets the size of the window (excluding any scaling).
     */
    cocos2d::CCSize getSize();

    /**
     * Gets the scale factor of the window from cocos units to window pixels.
     */
    float getScale();

    /**
     * Gets whether the window should close from GLFW. If your window is a traditional one, you should check this often
     * and discard of your window instance if it returns true.
     */
    bool getShouldClose();

    /**
     * Updates the size and position of the window if copy is enabled, and remakes the texture if the window size
     * changes. This will be called for you in Window::visit, so you should not have to call it yourself.
     */
    void updateProperties();

    /**
     * Renders the window.
     */
    void visit();
};

}
