#pragma once
#include "includes.hpp"

namespace robtop {

static_assert(GEODE_COMP_GD_VERSION == 22081, "You need to find GLFW function addresses for your current version!");

// > cocos2d::CCEGLView::setupWindow @ 0x180078af8
// > sub_1800D44F0 @ 0x1800d47d3
// > sub_1800D4950 @ 0x1800d49d1
// > <???> @ 0x1801ba53c
static auto glfwMakeContextCurrent = reinterpret_cast<decltype(&::glfwMakeContextCurrent)>(reinterpret_cast<uintptr_t>(geode::base::getCocos() + 0xd33f0));

// > cocos2d::CCEGLView::initWithRect @ 0x180077450
// > cocos2d::CCEGLView::updateFrameSize @ 0x1800790e9
// > cocos2d::CCEGLView::updateFrameSize @ 0x180079107
// > cocos2d::CCEGLView::updateWindow @ 0x18007923e
static auto glfwGetWindowSize = reinterpret_cast<decltype(&::glfwGetWindowSize)>(reinterpret_cast<uintptr_t>(geode::base::getCocos() + 0xd4a20));

}
