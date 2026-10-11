#include "Window.hpp"
#include "WindowManager.hpp"
#include "robglfw.hpp"
#include "DrawingData.hpp"
#include <alphalaneous.alphas-ui-pack/include/nodes/RenderNode.hpp>

using namespace window_api;

struct Window::Impl final {
    GLFWwindow* window = nullptr;
    std::optional<DrawingData> drawingData;

    std::string id = "";

    float scale = 1.f;

    bool capturedTouch = false;
    geode::Ref<cocos2d::CCTouch> touch = nullptr;
    cocos2d::CCPoint mousePos = { 0.f, 0.f };

    bool minimized = false;
    bool maximized = false;
    bool focused = false;

    bool copySize = true;
    bool copyPosition = false;

    geode::Ref<alpha::ui::RenderNode> renderNode = nullptr;
    geode::Ref<cocos2d::CCNode> node = nullptr;
};

Window::Window(cocos2d::CCNode* node, geode::ZStringView title)
    : m_impl(std::make_unique<Impl>()) {
    auto view = cocos2d::CCEGLView::get();

    // glsl 1.50
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

    // fake window so that it can share resources and shit
    // thanks mat for a struct layout for rob's glfwwindow though it was a bit different
    uint8_t fakeWindowForModernGLFW[1008] = {};
    auto robtop = (uint8_t*)view->m_pMainWindow;
    std::memcpy(&fakeWindowForModernGLFW[536], &robtop[512], 8);
    std::memcpy(&fakeWindowForModernGLFW[728], &robtop[640], 8);

    // this->unswapContexts();

    m_impl->window = glfwCreateWindow(
        std::max(1.f, node->getScaledContentWidth() * view->m_fFrameZoomFactor),
        std::max(1.f, node->getScaledContentHeight() * view->m_fFrameZoomFactor),
        title.c_str(),
        nullptr,
        (GLFWwindow*)fakeWindowForModernGLFW
    );

    this->unswapContexts();

    const char* err;
    int code = glfwGetError(&err);
    if (code != GLFW_NO_ERROR) {
        geode::log::error("Error when making window! {} ({})", err, code);
    }

    // scary...
    glfwSetWindowUserPointer(m_impl->window, this);

    glfwSetKeyCallback(m_impl->window, [](GLFWwindow* _window, int key, int scancode, int action, int mods) {
        auto window = static_cast<Window*>(glfwGetWindowUserPointer(_window));
        auto cocosKey = cocos2d::CCKeyboardDispatcher::convertKeyCode((cocos2d::enumKeyCodes)key);

        bool shift = mods & GLFW_MOD_SHIFT;
        bool ctrl = mods & GLFW_MOD_CONTROL;
        bool alt = mods & GLFW_MOD_ALT;
        bool cmd = mods & GLFW_MOD_SUPER;

        auto hasDelegate = cocos2d::CCIMEDispatcher::sharedDispatcher()->hasDelegate();

        if (action == GLFW_RELEASE) {
            if (!hasDelegate) {
                window->keyUp(cocosKey, shift, ctrl, alt, cmd);
            }

            return;
        }

        if (hasDelegate) {
            if (cocosKey == cocos2d::KEY_Left || cocosKey == cocos2d::KEY_Right) {
                // robtop functionality lol
                window->insertText("a", cocosKey);
            }

            if (cocosKey == cocos2d::KEY_Delete) {
                window->deleteForward();
            }

            if (cocosKey == cocos2d::KEY_Backspace) {
                window->deleteBackward();
            }
        }

        if (!hasDelegate || cocosKey == cocos2d::KEY_Escape || cocosKey == cocos2d::KEY_Enter) { // TODO: is this right?
            window->keyDown(cocosKey, shift, ctrl, alt, cmd);
        }
    });

    glfwSetCharCallback(m_impl->window, [](GLFWwindow* _window, unsigned int codepoint) {
        auto window = static_cast<Window*>(glfwGetWindowUserPointer(_window));
        auto hasDelegate = cocos2d::CCIMEDispatcher::sharedDispatcher()->hasDelegate();
        if (!hasDelegate) return;

        if (codepoint == 0x8) {
            window->deleteBackward();
        } else {
            window->insertText(geode::ZStringView(fmt::format("{:c}", codepoint)), cocos2d::KEY_Unknown);
        }
    });

    glfwSetCursorPosCallback(m_impl->window, [](GLFWwindow* _window, double x, double y) {
        auto window = static_cast<Window*>(glfwGetWindowUserPointer(_window));
        auto pos = window->getCocosPos({ (float)x, (float)y });
        window->m_impl->mousePos = pos;
        window->moveTouch(pos);
    });

    glfwSetMouseButtonCallback(m_impl->window, [](GLFWwindow* _window, int button, int action, int mods) {
        auto window = static_cast<Window*>(glfwGetWindowUserPointer(_window));

        if (button != GLFW_MOUSE_BUTTON_LEFT) return; // TODO: different button support?

        if (action == GLFW_PRESS) {
            window->beginTouch(window->m_impl->mousePos);
        } else if (action == GLFW_RELEASE) {
            window->endTouch(window->m_impl->mousePos);
        }
    });

    glfwSetScrollCallback(m_impl->window, [](GLFWwindow* _window, double x, double y) {
        auto window = static_cast<Window*>(glfwGetWindowUserPointer(_window));
    });

    glfwSetWindowIconifyCallback(m_impl->window, [](GLFWwindow* _window, int minimized) {
        auto window = static_cast<Window*>(glfwGetWindowUserPointer(_window));
        window->m_impl->minimized = minimized;
    });

    glfwSetWindowMaximizeCallback(m_impl->window, [](GLFWwindow* _window, int maximized) {
        auto window = static_cast<Window*>(glfwGetWindowUserPointer(_window));
        window->m_impl->maximized = maximized;
    });

    glfwSetWindowFocusCallback(m_impl->window, [](GLFWwindow* _window, int focused) {
        auto window = static_cast<Window*>(glfwGetWindowUserPointer(_window));
        window->m_impl->focused = focused;
    });

    m_impl->node = node;
    node->onEnter();
    node->onEnterTransitionDidFinish();

    // TODO: this is not correct
    int width, height;
    robtop::glfwGetWindowSize(view->m_pMainWindow, &width, &height);
    m_impl->scale = height / cocos2d::CCDirector::get()->getWinSize().height;

    this->makeRenderNode();

    this->swapContexts();
    m_impl->drawingData.emplace();
    this->unswapContexts();
}

void Window::makeRenderNode() {
    auto wrapper = cocos2d::CCNode::create();
    wrapper->setAnchorPoint({ .5f, .5f });
    wrapper->setContentSize(this->getSize());
    wrapper->addChild(m_impl->node);
    m_impl->renderNode = alpha::ui::RenderNode::create(wrapper, true);
    m_impl->renderNode->initFBO();
}

void Window::swapContexts() {
    glfwMakeContextCurrent(m_impl->window);

    int width, height;
    glfwGetFramebufferSize(m_impl->window, &width, &height);
    glViewport(0, 0, width, height);
}

void Window::unswapContexts() {
    auto view = cocos2d::CCEGLView::get();
    if (!view) return; // could happen in window dtor
    robtop::glfwMakeContextCurrent(view->m_pMainWindow);
}

// windowmanager pool stores weak ptrs so they get removed in visit, don't have to remove them here
Window::~Window() {
    glfwDestroyWindow(m_impl->window);
    geode::log::info("destroying window");
    this->unswapContexts();

    auto node = m_impl->node;
    if (node && node->isRunning()) {
        node->onExitTransitionDidStart();
        node->onExit();
    }
}

// just a wrapper over make shared and adding it to the pool
std::shared_ptr<Window> Window::create(cocos2d::CCNode* node, geode::ZStringView title) {
    auto ret = std::shared_ptr<Window>(new Window(node, title));
    WindowManager::get().addWindow(ret);
    return ret;
}

GLFWwindow* Window::getHandle() {
    return m_impl->window;
}

void Window::setID(geode::ZStringView id) {
    m_impl->id = id;
}

void Window::setCopyPosition(bool copy) {
    m_impl->copyPosition = copy;
    this->updateProperties();
}

void Window::setCopySize(bool copy) {
    m_impl->copySize = copy;
    this->updateProperties();
}

void Window::setPosition(cocos2d::CCPoint pos) {
    auto mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    auto center = cocos2d::CCPoint{mode->width / 2.f, mode->height / 2.f};
    auto size = this->getRealSize();

    auto newPos = (pos * m_impl->scale) + center - (size / 2.f);
    glfwSetWindowPos(m_impl->window, newPos.x, newPos.y);
}

void Window::setSize(cocos2d::CCSize size) {
    glfwSetWindowSize(m_impl->window, size.width * m_impl->scale, size.height * m_impl->scale);
}

void Window::setScale(float scale) {
    m_impl->scale = scale;
    this->updateProperties();
}

void Window::setMinimized(bool minimized) {
    if (minimized) {
        glfwIconifyWindow(m_impl->window);
    } else {
        glfwRestoreWindow(m_impl->window);
    }
}

void Window::setMaximized(bool maximized) {
    if (maximized) {
        glfwMaximizeWindow(m_impl->window);
    } else {
        glfwRestoreWindow(m_impl->window);
    }
}

void Window::focus() {
    glfwFocusWindow(m_impl->window);
}

std::pair<cocos2d::CCArray*, cocos2d::CCArray*> Window::filterTouchDispatcher() {
    auto dispatcher = cocos2d::CCTouchDispatcher::get();
    auto& manager = WindowManager::get();

    auto origStandardHandlers = dispatcher->m_pStandardHandlers->shallowCopy();
    auto origTargetedHandlers = dispatcher->m_pTargetedHandlers->shallowCopy();

    auto filterList = [&](cocos2d::CCArray* handlers) {
        for (int i = 0; i < handlers->count(); i++) {
            auto handler = static_cast<cocos2d::CCTouchHandler*>(handlers->objectAtIndex(i));
            auto delegate = geode::cast::typeinfo_cast<cocos2d::CCNode*>(handler->getDelegate());
            if (!delegate) continue;

            if (manager.findWindowForNode(delegate).get() != this) {
                handlers->removeObjectAtIndex(i);
                i--;
            }
        }
    };

    filterList(dispatcher->m_pStandardHandlers);
    filterList(dispatcher->m_pTargetedHandlers);

    return { origStandardHandlers, origTargetedHandlers };
}

void Window::unfilterTouchDispatcher(std::pair<cocos2d::CCArray*, cocos2d::CCArray*> handlers) {
    auto dispatcher = cocos2d::CCTouchDispatcher::get();
    dispatcher->m_pStandardHandlers->release();
    dispatcher->m_pTargetedHandlers->release();
    dispatcher->m_pStandardHandlers = handlers.first;
    dispatcher->m_pTargetedHandlers = handlers.second;
    dispatcher->m_pStandardHandlers->retain();
    dispatcher->m_pTargetedHandlers->retain();
}

void Window::beginTouch(cocos2d::CCPoint pos) {
    if (m_impl->capturedTouch) return;
    m_impl->capturedTouch = true;

    m_impl->touch = new cocos2d::CCTouch();
    m_impl->touch->autorelease();
    m_impl->touch->setTouchInfo(1, pos.x, pos.y);

    auto dispatcher = cocos2d::CCTouchDispatcher::get();

    auto origHandlers = this->filterTouchDispatcher();
    cocos2d::CCSet touches;
    touches.addObject(m_impl->touch);
    dispatcher->touchesBegan(&touches, nullptr);
    this->unfilterTouchDispatcher(origHandlers);
}

void Window::endTouch(cocos2d::CCPoint pos) {
    if (!m_impl->capturedTouch) return;
    m_impl->capturedTouch = false;

    m_impl->touch->setTouchInfo(1, pos.x, pos.y);

    auto dispatcher = cocos2d::CCTouchDispatcher::get();

    auto origHandlers = this->filterTouchDispatcher();
    cocos2d::CCSet touches;
    touches.addObject(m_impl->touch);
    dispatcher->touchesEnded(&touches, nullptr);
    this->unfilterTouchDispatcher(origHandlers);
}

void Window::moveTouch(cocos2d::CCPoint pos) {
    if (!m_impl->capturedTouch) return;

    m_impl->touch->setTouchInfo(1, pos.x, pos.y);

    auto dispatcher = cocos2d::CCTouchDispatcher::get();

    auto origHandlers = this->filterTouchDispatcher();
    cocos2d::CCSet touches;
    touches.addObject(m_impl->touch);
    dispatcher->touchesMoved(&touches, nullptr);
    this->unfilterTouchDispatcher(origHandlers);
}

cocos2d::CCArray* Window::filterKeyboardDispatcher() {
    auto dispatcher = cocos2d::CCKeyboardDispatcher::get();
    auto& manager = WindowManager::get();

    auto origDelegates = dispatcher->m_pDelegates->shallowCopy();

    for (int i = 0; i < dispatcher->m_pDelegates->count(); i++) {
        auto delegate = geode::cast::typeinfo_cast<cocos2d::CCNode*>(dispatcher->m_pDelegates->objectAtIndex(i));
        if (!delegate) continue;

        if (manager.findWindowForNode(delegate).get() != this) {
            dispatcher->m_pDelegates->removeObjectAtIndex(i);
            i--;
        }
    }

    return origDelegates;
}

void Window::unfilterKeyboardDispatcher(cocos2d::CCArray* handlers) {
    auto dispatcher = cocos2d::CCKeyboardDispatcher::get();
    dispatcher->m_pDelegates->release();
    dispatcher->m_pDelegates = handlers;
    dispatcher->m_pDelegates->retain();
}

void Window::keyDown(cocos2d::enumKeyCodes key, bool shift, bool ctrl, bool alt, bool cmd) {
    auto origHandlers = this->filterKeyboardDispatcher();

    auto dispatcher = cocos2d::CCKeyboardDispatcher::get();
    dispatcher->dispatchKeyboardMSG(key, true, false, 0);

    this->unfilterKeyboardDispatcher(origHandlers);
}

void Window::keyUp(cocos2d::enumKeyCodes key, bool shift, bool ctrl, bool alt, bool cmd) {
    auto origHandlers = this->filterKeyboardDispatcher();

    auto dispatcher = cocos2d::CCKeyboardDispatcher::get();
    dispatcher->dispatchKeyboardMSG(key, false, false, 0);

    this->unfilterKeyboardDispatcher(origHandlers);
}

struct IMEImpl {
    cocos2d::CCIMEDelegate* delegate;
};

cocos2d::CCIMEDelegate* Window::filterIMEDelegate() {
    auto dispatcher = cocos2d::CCIMEDispatcher::sharedDispatcher();
    auto& manager = WindowManager::get();

    // ccimedispatcher is pimpl, where the impl struct has one member whcih is the ccimedelegate
    auto impl = *(IMEImpl**)dispatcher;

    auto node = geode::cast::typeinfo_cast<cocos2d::CCNode*>(impl->delegate);
    if (!node) return impl->delegate;

    if (manager.findWindowForNode(node).get() != this) {
        impl->delegate = nullptr;
    }

    return impl->delegate;
}

void Window::unfilterIMEDelegate(cocos2d::CCIMEDelegate* delegate) {
    auto dispatcher = cocos2d::CCIMEDispatcher::sharedDispatcher();
    auto& manager = WindowManager::get();
    auto impl = *(IMEImpl**)dispatcher;

    impl->delegate = delegate;
}

void Window::insertText(geode::ZStringView text, cocos2d::enumKeyCodes keys) {
    auto delegate = this->filterIMEDelegate();

    auto dispatcher = cocos2d::CCIMEDispatcher::sharedDispatcher();
    dispatcher->dispatchInsertText(text.c_str(), text.length(), keys);

    this->unfilterIMEDelegate(delegate);
}

void Window::deleteForward() {
    auto delegate = this->filterIMEDelegate();

    auto dispatcher = cocos2d::CCIMEDispatcher::sharedDispatcher();
    dispatcher->dispatchDeleteForward();

    this->unfilterIMEDelegate(delegate);
}

void Window::deleteBackward() {
    auto delegate = this->filterIMEDelegate();

    auto dispatcher = cocos2d::CCIMEDispatcher::sharedDispatcher();
    dispatcher->dispatchDeleteBackward();

    this->unfilterIMEDelegate(delegate);
}

cocos2d::CCNode* Window::getNode() {
    return m_impl->node;
}

geode::ZStringView Window::getID() {
    return m_impl->id;
}

bool Window::getCopyPosition() {
    return m_impl->copyPosition;
}

bool Window::getCopySize() {
    return m_impl->copySize;
}

cocos2d::CCPoint Window::getRealPos() {
    int x, y;
    glfwGetWindowPos(m_impl->window, &x, &y);
    return { (float)x, (float)y };
}

cocos2d::CCPoint Window::getPos() {
    auto mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    auto center = cocos2d::CCPoint{mode->width / 2.f, mode->height / 2.f};
    auto size = this->getRealSize();

    return (this->getRealPos() + size / 2.f - center) / m_impl->scale;
}

cocos2d::CCSize Window::getRealSize() {
    int width, height;
    glfwGetWindowSize(m_impl->window, &width, &height);
    return cocos2d::CCSize{ (float)width, (float)height };
}

cocos2d::CCSize Window::getSize() {
    return this->getRealSize() / m_impl->scale;
}

bool Window::getShouldClose() {
    return glfwWindowShouldClose(m_impl->window);
}

bool Window::getMinimized() {
    return m_impl->minimized;
}

bool Window::getMaximized() {
    return m_impl->maximized;
}

bool Window::getFocused() {
    return m_impl->focused;
}

float Window::getScale() {
    return m_impl->scale;
}

bool Window::getCapturedTouch() {
    return m_impl->capturedTouch;
}

cocos2d::CCPoint Window::getCocosPos(cocos2d::CCPoint glfwPos) {
    auto height = this->getRealSize().height;
    // glfwPos.y = height - glfwPos.y;
    return glfwPos / m_impl->scale;
}

void Window::updateProperties() {
    if (m_impl->copyPosition) {
        this->setPosition(m_impl->node->getPosition());
    }

    if (m_impl->copySize) {
        this->setSize(m_impl->node->getScaledContentSize());
    }

    if (m_impl->renderNode->getContentSize() != this->getSize()) {
        this->makeRenderNode();
    }
}

void Window::visit() {
    this->updateProperties();

    this->unswapContexts();

    auto origPos = m_impl->node->getPosition();
    m_impl->node->setPosition(m_impl->renderNode->getContentSize() * (m_impl->node->isIgnoreAnchorPointForPosition() ? cocos2d::CCPoint{ 0.f, 0.f } : m_impl->node->getAnchorPoint()));
    cocos2d::ccGLBindTexture2D(123456); // cocos bound texture cache is broken? invalidateCache seems to break everything...
    m_impl->renderNode->render();
    m_impl->node->setPosition(origPos);

    this->swapContexts();

    auto tex = m_impl->renderNode->getTexture()->getName();
    if (!glIsTexture(tex)) {
        geode::log::error("sharing not working!!!!");
    }

    // some of this could probably only be done once in this context but whatever
    auto& renderData = m_impl->drawingData.value();
    glUseProgram(renderData.shader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1i(renderData.textureLocation, 0);

    int width, height;
    glfwGetFramebufferSize(m_impl->window, &width, &height);
    glUniform2f(renderData.winSizeLocation, (float)width, (float)height);

    glBindVertexArray(renderData.vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glfwSwapBuffers(m_impl->window);

    glBindVertexArray(0);

    this->unswapContexts();
}
