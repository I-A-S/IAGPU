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

#define VMA_IMPLEMENTATION

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
    VK_CALL(vkCreateDescriptorPool(m_handle, &pool_info, nullptr, m_descriptor_pool.ptr()),
            "Failed to create descriptor pool");

    return {};
  }

  auto Device::wait_idle() -> void
  {
    vkDeviceWaitIdle(m_handle);
  }

  auto Device::initialize_device(VkInstance instance, Span<const char *> extensions) -> Result<void>
  {
    m_physical_device = AU_TRY(select_physical_device(instance));

    Mut<Vec<VkDeviceQueueCreateInfo>> device_queue_create_infos;

    Mut<Vec<VkQueueFamilyProperties>> queue_family_props;
    VK_ENUM_CALL(vkGetPhysicalDeviceQueueFamilyProperties, queue_family_props, m_physical_device);

    for (Mut<u32> i = 0; i < queue_family_props.size(); i++)
    {
      const auto &props = queue_family_props[i];
      if (props.queueFlags & VK_QUEUE_COMPUTE_BIT)
      {
        if (m_compute_queue_family == UINT32_MAX)
          m_compute_queue_family = i;
        else if (m_graphics_queue_family != UINT32_MAX && i != m_graphics_queue_family)
        {
          m_graphics_queue_family = i;
          break;
        }
      }
    }

    for (Mut<u32> i = 0; i < queue_family_props.size(); i++)
    {
      const auto &props = queue_family_props[i];
      if (props.queueFlags & VK_QUEUE_TRANSFER_BIT)
      {
        if (m_transfer_queue_family == UINT32_MAX)
          m_transfer_queue_family = i;
        else if (i != m_graphics_queue_family && i != m_compute_queue_family)
        {
          m_transfer_queue_family = i;
          break;
        }
      }
    }

    if (m_compute_queue_family == UINT32_MAX)
      return fail("Failed to find a compute queue");

    Mut<HashMap<u32, u32>> queue_family_index_map;

    if (m_graphics_queue_family != UINT32_MAX)
      queue_family_index_map[m_graphics_queue_family]++;
    if (m_compute_queue_family != UINT32_MAX)
      queue_family_index_map[m_compute_queue_family]++;
    if (m_transfer_queue_family != UINT32_MAX)
      queue_family_index_map[m_transfer_queue_family]++;

    Mut<Vec<Vec<f32>>> priority_storage;
    priority_storage.reserve(queue_family_index_map.size());
    for (auto &[familyIndex, count] : queue_family_index_map)
    {
      Mut<VkDeviceQueueCreateInfo> info{};
      info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      info.queueFamilyIndex = familyIndex;

      info.queueCount = std::min(count, queue_family_props[familyIndex].queueCount);

      Mut<Vec<f32>> priorities(info.queueCount, 1.0f);
      priority_storage.push_back(priorities);

      info.pQueuePriorities = priority_storage.back().data();
      device_queue_create_infos.push_back(info);
    }

    Mut<VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT> dynamic_vertex_input_features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT,
        .vertexInputDynamicState = VK_TRUE,
    };

    Mut<VkPhysicalDeviceExtendedDynamicStateFeaturesEXT> enable_extended_dynamic_state_features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
        .pNext = &dynamic_vertex_input_features,
        .extendedDynamicState = VK_TRUE,
    };

    Mut<VkPhysicalDeviceVulkan13Features> enable_vulkan13_features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &enable_extended_dynamic_state_features,
        .synchronization2 = VK_TRUE,
        .dynamicRendering = VK_TRUE,
    };

    const VkPhysicalDeviceFeatures2 enable_device_features2{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &enable_vulkan13_features,
    };

    const VkDeviceCreateInfo device_create_info{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &enable_device_features2,
        .queueCreateInfoCount = static_cast<u32>(device_queue_create_infos.size()),
        .pQueueCreateInfos = device_queue_create_infos.data(),
        .enabledLayerCount = 0,
        .enabledExtensionCount = static_cast<u32>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };
    VK_CALL(vkCreateDevice(m_physical_device, &device_create_info, nullptr, m_handle.ptr()), "Creating logical device");

    volkLoadDevice(m_handle);

    Mut<HashMap<u32, u32>> tmp_queue_family_index_map;
    if (m_graphics_queue_family != UINT32_MAX)
    {
      u32 q_index = tmp_queue_family_index_map[m_graphics_queue_family]++;
      if (q_index < queue_family_props[m_graphics_queue_family].queueCount)
        vkGetDeviceQueue(m_handle, m_graphics_queue_family, q_index, &m_graphics_queue);
    }
    if (m_compute_queue_family != UINT32_MAX)
    {
      u32 q_index = tmp_queue_family_index_map[m_compute_queue_family]++;
      if (q_index >= queue_family_props[m_compute_queue_family].queueCount)
        q_index = 0;

      vkGetDeviceQueue(m_handle, m_compute_queue_family, q_index, &m_compute_queue);
    }
    if (m_transfer_queue_family != UINT32_MAX)
    {
      u32 q_index = tmp_queue_family_index_map[m_transfer_queue_family]++;
      if (q_index >= queue_family_props[m_transfer_queue_family].queueCount)
        q_index = 0;

      vkGetDeviceQueue(m_handle, m_transfer_queue_family, q_index, &m_transfer_queue);
    }

    VkFenceCreateInfo fence_create_info{};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VK_CALL(vkCreateFence(m_handle, &fence_create_info, nullptr, m_command_submit_fence.ptr()),
            "Creating command submit fence");

    Mut<VmaAllocatorCreateInfo> allocator_create_info{
        .physicalDevice = m_physical_device,
        .device = m_handle,
        .instance = instance,
        .vulkanApiVersion = VULKAN_API_VERSION,
    };
    Mut<VmaVulkanFunctions> vma_vulkan_functions{};
    VK_CALL(vmaImportVulkanFunctionsFromVolk(&allocator_create_info, &vma_vulkan_functions), "Importing VMA functions");
    allocator_create_info.pVulkanFunctions = &vma_vulkan_functions;
    VK_CALL(vmaCreateAllocator(&allocator_create_info, m_allocator.ptr()), "Creating VMA allocator");

    return {};
  }

  auto Device::select_physical_device(VkInstance instance) -> Result<VkPhysicalDevice>
  {
    Mut<VkPhysicalDeviceProperties> props{};
    Mut<VkPhysicalDeviceFeatures> features{};
    Mut<Vec<VkPhysicalDevice>> physical_devices;
    VK_ENUM_CALL(vkEnumeratePhysicalDevices, physical_devices, instance);

    for (const auto &pd : physical_devices)
    {
      vkGetPhysicalDeviceProperties(pd, &props);
      vkGetPhysicalDeviceFeatures(pd, &features);

      // [IATODO]: IMPL GPU Ranking

      if (m_surface == nullptr)
      {
        m_physical_device = pd;
        break;
      }
      else
      {
        Mut<u32> queue_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(pd, &queue_count, nullptr);
        Mut<Vec<VkQueueFamilyProperties>> queues(queue_count);
        vkGetPhysicalDeviceQueueFamilyProperties(pd, &queue_count, queues.data());

        for (Mut<u32> i = 0; i < queue_count; i++)
        {
          Mut<VkBool32> supports_present = VK_FALSE;
          vkGetPhysicalDeviceSurfaceSupportKHR(pd, i, m_surface, &supports_present);

          if ((queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && supports_present)
          {
            m_physical_device = pd;
            m_graphics_queue_family = i;
            goto found;
          }
        }
      }
    }

  found:
    if (m_physical_device == VK_NULL_HANDLE)
      return fail("Failed to find suitable graphics hardware.");

    GPU_LOG_INFO("Using the hardware device \"{}\"", props.deviceName);
    return m_physical_device;
  }
} // namespace ia::gpu::vulkan