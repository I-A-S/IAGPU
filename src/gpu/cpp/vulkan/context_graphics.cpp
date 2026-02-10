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
#if !IAGPU_DISABLE_GRAPHICS
  auto Context::initialize_swapchain(u32 width, u32 height) -> Result<void>
  {
    Mut<VkSurfaceFormatKHR> selected_surface_format;
    Mut<Vec<VkSurfaceFormatKHR>> surface_formats;
    VK_ENUM_CALL(vkGetPhysicalDeviceSurfaceFormatsKHR, surface_formats, m_device.get_physical_hande(), m_surface);
    for (const auto &format : surface_formats)
    {
      selected_surface_format = format;
      if (format.format == VK_FORMAT_B8G8R8A8_SRGB)
        break;
    }
    m_swapchain_format = selected_surface_format.format;
    m_swapchain_colorspace = selected_surface_format.colorSpace;

    Mut<VkSurfaceCapabilitiesKHR> surface_capabilities;
    VK_CALL(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_device.get_physical_hande(), m_surface, &surface_capabilities),
            "Fetching surface capabilities");
    m_swapchain_buffer_count = std::max(MAX_PENDING_FRAME_COUNT, surface_capabilities.minImageCount);
    if (surface_capabilities.maxImageCount > 0)
      m_swapchain_buffer_count = std::min(m_swapchain_buffer_count, (i32) surface_capabilities.maxImageCount);
    m_swapchain_min_possible_extent = surface_capabilities.minImageExtent;
    m_swapchain_max_possible_extent = surface_capabilities.maxImageExtent;

    const VkFenceCreateInfo fence_create_info{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    const VkCommandPoolCreateInfo command_pool_create_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = m_device.get_graphics_queue_family(),
    };

    for (Mut<i32> i = 0; i < m_swapchain_buffer_count; i++)
    {
      VK_CALL(vkCreateFence(m_device.get_handle(), &fence_create_info, nullptr, &m_frames[i].in_flight_fence),
              "Creating swapchain inflight fence");
      VK_CALL(vkCreateCommandPool(m_device.get_handle(), &command_pool_create_info, nullptr, &m_frames[i].command_pool),
              "Creating swapchain command pool");
      m_frames[i].render_target_texture = new TextureImpl();
    }

    m_swapchain = VK_NULL_HANDLE;

    AU_TRY_PURE(resize_swapchain(width, height));

    return {};
  }

  auto Context::destroy_swapchain() -> void
  {
    for (Mut<i32> i = 0; i < m_swapchain_buffer_count; i++)
    {
      m_frames[i].cmd_list_cache.clear();
      m_frames[i].used_cmd_list_count = 0;
      delete m_frames[i].render_target_texture;
      vkDestroyFence(m_device.get_handle(), m_frames[i].in_flight_fence, nullptr);
      vkDestroyImageView(m_device.get_handle(), m_frames[i].swapchain_image_view, nullptr);
      vkDestroyCommandPool(m_device.get_handle(), m_frames[i].command_pool, nullptr);
      vkDestroySemaphore(m_device.get_handle(), m_frames[i].image_available_semaphore, nullptr);
      vkDestroySemaphore(m_device.get_handle(), m_frames[i].render_finished_semaphore, nullptr);
    }

    vkDestroySwapchainKHR(m_device.get_handle(), m_swapchain, nullptr);
  }
#endif

  Result<void> Context::resize_swapchain(u32 width, u32 height)
  {
#if !IAGPU_DISABLE_GRAPHICS
    VK_CALL(vkDeviceWaitIdle(m_device.get_handle()), "Waiting device idle");

    if (m_swapchain != VK_NULL_HANDLE)
    {
      for (Mut<i32> i = 0; i < m_swapchain_buffer_count; i++)
        vkDestroyImageView(m_device.get_handle(), m_frames[i].swapchain_image_view, nullptr);
    }

    const auto graphics_queue_family = m_device.get_graphics_queue_family();

    m_swapchain_extent.width =
        std::min(std::max(width, m_swapchain_min_possible_extent.width), m_swapchain_max_possible_extent.width);
    m_swapchain_extent.height =
        std::min(std::max(height, m_swapchain_min_possible_extent.height), m_swapchain_max_possible_extent.height);

    Mut<VkSwapchainCreateInfoKHR> create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.oldSwapchain = m_swapchain;
    create_info.surface = m_surface;
    create_info.imageFormat = m_swapchain_format;
    create_info.imageColorSpace = m_swapchain_colorspace;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    create_info.queueFamilyIndexCount = 1;
    create_info.pQueueFamilyIndices = &graphics_queue_family;
    create_info.minImageCount = m_swapchain_buffer_count;
    create_info.imageExtent = m_swapchain_extent;
    create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    VK_CALL(vkCreateSwapchainKHR(m_device.get_handle(), &create_info, nullptr, &m_swapchain), "Creating swapchain");
    vkDestroySwapchainKHR(m_device.get_handle(), create_info.oldSwapchain, nullptr);

    Mut<Vec<VkImage>> swapchain_images;
    VK_ENUM_CALL(vkGetSwapchainImagesKHR, swapchain_images, m_device.get_handle(), m_swapchain);
    Mut<i32> frame_index{0};
    for (const auto &img : swapchain_images)
    {
      Mut<VkImageView> view{};

      Mut<VkImageViewCreateInfo> view_create_info{};
      view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      view_create_info.image = img;
      view_create_info.format = m_swapchain_format;
      view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
      view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      view_create_info.subresourceRange.baseArrayLayer = 0;
      view_create_info.subresourceRange.baseMipLevel = 0;
      view_create_info.subresourceRange.layerCount = 1;
      view_create_info.subresourceRange.levelCount = 1;
      VK_CALL(vkCreateImageView(m_device.get_handle(), &view_create_info, nullptr, &view),
              "Creating swapchain image view");

      m_frames[frame_index].swapchain_image = img;
      m_frames[frame_index].swapchain_image_view = view;
      *m_frames[frame_index++].render_target_texture = TextureImpl(img, view, m_swapchain_extent);
    }

    const VkSemaphoreCreateInfo semaphore_create_info{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    for (Mut<i32> i = 0; i < m_swapchain_buffer_count; i++)
    {
      vkDestroySemaphore(m_device.get_handle(), m_frames[i].image_available_semaphore, nullptr);
      vkDestroySemaphore(m_device.get_handle(), m_frames[i].render_finished_semaphore, nullptr);

      VK_CALL(vkCreateSemaphore(m_device.get_handle(), &semaphore_create_info, nullptr,
                                &m_frames[i].render_finished_semaphore),
              "Creating swapchain render finished semaphore");
      VK_CALL(vkCreateSemaphore(m_device.get_handle(), &semaphore_create_info, nullptr,
                                &m_frames[i].image_available_semaphore),
              "Creating swapchain semaphore");
    }

    GPU_LOG_INFO("Recreated swapchain ({}x{}x{})", m_swapchain_extent.width, m_swapchain_extent.height,
                 create_info.minImageCount);

    return {};
#else
    return fail("ResizeSwapchain must not be called when IAGPU_DISABLE_GRAPHICS is TRUE");
#endif
  }
} // namespace ia::gpu::vulkan