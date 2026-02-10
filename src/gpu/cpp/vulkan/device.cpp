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

#include <vulkan/device.hpp>

namespace ia::gpu::vulkan
{
  auto Device::boot(VkInstance instance, VkSurfaceKHR surface, Span<const char *> extensions) -> Result<void>
  {
    m_surface = surface;

    AU_TRY_PURE(initialize_device(instance, extensions));

    const VkDescriptorPoolSize pool_sizes[] = {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024},
                                               {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1024},
                                               {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4096},
                                               {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1024}};
    const VkDescriptorPoolCreateInfo pool_info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = 1000,
        .poolSizeCount = 4,
        .pPoolSizes = pool_sizes,
    };
    VK_CALL(vkCreateDescriptorPool(m_device, &pool_info, nullptr, &m_descriptor_pool),
            "Failed to create descriptor pool");

    return {};
  }

  auto Device::shutdown() -> void
  {
  }

  auto Device::wait_idle() -> void
  {
  }

  auto Device::initialize_device(VkInstance instance, Span<const char *> extensions) -> Result<void>
  {
  }

  auto Device::select_physical_device(VkInstance instance) -> Result<VkPhysicalDevice>
  {
  }
} // namespace ia::gpu::vulkan