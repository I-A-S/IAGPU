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

#include <gpu/concepts.hpp>

namespace ia::gpu::vulkan
{
  class CommandList
  {
public:
    void begin_rendering(u32 count, const ColorAttachment *colors, const DepthAttachment *depth);
    void end_rendering();

    void begin_compute();
    void end_compute();

    void bind_vertex_buffers(u32 first, std::span<const Buffer> buffers, std::span<const u64> offsets);
    void bind_index_buffer(Buffer buffer, u64 offset, bool use_32_bit);
    void bind_pipeline(Pipeline pipeline);
    void bind_descriptor_table(u32 index, DescriptorTable table);

    void push_constants(EShaderStage stage, u32 offset, u32 size, const void *data);

    void set_viewport(const Viewport &vp);
    void set_scissor(const Rect2D &rect);

    void draw(u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance);
    void draw_indexed(u32 index_count, u32 instance_count, u32 first_index, u32 vertex_offset, u32 first_instance);
    void draw_indexed_indirect(Buffer buffer, u64 offset, u32 draw_count, u32 stride);
    void dispatch(u32 x, u32 y, u32 z);

    void transition_buffer(Buffer buffer, EResourceState state);
    void transition_texture(Texture texture, EResourceState state);
    void transition_texture(Texture texture, EResourceState state, u32 base_mip, u32 mip_count, u32 base_layer,
                            u32 layer_count);
    void flush_transitions();

    void pipeline_barrier(std::span<const BufferBarrier> buf_barriers, std::span<const TextureBarrier> tex_barriers);

    void copy_buffer(Buffer src, Buffer dst, std::span<const BufferCopyRegion> regions);
    void copy_texture(std::span<const TextureCopyRegion> regions);
    void copy_buffer_to_texture(Buffer src, std::span<const BufferTextureCopyRegion> regions);
    void copy_texture_to_buffer(Buffer src, std::span<const BufferTextureCopyRegion> regions);
    void blit_texture(Texture src, EResourceState src_state, Texture dst, EResourceState dst_state,
                      std::span<const TextureBlitRegion> regions, bool filter);
  };

  static_assert(IsCommandList<CommandList>, "CommandList must satisfy IsCommandList concept");
} // namespace ia::gpu::vulkan