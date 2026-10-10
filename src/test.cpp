#include <Geode/modify/MenuLayer.hpp>
#include "Window.hpp"

class $modify(MenuLayer) {
    struct Fields {
        std::shared_ptr<window_api::Window> window;
    };

    bool init() {
        if (!MenuLayer::init()) return false;

        auto label = cocos2d::CCLabelBMFont::create("Hello!!", "bigFont.fnt");
        label->setScale(1.5f);

        auto window = window_api::Window::create(label, "my label");
        window->setCopyPosition(true);

        label->runAction(cocos2d::CCRepeatForever::create(
            cocos2d::CCSequence::createWithTwoActions(
                cocos2d::CCEaseExponentialInOut::create(cocos2d::CCScaleTo::create(.8f, .5f)),
                cocos2d::CCEaseExponentialInOut::create(cocos2d::CCScaleTo::create(.8f, 1.5f))
            )
        ));

        label->setPosition({ -250.f, -100.f });

        label->runAction(cocos2d::CCRepeatForever::create(
            cocos2d::CCSequence::create(
                cocos2d::CCEaseBackOut::create(cocos2d::CCMoveBy::create(2.f, { 250.f, 0.f })),
                cocos2d::CCEaseBackOut::create(cocos2d::CCMoveBy::create(2.f, { 0.f, 100.f })),
                cocos2d::CCEaseBackOut::create(cocos2d::CCMoveBy::create(2.f, { -250.f, 0.f })),
                cocos2d::CCEaseBackOut::create(cocos2d::CCMoveBy::create(2.f, { 0.f, -100.f })),
                nullptr
            )
        ));

        m_fields->window = window;

        return true;
    }
};
