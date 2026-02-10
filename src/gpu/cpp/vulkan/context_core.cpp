// IAGPU: IA GPU Hardware Interface.
// Copyright (C) 2026 IAS (ias@iasoft.dev)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <vulkan/context.hpp>

namespace ia::gpu::vulkan
{

}

namespace ia::gpu::vulkan
{
  static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                                       VkDebugUtilsMessageTypeFlagsEXT message_type,
                                                       const VkDebugUtilsMessengerCallbackDataEXT *p_callback_data,
                                                       void *p_user_data)
  {
    AU_UNUSED(p_user_data);
    AU_UNUSED(message_type);

    if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
      GPU_LOG_ERROR("[Validation]: {}", p_callback_data->pMessage);
    else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
      GPU_LOG_WARN("[Validation]: {}", p_callback_data->pMessage);
    else
      GPU_LOG_TRACE("[Validation]: {}", p_callback_data->pMessage);

    return VK_FALSE;
  }

  auto Context::create(Ref<ContextConfig> config) -> Result<Context>
  {
    Mut<Context> result(config);
    AU_TRY_PURE(result.initialize_instance(config.validation_enabled));

    Mut<VkSurfaceKHR> surface{};

#if !IAGPU_DISABLE_GRAPHICS
    if (!config.surface_creation_callback)
      return fail("surface_creation_callback must not be NULL when IAGPU_DISABLE_GRAPHICS is FALSE");
    assert(surface = (VkSurfaceKHR) config.surface_creation_callback(result.m_instance,
                                                                     config.surface_creation_callback_user_data));
    result.m_surface = surface;
#endif

    AU_TRY_PURE(result.m_device.boot(result.m_instance, surface, result.m_device_extensions));

#if !IAGPU_DISABLE_GRAPHICS
    const auto intial_width = 800;
    const auto intial_height = 600;
    AU_TRY_PURE(result.initialize_swapchain(intial_width, intial_height));
#else
    result.m_swapchain_buffer_count = MAX_PENDING_FRAME_COUNT;
    const VkFenceCreateInfo fence_create_info{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };
    const VkCommandPoolCreateInfo command_pool_create_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = result.m_device.get_compute_queue_family(),
    };
    for (u32 i = 0; i < MAX_PENDING_FRAME_COUNT; i++)
    {
      VK_CALL(
          vkCreateFence(result.m_device.get_handle(), &fence_create_info, nullptr, &result.m_frames[i].in_flight_fence),
          "Creating inflight fence");
      VK_CALL(vkCreateCommandPool(result.m_device.get_handle(), &command_pool_create_info, nullptr,
                                  &result.m_frames[i].command_pool),
              "Creating command pool");
    }
#endif

    {
      Mut<VkCommandPoolCreateInfo> command_pool_create_info{};
      command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

#if !IAGPU_DISABLE_GRAPHICS
      command_pool_create_info.queueFamilyIndex = result.m_device.get_graphics_queue_family();
#else
      command_pool_create_info.queueFamilyIndex = result.m_device.get_compute_queue_family();
#endif

      command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
      VK_CALL(vkCreateCommandPool(result.m_device.get_handle(), &command_pool_create_info, nullptr,
                                  &result.m_transient_command_pool),
              "Creating immediate command pool");
    }

    // [IATODO]
    //{
    //  const SamplerDesc desc{
    //      .linear_filter = true,
    //      .repeat_uv = true,
    //      .debug_name = "Default Sampler",
    //  };
    //  Mut<Vec<Sampler>> out_samplers = {result.m_default_sampler};
    //  if (!result.create_samplers(std::initializer_list<const SamplerDesc>{desc}, out_samplers))
    //    return fail("Failed to create default sampler");
    //}

    return std::move(result);
  }

  Context::Context(Ref<ContextConfig> config) : m_config(config)
  {
  }

  Context::~Context()
  {
    // [IATODO]
    // #if !IAGPU_DISABLE_GRAPHICS
    //     destroy_swapchain();
    //     vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    // #else
    //     for (UINT32 i = 0; i < MAX_PENDING_FRAME_COUNT; i++)
    //     {
    //       for (auto &t : m_frames[i].cmdListCache)
    //         delete t;
    //       m_frames[i].cmdListCache.clear();
    //       m_frames[i].usedCmdListCount = 0;
    //       vkDestroyFence(m_device.GetHandle(), m_frames[i].inFlightFence, nullptr);
    //       vkDestroyCommandPool(m_device.GetHandle(), m_frames[i].commandPool, nullptr);
    //     }
    // #endif
    //
    //     destroy_buffers(1, &m_staging_buffer_handle);
    //
    //     vkDestroyCommandPool(m_device.get_handle(), m_transient_command_pool, nullptr);
    //
    //     destroy_samplers(1, &m_default_sampler);

    destroy_instance();
  }

  auto Context::initialize_instance(bool enable_validation) -> Result<void>
  {
    {
      m_instance_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

#if IA_PLATFORM_WINDOWS
      m_instance_extensions.push_back("VK_KHR_win32_surface");
#elif IA_PLATFORM_ANDROID
      m_instance_extensions.push_back("VK_KHR_android_surface");
#elif IA_PLATFORM_LINUX
      m_instance_extensions.push_back("VK_KHR_xcb_surface");
      m_instance_extensions.push_back("VK_KHR_xlib_surface");
#endif
      m_device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }

    VK_CALL(volkInitialize(), "Initializing Vulkan loader");

    Mut<u32> instance_version{};
    VK_CALL(vkEnumerateInstanceVersion(&instance_version), "Enumerating Vulkan version");

    const VkApplicationInfo application_info{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "IAGPU",
        .applicationVersion = 1,
        .pEngineName = "IAGPU",
        .engineVersion = 1,
        .apiVersion = VULKAN_API_VERSION,
    };

    static const char *const validation_layer = "VK_LAYER_KHRONOS_validation";
    Mut<bool> validation_found = false;

    if (enable_validation)
    {
      Mut<u32> layer_count;
      vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
      Mut<Vec<VkLayerProperties>> available_layers(layer_count);
      vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

      for (const auto &layer : available_layers)
      {
        if (strcmp(validation_layer, layer.layerName) == 0)
        {
          validation_found = true;
          break;
        }
      }

      if (!validation_found)
      {
        GPU_LOG_WARN("Validation layer '{}' not found. Debugging will be disabled.", validation_layer);
        enable_validation = false;
      }
    }

    Mut<VkInstanceCreateInfo> instance_create_info{};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.enabledLayerCount = 0;

    Mut<VkDebugUtilsMessengerCreateInfoEXT> debug_create_info{};

    if (enable_validation)
    {
      instance_create_info.enabledLayerCount = 1;
      instance_create_info.ppEnabledLayerNames = &validation_layer;

      debug_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
      debug_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
      debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
      debug_create_info.pfnUserCallback = debug_callback;

      instance_create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT *) &debug_create_info;

      m_instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    instance_create_info.enabledExtensionCount = m_instance_extensions.size();
    instance_create_info.ppEnabledExtensionNames = m_instance_extensions.data();
    instance_create_info.pApplicationInfo = &application_info;

    if (instance_version < application_info.apiVersion)
      return fail(std::format("IAGHI requires graphics hardware that supports at least Vulkan API version ",
                              VULKAN_API_VERSION));

    VK_CALL(vkCreateInstance(&instance_create_info, nullptr, &m_instance), "Creating Vulkan instance");
    volkLoadInstance(m_instance);

    if (enable_validation)
    {
      if (vkCreateDebugUtilsMessengerEXT(m_instance, &debug_create_info, nullptr, &m_debug_messenger) != VK_SUCCESS)
        GPU_LOG_WARN("Failed to set up debug messenger");
    }

    return {};
  }

  auto Context::destroy_instance() -> void
  {
    if (m_debug_messenger != VK_NULL_HANDLE)
      vkDestroyDebugUtilsMessengerEXT(m_instance, m_debug_messenger, nullptr);

    vkDestroyInstance(m_instance, nullptr);
  }

  auto Context::begin_compute_only_frame() -> void
  {
  }

  auto Context::end_compute_only_frame(MutRef<CmdListType> cmd) -> bool
  {
  }

  auto Context::begin_graphics_frame() -> void
  {
  }

  auto Context::end_graphics_frame(MutRef<CmdListType> cmd) -> bool
  {
  }

  auto Context::advance_current_frame() -> MutRef<CmdListType>
  {
  }

  auto Context::prepare_staging_memory(u64 size) -> Result<void *>
  {
  }
} // namespace ia::gpu::vulkan