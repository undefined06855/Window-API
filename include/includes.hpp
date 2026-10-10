#pragma once
// TODO: see if these are okay in public header files

#undef _glfw3_native_h_
#undef _glfw3_h_
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#undef __CC_EGLVIEW_WIN32_H__
#include <Geode/cocos/platform/win32/CCEGLView.h>

#undef __CC_IME_DISPATCHER_H__
#define protected public
#include <Geode/cocos/text_input_node/CCIMEDispatcher.h>
#undef protected

#ifdef GEODE_IS_WINDOWS
    #ifdef WINDOW_API_EXPORTING
        #define WINDOW_API_DLL __declspec(dllexport)
    #else
        #define WINDOW_API_DLL __declspec(dllimport)
    #endif
#else
    #define WINDOW_API_DLL __attribute__((visibility("default")))
#endif
