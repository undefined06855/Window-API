#include "CCDirector.hpp"
#include "WindowManager.hpp"

using namespace window_api;

void HookedCCDirector::onModify(HookedCCDirector::SelfType& self) {
    (void)self.setHookPriorityPost("cocos2d::CCDirector::drawScene", geode::Priority::Late);
}

void HookedCCDirector::drawScene() {
    CCDirector::drawScene();
    WindowManager::get().visit();
}
