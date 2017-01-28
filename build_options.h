#ifndef BUILD_OPTIONS_H
    #define BUILD_OPTIONS_H


    #ifdef WIN32
    #define VK_USE_PLATFORM_WIN32_KHR
    #endif

    #ifdef __linux__
    #define VK_USE_PLATFORM_XCB_KHR 1
    #endif /* linux */



#endif // BUILD_OPTIONS_H
