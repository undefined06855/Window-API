#pragma once
#include <Geode/modify/CCDirector.hpp>

class $modify(HookedCCDirector, cocos2d::CCDirector) {
    using SelfType = ModifyBase<geode::modifier::ModifyDerive<HookedCCDirector, cocos2d::CCDirector>>;
    static void onModify(SelfType& self);
    void drawScene();
};
