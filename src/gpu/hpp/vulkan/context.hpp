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

#include <vulkan/command_list.hpp>

namespace ia::gpu::vulkan
{
  class Context
  {
public:
    using CmdListType = CommandList;

    void wait_idle();

    std::pair<CmdListType *, u32> begin_frame();
    bool end_frame(CmdListType *cmd);

    bool create_buffers(std::span<const BufferDesc> descs, std::span<Buffer> out);
    void destroy_buffers(std::span<const Buffer> buffers);

    bool create_textures(std::span<const TextureDesc> descs, std::span<Texture> out);
    void destroy_textures(std::span<const Texture> textures);

    Result<Pipeline> create_compute_pipeline(const ComputePipelineDesc &desc);
    Result<Pipeline> create_graphics_pipeline(const GraphicsPipelineDesc &desc);
    void destroy_pipeline(Pipeline p);

    bool create_samplers(std::span<const SamplerDesc> descs, std::span<Sampler> out);
    void destroy_samplers(std::span<Sampler> samplers);

    bool create_fences(std::span<Fence> out, bool signaled);
    void destroy_fences(std::span<const Fence> fences);
    bool wait_for_fences(std::span<const Fence> fences, bool wait_all, u64 timeout);
    bool reset_fences(std::span<const Fence> fences);

    Result<Shader> create_shader(std::span<const u8> data);
    void destroy_shader(Shader s);

    Result<BindingLayout> create_binding_layout(std::span<const BindingLayoutEntry> entries);
    void destroy_binding_layout(BindingLayout l);

    bool create_descriptor_tables(BindingLayout layout, std::span<DescriptorTable> out);
    void destroy_descriptor_tables(std::span<DescriptorTable> tables);
    void update_descriptor_tables(std::span<const DescriptorUpdate> updates);

    bool resize_swapchain(u32 width, u32 height);
    Texture get_back_buffer();

    void update_host_visible_buffer(Buffer buffer, u64 offset, std::span<const u8> data);
    void read_host_visible_buffer(Buffer buffer, u64 offset, std::span<u8> data);

    bool update_texture(Texture texture, std::span<const u8> data, std::span<const BufferTextureCopyRegion> regions);
    bool generate_mipmaps(Texture texture);

    Sampler get_default_sampler();
    u32 get_buffer_size(Buffer b);
    TextureInfo get_texture_info(Texture t);

    template<typename Func> bool execute_immediate_commands(Func &&func);
  };

  static_assert(IsContext<Context>, "VulkanContext must satisfy IsContext concept");
} // namespace ia::gpu::vulkan