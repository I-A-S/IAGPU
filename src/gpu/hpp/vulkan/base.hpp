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

#include <vulkan/context.hpp>

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0

#include <volk.h>
#include <vk_mem_alloc.h>

#define VK_CALL(call, description)                                                                                     \
  {                                                                                                                    \
    const auto r = call;                                                                                               \
    if IA_B_UNLIKELY (r != VK_SUCCESS)                                                                                 \
      return fail("'{}' failed with code {}", description, (i64) r);                                                   \
  }

#define VK_ENUM_CALL(call, result, ...)                                                                                \
  {                                                                                                                    \
    u32 __count;                                                                                                       \
    call(__VA_ARGS__, &__count, nullptr);                                                                              \
    result.resize(__count);                                                                                            \
    call(__VA_ARGS__, &__count, result.data());                                                                        \
  }

namespace ia::gpu::vulkan
{
  static constexpr u32 VULKAN_API_VERSION = VK_MAKE_VERSION(1, 3, 0);

  struct BufferImpl
  {
    VmaAllocator vma_allocator;

    VkBuffer handle;
    VmaAllocation allocation;
    VmaAllocationInfo alloc_info;
    u64 size;

    EResourceState current_state{EResourceState::Undefined};

    BufferImpl(VmaAllocator allocator, VkBuffer buffer, VmaAllocation allocation, Ref<VmaAllocationInfo> info,
               u64 size_bytes)
        : vma_allocator(allocator), handle(buffer), allocation(allocation), alloc_info(info), size(size_bytes)
    {
    }

    void *map()
    {
      if (alloc_info.pMappedData)
        return alloc_info.pMappedData;

      Mut<void *> data;
      vmaMapMemory(vma_allocator, allocation, &data);
      return data;
    }

    void unmap()
    {
      if (!alloc_info.pMappedData)
        vmaUnmapMemory(vma_allocator, allocation);
    }
  };

  struct BindingLayoutImpl
  {
    VkDescriptorSetLayout handle{VK_NULL_HANDLE};
    HashMap<u32, VkDescriptorType> binding_types;
  };

  struct PipelineImpl
  {
    u64 attachment_hash{};
    VkPipeline handle{VK_NULL_HANDLE};
    VkPipelineLayout layout{VK_NULL_HANDLE};
    VkPipelineBindPoint bind_point{VK_PIPELINE_BIND_POINT_GRAPHICS};
  };

  struct DescriptorTableImpl
  {
    VkDescriptorSet handle{VK_NULL_HANDLE};
    BindingLayoutImpl *layout{nullptr};
  };

  struct ShaderImpl
  {
    VkShaderModule handle{VK_NULL_HANDLE};
    VkPipelineShaderStageCreateInfo stage_create_info{};
  };

  struct TextureImpl
  {
    VmaAllocator vma_allocator;
    VkImage handle{VK_NULL_HANDLE};
    VkImageView view_handle{VK_NULL_HANDLE};
    VmaAllocation allocation{VK_NULL_HANDLE};
    VmaAllocationInfo alloc_info{};
    bool is_compressed_data{};

    VkExtent3D extent{};
    VkFormat vk_format{VK_FORMAT_UNDEFINED};
    EFormat format{EFormat::Undefined};
    u32 mip_levels{1};
    u32 array_layer_count{1};

    TextureImpl() = default;

    TextureImpl(VkImage handle, VkImageView view, VkExtent2D extent2_d) : handle(handle), view_handle(view)
    {
      extent = {extent2_d.width, extent2_d.height, 1};
      format = EFormat::B8G8R8A8Srgb;
      vk_format = VK_FORMAT_B8G8R8A8_SRGB;
      vma_allocator = nullptr;
      m_subresource_states.resize(mip_levels * array_layer_count, EResourceState::Undefined);
    }

    EResourceState get_current_state(u32 layer, u32 level)
    {
      if IA_B_UNLIKELY (m_subresource_states.empty())
        m_subresource_states.resize(mip_levels * array_layer_count, EResourceState::Undefined);
      return m_subresource_states[layer * this->mip_levels + level];
    }

    void set_current_state(EResourceState new_state, u32 mip_base, u32 mip_count, u32 layer_base, u32 layer_count)
    {
      Mut<u32> actual_mip_count = mip_count;
      if (mip_count == VK_REMAINING_MIP_LEVELS)
        actual_mip_count = this->mip_levels - mip_base;

      Mut<u32> actual_layer_count = layer_count;
      if (layer_count == VK_REMAINING_ARRAY_LAYERS)
        actual_layer_count = this->array_layer_count - layer_base;

      for (Mut<u32> l = 0; l < actual_layer_count; l++)
      {
        const u32 row_offset = (layer_base + l) * this->mip_levels;
        for (Mut<u32> m = 0; m < actual_mip_count; m++)
          m_subresource_states[row_offset + (mip_base + m)] = new_state;
      }
    }

private:
    Vec<EResourceState> m_subresource_states;
  };

  inline constexpr VkFormat map_format(EFormat format)
  {
    switch (format)
    {
    case EFormat::R8G8B8A8Unorm:
      return VK_FORMAT_R8G8B8A8_UNORM;
    case EFormat::R8G8B8A8Srgb:
      return VK_FORMAT_R8G8B8A8_SRGB;
    case EFormat::B8G8R8A8Srgb:
      return VK_FORMAT_B8G8R8A8_SRGB;
    case EFormat::B8G8R8A8Unorm:
      return VK_FORMAT_B8G8R8A8_UNORM;
    case EFormat::R32Float:
      return VK_FORMAT_R32_SFLOAT;
    case EFormat::R32G32B32A32Float:
      return VK_FORMAT_R32G32B32A32_SFLOAT;
    case EFormat::R32Uint:
      return VK_FORMAT_R32_UINT;
    case EFormat::R32G32Float:
      return VK_FORMAT_R32G32_SFLOAT;
    case EFormat::R32G32B32Float:
      return VK_FORMAT_R32G32B32_SFLOAT;

    case EFormat::D32Sfloat:
      return VK_FORMAT_D32_SFLOAT;
    case EFormat::D16Unorm:
      return VK_FORMAT_D16_UNORM;
    case EFormat::D16UnormS8Uint:
      return VK_FORMAT_D16_UNORM_S8_UINT;
    case EFormat::D24UnormS8Uint:
      return VK_FORMAT_D24_UNORM_S8_UINT;
    case EFormat::D32SfloatS8Uint:
      return VK_FORMAT_D32_SFLOAT_S8_UINT;

    case EFormat::Bc1RgbUnormBlock:
      return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
    case EFormat::Bc1RgbSrgbBlock:
      return VK_FORMAT_BC1_RGB_SRGB_BLOCK;
    case EFormat::Bc1RgbaUnormBlock:
      return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    case EFormat::Bc1RgbaSrgbBlock:
      return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
    case EFormat::Bc2UnormBlock:
      return VK_FORMAT_BC2_UNORM_BLOCK;
    case EFormat::Bc2SrgbBlock:
      return VK_FORMAT_BC2_SRGB_BLOCK;
    case EFormat::Bc3UnormBlock:
      return VK_FORMAT_BC3_UNORM_BLOCK;
    case EFormat::Bc3SrgbBlock:
      return VK_FORMAT_BC3_SRGB_BLOCK;
    case EFormat::Bc5UnormBlock:
      return VK_FORMAT_BC5_UNORM_BLOCK;
    case EFormat::Bc5SnormBlock:
      return VK_FORMAT_BC5_SNORM_BLOCK;

    default:
      return VK_FORMAT_UNDEFINED;
    }
  }

  inline constexpr VkDescriptorType map_descriptor_type(EDescriptorType type)
  {
    switch (type)
    {
    case EDescriptorType::UniformBuffer:
      return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case EDescriptorType::StorageBuffer:
      return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    case EDescriptorType::SampledImage:
      return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    case EDescriptorType::StorageImage:
      return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    default:
      return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    }
  }

  inline constexpr VkShaderStageFlags map_shader_stages(EShaderStage stage)
  {
    Mut<VkShaderStageFlags> flags = 0;
    if ((u32) stage & (u32) EShaderStage::Vertex)
      flags |= VK_SHADER_STAGE_VERTEX_BIT;
    if ((u32) stage & (u32) EShaderStage::Fragment)
      flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
    if ((u32) stage & (u32) EShaderStage::Compute)
      flags |= VK_SHADER_STAGE_COMPUTE_BIT;
    return flags;
  }

  inline constexpr VkAttachmentLoadOp map_load_op(ELoadOp op)
  {
    switch (op)
    {
    case ELoadOp::Load:
      return VK_ATTACHMENT_LOAD_OP_LOAD;
    case ELoadOp::Clear:
      return VK_ATTACHMENT_LOAD_OP_CLEAR;
    case ELoadOp::DontCare:
      return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    }
    return VK_ATTACHMENT_LOAD_OP_NONE;
  }

  inline constexpr VkAttachmentStoreOp map_store_op(EStoreOp op)
  {
    switch (op)
    {
    case EStoreOp::Store:
      return VK_ATTACHMENT_STORE_OP_STORE;
    case EStoreOp::DontCare:
      return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    }

    return VK_ATTACHMENT_STORE_OP_NONE;
  }

  inline constexpr VkImageLayout map_image_layout(EResourceState state)
  {
    switch (state)
    {
    case EResourceState::Undefined:
      return VK_IMAGE_LAYOUT_UNDEFINED;

    case EResourceState::TransferSrc:
      return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

    case EResourceState::TransferDst:
      return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

    case EResourceState::GeneralRead:
      return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    case EResourceState::GeneralWrite:
      return VK_IMAGE_LAYOUT_GENERAL;

    case EResourceState::ColorTarget:
      return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    case EResourceState::DepthTarget:
      return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    case EResourceState::Present:
      return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    default:
      return VK_IMAGE_LAYOUT_GENERAL;
    }
  }
} // namespace ia::gpu::vulkan