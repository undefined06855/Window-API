#include <Geode/modify/MenuLayer.hpp>
#include "Window.hpp"

class TouchDetector : public cocos2d::CCLayer {
public:
    static TouchDetector* create() {
        auto ret = new TouchDetector();
        ret->init();
        ret->autorelease();
        return ret;
    }

    cocos2d::CCSprite* m_indicator;

    bool init() {
        CCLayer::init();

        this->setContentSize(cocos2d::CCDirector::get()->getWinSize());
        this->ignoreAnchorPointForPosition(true);
        this->setTouchEnabled(true);

        m_indicator = cocos2d::CCSprite::createWithSpriteFrameName("GJ_diamondsIcon_001.png");
        this->addChild(m_indicator);
        m_indicator->setOpacity(90);

        return true;
    }

    bool ccTouchBegan(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) {
        geode::log::info("started at {}", touch->getLocation());
        m_indicator->setPosition(touch->getLocation());
        m_indicator->setOpacity(255);
        return true;
    }

    void ccTouchMoved(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) {
        geode::log::info("moved at {}", touch->getLocation());
        m_indicator->setPosition(touch->getLocation());
    }

    void ccTouchEnded(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) {
        geode::log::info("ended at {}", touch->getLocation());
        m_indicator->setPosition(touch->getLocation());
        m_indicator->setOpacity(90);
    }

    void registerWithTouchDispatcher() {
        cocos2d::CCTouchDispatcher::get()->addTargetedDelegate(this, getTouchPriority(), true);
    }
};

class $modify(MenuLayer) {
    struct Fields {
        std::vector<std::shared_ptr<window_api::Window>> windows;
    };

    bool init() {
        if (!MenuLayer::init()) return false;

        // auto label = cocos2d::CCLabelBMFont::create("Hello!!", "bigFont.fnt");
        // label->setScale(1.5f);

        // auto window = window_api::Window::create(label, "my label");
        // window->setCopyPosition(true);

        // label->runAction(cocos2d::CCRepeatForever::create(
        //     cocos2d::CCSequence::createWithTwoActions(
        //         cocos2d::CCEaseExponentialInOut::create(cocos2d::CCScaleTo::create(.8f, .5f)),
        //         cocos2d::CCEaseExponentialInOut::create(cocos2d::CCScaleTo::create(.8f, 1.5f))
        //     )
        // ));

        // label->setPosition({ -250.f, -100.f });

        // label->runAction(cocos2d::CCRepeatForever::create(
        //     cocos2d::CCSequence::create(
        //         cocos2d::CCEaseBackOut::create(cocos2d::CCMoveBy::create(2.f, { 250.f, 0.f })),
        //         cocos2d::CCEaseBackOut::create(cocos2d::CCMoveBy::create(2.f, { 0.f, 100.f })),
        //         cocos2d::CCEaseBackOut::create(cocos2d::CCMoveBy::create(2.f, { -250.f, 0.f })),
        //         cocos2d::CCEaseBackOut::create(cocos2d::CCMoveBy::create(2.f, { 0.f, -100.f })),
        //         nullptr
        //     )
        // ));

        // m_fields->windows.push_back(window);

        // m_fields->windows.push_back(window_api::Window::create(FLAlertLayer::create("hello", "hi 0", "ok")));
        // m_fields->windows.push_back(window_api::Window::create(FLAlertLayer::create("hello", "hi 1", "yeah")));
        // m_fields->windows.push_back(window_api::Window::create(FLAlertLayer::create("hello", "hi 2", "cool")));
        // m_fields->windows.push_back(window_api::Window::create(FLAlertLayer::create("hello", "hi 3", "damn")));
        // m_fields->windows.push_back(window_api::Window::create(FLAlertLayer::create("hello", "hi 4", "yep")));

        m_fields->windows.push_back(window_api::Window::create(TouchDetector::create()));

        return true;
    }
};
