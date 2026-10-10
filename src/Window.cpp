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

    glfwSetCursorPosCallback(m_impl->window, [](GLFWwindow*, double x, double y) {

    });

    glfwSetWindowFocusCallback(m_impl->window, [](GLFWwindow*, int focus) {

    });

    glfwSetWindowIconifyCallback(m_impl->window, [](GLFWwindow*, int minimized) {

    });

    glfwSetWindowSizeCallback(m_impl->window, [](GLFWwindow*, int width, int height) {

    });

    glfwSetKeyCallback(m_impl->window, [](GLFWwindow*, int key, int scancode, int action, int mods) {

    });

    glfwSetMouseButtonCallback(m_impl->window, [](GLFWwindow*, int button, int action, int mods) {

    });

    glfwSetScrollCallback(m_impl->window, [](GLFWwindow*, double x, double y) {

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

float Window::getScale() {
    return m_impl->scale;
}

bool Window::getShouldClose() {
    return glfwWindowShouldClose(m_impl->window);
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
    m_impl->node->setPosition(m_impl->renderNode->getContentSize() / 2.f);
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
