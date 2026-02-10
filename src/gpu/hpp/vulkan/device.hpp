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

#pragma once

#include <vulkan/base.hpp>

namespace ia::gpu::vulkan
{
  class Device
  {
public:
    auto boot(VkInstance instance, VkSurfaceKHR surface, Span<const char *> extensions) -> Result<void>;
    auto shutdown() -> void;

    auto wait_idle() -> void;

public:
    [[nodiscard]] auto get_handle() const -> VkDevice
    {
      return m_device;
    }

    [[nodiscard]] auto get_physical_hande() const -> VkPhysicalDevice
    {
      return m_physical_device;
    }

    [[nodiscard]] auto get_allocator() const -> VmaAllocator
    {
      return m_allocator;
    }

    [[nodiscard]] auto get_descriptor_pool() const -> VkDescriptorPool
    {
      return m_descriptor_pool;
    }

    [[nodiscard]] auto get_graphics_queue() const -> VkQueue
    {
      return m_graphics_queue;
    }

    [[nodiscard]] auto get_compute_queue() const -> VkQueue
    {
      return m_compute_queue;
    }

    [[nodiscard]] auto get_transfer_queue() const -> VkQueue
    {
      return m_transfer_queue;
    }

    [[nodiscard]] auto get_graphics_queue_family() const -> u32
    {
      return m_graphics_queue_family;
    }

    [[nodiscard]] auto get_compute_queue_family() const -> u32
    {
      return m_compute_queue_family;
    }

    [[nodiscard]] auto get_transfer_queue_family() const -> u32
    {
      return m_transfer_queue_family;
    }

private:
    auto initialize_device(VkInstance instance, Span<const char *> extensions) -> Result<void>;

    auto select_physical_device(VkInstance instance) -> Result<VkPhysicalDevice>;

private:
    VkDevice m_device{};
    VkPhysicalDevice m_physical_device{};

    VkQueue m_compute_queue{};
    VkQueue m_graphics_queue{};
    VkQueue m_transfer_queue{};
    u32 m_graphics_queue_family{UINT32_MAX};
    u32 m_compute_queue_family{UINT32_MAX};
    u32 m_transfer_queue_family{UINT32_MAX};

    VkFence m_command_submit_fence;

    VmaAllocator m_allocator{};

    VkSurfaceKHR m_surface{};

    VkDescriptorPool m_descriptor_pool{VK_NULL_HANDLE};

public:
    Device() = default;
    ~Device() = default;
  };
} // namespace ia::gpu::vulkan