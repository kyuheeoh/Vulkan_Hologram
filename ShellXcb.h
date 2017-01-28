
#pragma once

#ifndef SHELL_XCB_H
#define SHELL_XCB_H

#include <xcb/xcb.h>
#include "Shell.h"

class ShellXcb : public Shell {
public:
    ShellXcb(Application &app);
    ~ShellXcb();

    void run();
    void quit() { quit_ = true; }

private:
    void init_connection();

    PFN_vkGetInstanceProcAddr load_vk();
    bool can_present(VkPhysicalDevice phy, uint32_t queue_family);

    void create_window();
    VkSurfaceKHR create_surface(VkInstance instance);

    void handle_event(const xcb_generic_event_t *ev);
    void loop_wait();
    void loop_poll();

    xcb_connection_t *c_;                       // connection 도 두개
    xcb_screen_t *scr_;                         // 스크린도 두개
    xcb_window_t win_;                          // 윈도우도 두개

    xcb_atom_t wm_protocols_;
    xcb_atom_t wm_delete_window_;

    void *lib_handle_;

    bool quit_;
};

#endif // SHELL_XCB_H
