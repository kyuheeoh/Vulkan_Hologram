#pragma once
#include <cassert>
#include <array>
#include <iostream>
#include <string>
#include <sstream>
#include <set>
#include "Helpers.h"
#include "Shell.h"
#include "vkapplication.h"


int Shell::CreateDebugReportCallbackEXT(VkInstance                          instance,
                                  const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                  const VkAllocationCallbacks*              pAllocator,
                                  VkDebugReportCallbackEXT*                 pCallback)
{
    auto func = (PFN_vkCreateDebugReportCallbackEXT)
            vkGetInstanceProcAddr(instance,
                                  "vkCreateDebugReportCallbackEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pCallback);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void Shell::DestroyDebugReportCallbackEXT (VkInstance               instance,
                                    VkDebugReportCallbackEXT        callback,
                                    const VkAllocationCallbacks*    pAllocator)
{
    auto func = (PFN_vkDestroyDebugReportCallbackEXT)
            vkGetInstanceProcAddr(instance,
                                  "vkDestroyDebugReportCallbackEXT");
    if (func != nullptr)
    {
        func(instance, callback, pAllocator);
    }
}


Shell::Shell(Application &application)
    : application_(application), settings_(application.settings()), ctx_(),
      app_tick_(1.0f / settings_.ticks_per_second), app_time_(app_tick_)
{
    // require generic WSI extensions
    instance_extensions_.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    device_extensions_.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    // require "standard" validation layers
    if (settings_.validate) {
        instance_layers_.push_back("VK_LAYER_LUNARG_standard_validation");
        instance_extensions_.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }
}

void Shell::log(LogPriority priority, const char *msg) const
{
    std::ostream &st = (priority >= LOG_ERR) ? std::cerr : std::cout;
    st << msg << "\n";
}

void Shell::init_vk()
{
    //init_dispatch_table_top(load_vk());

    init_instance();
    //init_dispatch_table_middle(ctx_.instance, false);

    init_debug_report();
    init_physical_dev();
}

void Shell::cleanup_vk()
{
/////////////////////////////////////////////////////////////////////////////////////////////////
    if (settings_.validate)
        DestroyDebugReportCallbackEXT(ctx_.instance, ctx_.debug_report, nullptr);
/////////////////////////////////////////////////////////////////////////////////////////////////
    vkDestroyInstance(ctx_.instance, nullptr);
}

bool Shell::debug_report_callback(VkDebugReportFlagsEXT flags,
                                  VkDebugReportObjectTypeEXT obj_type,
                                  uint64_t object,
                                  size_t location,
                                  int32_t msg_code,
                                  const char *layer_prefix,
                                  const char *msg)
{
    LogPriority prio = LOG_WARN;
    if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
        prio = LOG_ERR;
    else if (flags & (VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT))
        prio = LOG_WARN;
    else if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
        prio = LOG_INFO;
    else if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
        prio = LOG_DEBUG;

    std::stringstream ss;
    ss << layer_prefix << ": " << msg;

    log(prio, ss.str().c_str());

    return false;
}

void Shell::assert_all_instance_layers() const
{
    // enumerate instance layer
    std::vector<VkLayerProperties> layers;
    enumerate(layers);

    std::set<std::string> layer_names;
    for (const auto &layer : layers)
        layer_names.insert(layer.layerName);

    // all listed instance layers are required
    for (const auto &name : instance_layers_) {
        if (layer_names.find(name) == layer_names.end()) {
            std::stringstream ss;
            ss << "instance layer " << name << " is missing";
            throw std::runtime_error(ss.str());
        }
    }
}

void Shell::assert_all_instance_extensions() const
{
    // enumerate instance extensions
    std::vector<VkExtensionProperties> exts;
    enumerate(nullptr, exts);

    std::set<std::string> ext_names;
    for (const auto &ext : exts)
        ext_names.insert(ext.extensionName);

    for (auto &layer : instance_layers_) {
        enumerate(layer, exts);
        for (const auto &ext : exts)
            ext_names.insert(ext.extensionName);
    }

    // all listed instance extensions are required
    for (const auto &name : instance_extensions_) {
        if (ext_names.find(name) == ext_names.end()) {
            std::stringstream ss;
            ss << "instance extension " << name << " is missing";
            throw std::runtime_error(ss.str());
        }
    }
}

bool Shell::has_all_device_extensions(VkPhysicalDevice phy) const
{
    // enumerate device extensions
    std::vector<VkExtensionProperties> exts;
    enumerate(phy, nullptr, exts);

    std::set<std::string> ext_names;
    for (const auto &ext : exts)
        ext_names.insert(ext.extensionName);

    // all listed device extensions are required
    for (const auto &name : device_extensions_) {
        if (ext_names.find(name) == ext_names.end())
            return false;
    }

    return true;
}

void Shell::init_instance()
{
    assert_all_instance_layers();
    assert_all_instance_extensions();

    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = settings_.name.c_str();
    app_info.applicationVersion = VK_MAKE_VERSION (0,0,1);
    app_info.apiVersion = VK_MAKE_VERSION (1,0,2);

    VkInstanceCreateInfo instance_info = {};
    instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_info.pApplicationInfo = &app_info;
    instance_info.enabledLayerCount = static_cast<uint32_t>(instance_layers_.size());
    instance_info.ppEnabledLayerNames = instance_layers_.data();
    instance_info.enabledExtensionCount = static_cast<uint32_t>(instance_extensions_.size());
    instance_info.ppEnabledExtensionNames = instance_extensions_.data();

    assert_success(vkCreateInstance(&instance_info, nullptr, &ctx_.instance));
}

void Shell::init_debug_report()
{
    if (!settings_.validate)
        return;

    VkDebugReportCallbackCreateInfoEXT debug_report_info = {};
    debug_report_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;

    debug_report_info.flags = VK_DEBUG_REPORT_WARNING_BIT_EXT |
                              VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
                              VK_DEBUG_REPORT_ERROR_BIT_EXT;
    if (settings_.validate_verbose) {
        debug_report_info.flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
                                  VK_DEBUG_REPORT_DEBUG_BIT_EXT;
    }

    debug_report_info.pfnCallback = debug_report_callback;
    debug_report_info.pUserData = reinterpret_cast<void *>(this);
///////////////////////////////////////////////////////////////////////////////////////////////////////
    int k =(CreateDebugReportCallbackEXT(ctx_.instance,
                &debug_report_info, nullptr, &ctx_.debug_report));
    if (k<0)
    {
        std::cerr << "Failed to Create DebugReportCallBack Extension\n";
        std::exit(-1);
    }
////////////////////////////////////////////////////////////////////////////////////////////////////////
}

void Shell::init_physical_dev()
{
    // enumerate physical devices
    std::vector<VkPhysicalDevice> phys;
    assert_success(enumerate(ctx_.instance, phys));

    ctx_.physical_dev = VK_NULL_HANDLE;
    for (auto phy : phys) {
        if (!has_all_device_extensions(phy))
            continue;

        // get queue properties
        std::vector<VkQueueFamilyProperties> queues;
        get(phy, queues);

        int game_queue_family = -1, present_queue_family = -1;
        for (uint32_t i = 0; i < queues.size(); i++) {
            const VkQueueFamilyProperties &q = queues[i];

            // requires only GRAPHICS for game queues
            const VkFlags game_queue_flags = VK_QUEUE_GRAPHICS_BIT;
            if (game_queue_family < 0 &&
                (q.queueFlags & game_queue_flags) == game_queue_flags)
                game_queue_family = i;

            // present queue must support the surface
            if (present_queue_family < 0 && can_present(phy, i))
                present_queue_family = i;

            if (game_queue_family >= 0 && present_queue_family >= 0)
                break;
        }

        if (game_queue_family >= 0 && present_queue_family >= 0) {
            ctx_.physical_dev = phy;
            ctx_.game_queue_family = game_queue_family;
            ctx_.present_queue_family = present_queue_family;
            break;
        }
    }

    if (ctx_.physical_dev == VK_NULL_HANDLE)
        throw std::runtime_error("failed to find any capable Vulkan physical device");
}

void Shell::create_context()
{
    create_dev();
    //init_dispatch_table_bottom(ctx_.instance, ctx_.dev);

    vkGetDeviceQueue(ctx_.dev, ctx_.game_queue_family, 0, &ctx_.game_queue);
    vkGetDeviceQueue(ctx_.dev, ctx_.present_queue_family, 0, &ctx_.present_queue);

    create_back_buffers();

    // initialize ctx_.{surface,format} before attach_shell
    create_swapchain();

    application_.attach_shell(*this);
}

void Shell::destroy_context()
{
    if (ctx_.dev == VK_NULL_HANDLE)
        return;

    vkDeviceWaitIdle(ctx_.dev);

    destroy_swapchain();

    application_.detach_shell();

    destroy_back_buffers();

    ctx_.game_queue = VK_NULL_HANDLE;
    ctx_.present_queue = VK_NULL_HANDLE;

    vkDeviceWaitIdle(ctx_.dev);
    vkDestroyDevice(ctx_.dev, nullptr);
    ctx_.dev = VK_NULL_HANDLE;
}

void Shell::create_dev()
{
    VkDeviceCreateInfo dev_info = {};
    dev_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    const std::vector<float> queue_priorities(settings_.queue_count, 0.0f);
    std::array<VkDeviceQueueCreateInfo, 2> queue_info = {};
    queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info[0].queueFamilyIndex = ctx_.game_queue_family;
    queue_info[0].queueCount = settings_.queue_count;
    queue_info[0].pQueuePriorities = queue_priorities.data();

    if (ctx_.game_queue_family != ctx_.present_queue_family) {
        queue_info[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info[1].queueFamilyIndex = ctx_.present_queue_family;
        queue_info[1].queueCount = 1;
        queue_info[1].pQueuePriorities = queue_priorities.data();

        dev_info.queueCreateInfoCount = 2;
    } else {
        dev_info.queueCreateInfoCount = 1;
    }

    dev_info.pQueueCreateInfos = queue_info.data();
    dev_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions_.size());
    dev_info.ppEnabledExtensionNames = device_extensions_.data();

    // disable all features
    VkPhysicalDeviceFeatures features = {};
    dev_info.pEnabledFeatures = &features;

    assert_success(vkCreateDevice(ctx_.physical_dev, &dev_info, nullptr, &ctx_.dev));
}

void Shell::create_back_buffers()
{
    VkSemaphoreCreateInfo sem_info = {};
    sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    // BackBuffer is used to track which swapchain image and its associated
    // sync primitives are busy.  Having more BackBuffer's than swapchain
    // images may allows us to replace CPU wait on present_fence by GPU wait
    // on acquire_semaphore.
    const int count = settings_.back_buffer_count + 1;
    for (int i = 0; i < count; i++) {
        BackBuffer buf = {};
        assert_success(vkCreateSemaphore(ctx_.dev, &sem_info, nullptr, &buf.acquire_semaphore));
        assert_success(vkCreateSemaphore(ctx_.dev, &sem_info, nullptr, &buf.render_semaphore));
        assert_success(vkCreateFence(ctx_.dev, &fence_info, nullptr, &buf.present_fence));

        ctx_.back_buffers.push(buf);
    }
}

void Shell::destroy_back_buffers()
{
    while (!ctx_.back_buffers.empty()) {
        const auto &buf = ctx_.back_buffers.front();

        vkDestroySemaphore(ctx_.dev, buf.acquire_semaphore, nullptr);
        vkDestroySemaphore(ctx_.dev, buf.render_semaphore, nullptr);
        vkDestroyFence(ctx_.dev, buf.present_fence, nullptr);

        ctx_.back_buffers.pop();
    }
}

void Shell::create_swapchain()
{
    ctx_.surface = create_surface(ctx_.instance);

    VkBool32 supported;
    assert_success(vkGetPhysicalDeviceSurfaceSupportKHR(ctx_.physical_dev,
                ctx_.present_queue_family, ctx_.surface, &supported));
    // this should be guaranteed by the platform-specific can_present call
    assert(supported);

    std::vector<VkSurfaceFormatKHR> formats;
    get(ctx_.physical_dev, ctx_.surface, formats);
    ctx_.format = formats[0];

    // defer to resize_swapchain()
    ctx_.swapchain = VK_NULL_HANDLE;
    ctx_.extent.width = (uint32_t) -1;
    ctx_.extent.height = (uint32_t) -1;
}

void Shell::destroy_swapchain()
{
    if (ctx_.swapchain != VK_NULL_HANDLE) {
        application_.detach_swapchain();

        vkDestroySwapchainKHR(ctx_.dev, ctx_.swapchain, nullptr);
        ctx_.swapchain = VK_NULL_HANDLE;
    }

    vkDestroySurfaceKHR(ctx_.instance, ctx_.surface, nullptr);
    ctx_.surface = VK_NULL_HANDLE;
}

void Shell::resize_swapchain(uint32_t width_hint, uint32_t height_hint)
{
    VkSurfaceCapabilitiesKHR caps;
    assert_success(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ctx_.physical_dev,
                ctx_.surface, &caps));

    VkExtent2D extent = caps.currentExtent;
    // use the hints
    if (extent.width == (uint32_t) -1) {
        extent.width = width_hint;
        extent.height = height_hint;
    }
    // clamp width; to protect us from broken hints?
    if (extent.width < caps.minImageExtent.width)
        extent.width = caps.minImageExtent.width;
    else if (extent.width > caps.maxImageExtent.width)
        extent.width = caps.maxImageExtent.width;
    // clamp height
    if (extent.height < caps.minImageExtent.height)
        extent.height = caps.minImageExtent.height;
    else if (extent.height > caps.maxImageExtent.height)
        extent.height = caps.maxImageExtent.height;

    if (ctx_.extent.width == extent.width && ctx_.extent.height == extent.height)
        return;

    uint32_t image_count = settings_.back_buffer_count;
    if (image_count < caps.minImageCount)
        image_count = caps.minImageCount;
    else if (image_count > caps.maxImageCount)
        image_count = caps.maxImageCount;

    assert(caps.supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    assert(caps.supportedTransforms & caps.currentTransform);
    assert(caps.supportedCompositeAlpha & (VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR |
                                           VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR));
    VkCompositeAlphaFlagBitsKHR composite_alpha =
        (caps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) ?
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR : VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    std::vector<VkPresentModeKHR> modes;
    get(ctx_.physical_dev, ctx_.surface, modes);

    // FIFO is the only mode universally supported
    VkPresentModeKHR mode = VK_PRESENT_MODE_FIFO_KHR;
    for (auto m : modes) {
        if ((settings_.vsync && m == VK_PRESENT_MODE_MAILBOX_KHR) ||
            (!settings_.vsync && m == VK_PRESENT_MODE_IMMEDIATE_KHR)) {
            mode = m;
            break;
        }
    }

    VkSwapchainCreateInfoKHR swapchain_info = {};
    swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_info.surface = ctx_.surface;
    swapchain_info.minImageCount = image_count;
    swapchain_info.imageFormat = ctx_.format.format;
    swapchain_info.imageColorSpace = ctx_.format.colorSpace;
    swapchain_info.imageExtent = extent;
    swapchain_info.imageArrayLayers = 1;
    swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    std::vector<uint32_t> queue_families(1, ctx_.game_queue_family);
    if (ctx_.game_queue_family != ctx_.present_queue_family) {
        queue_families.push_back(ctx_.present_queue_family);

        swapchain_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_info.queueFamilyIndexCount = (uint32_t)queue_families.size();
        swapchain_info.pQueueFamilyIndices = queue_families.data();
    } else {
        swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    swapchain_info.preTransform = caps.currentTransform;;
    swapchain_info.compositeAlpha = composite_alpha;
    swapchain_info.presentMode = mode;
    swapchain_info.clipped = true;
    swapchain_info.oldSwapchain = ctx_.swapchain;

    assert_success(vkCreateSwapchainKHR(ctx_.dev, &swapchain_info, nullptr, &ctx_.swapchain));
    ctx_.extent = extent;

    // destroy the old swapchain
    if (swapchain_info.oldSwapchain != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(ctx_.dev);

        application_.detach_swapchain();
        vkDestroySwapchainKHR(ctx_.dev, swapchain_info.oldSwapchain, nullptr);
    }

    application_.attach_swapchain();
}

void Shell::add_app_time(float time)
{
    int max_ticks = 3;

    if (!settings_.no_tick)
        app_time_ += time;

    while (app_time_ >= app_tick_ && max_ticks--) {
        application_.on_tick();
        app_time_ -= app_tick_;
    }
}

void Shell::acquire_back_buffer()
{
    // acquire just once when not presenting
    if (settings_.no_present &&
        ctx_.acquired_back_buffer.acquire_semaphore != VK_NULL_HANDLE)
        return;

    auto &buf = ctx_.back_buffers.front();

    // wait until acquire and render semaphores are waited/unsignaled
    assert_success(vkWaitForFences(ctx_.dev, 1, &buf.present_fence,
                true, UINT64_MAX));
    // reset the fence
    assert_success(vkResetFences(ctx_.dev, 1, &buf.present_fence));

    assert_success(vkAcquireNextImageKHR(ctx_.dev, ctx_.swapchain,
                UINT64_MAX, buf.acquire_semaphore, VK_NULL_HANDLE,
                &buf.image_index));

    ctx_.acquired_back_buffer = buf;
    ctx_.back_buffers.pop();
}

void Shell::present_back_buffer()
{
    const auto &buf = ctx_.acquired_back_buffer;

    if (!settings_.no_render)
        application_.on_frame(app_time_ / app_tick_);

    if (settings_.no_present) {
        fake_present();
        return;
    }

    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = (settings_.no_render) ?
        &buf.acquire_semaphore : &buf.render_semaphore;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &ctx_.swapchain;
    present_info.pImageIndices = &buf.image_index;

    assert_success(vkQueuePresentKHR(ctx_.present_queue, &present_info));

    assert_success(vkQueueSubmit(ctx_.present_queue, 0, nullptr, buf.present_fence));
    ctx_.back_buffers.push(buf);
}

void Shell::fake_present()
{
    const auto &buf = ctx_.acquired_back_buffer;

    assert(settings_.no_present);

    // wait render semaphore and signal acquire semaphore
    if (!settings_.no_render) {
        VkPipelineStageFlags stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &buf.render_semaphore;
        submit_info.pWaitDstStageMask = &stage;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &buf.acquire_semaphore;
        assert_success(vkQueueSubmit(ctx_.game_queue, 1, &submit_info, VK_NULL_HANDLE));
    }

    // push the buffer back just once for Shell::cleanup_vk
    if (buf.acquire_semaphore != ctx_.back_buffers.back().acquire_semaphore)
        ctx_.back_buffers.push(buf);
}
