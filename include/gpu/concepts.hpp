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

#include <gpu/structs.hpp>

#include <concepts>
#include <span>
#include <utility>

namespace ia::gpu
{
  template<typename T>
  concept IsCommandList = requires(T cmd, bool bool_val, u32 u32_val, u64 u64_val, Buffer buffer, Texture texture,
                                   Pipeline pipeline, DescriptorTable descriptor_table,
                                   const ColorAttachment *color_attachments, const DepthAttachment *depth_attachment,
                                   const Viewport &viewport, const Rect2D &scissor, EShaderStage shader_stage,
                                   EResourceState resource_state, const void *const_void_ptr) {
    { cmd.begin_rendering(u32_val, color_attachments, depth_attachment) } -> std::same_as<void>;
    { cmd.end_rendering() } -> std::same_as<void>;

    { cmd.begin_compute() } -> std::same_as<void>;
    { cmd.end_compute() } -> std::same_as<void>;

    { cmd.bind_vertex_buffers(u32_val, std::span<const Buffer>{}, std::span<const u64>{}) } -> std::same_as<void>;
    { cmd.bind_index_buffer(buffer, u64_val, bool_val) } -> std::same_as<void>;
    { cmd.bind_pipeline(pipeline) } -> std::same_as<void>;
    { cmd.bind_descriptor_table(u32_val, descriptor_table) } -> std::same_as<void>;

    { cmd.push_constants(shader_stage, u32_val, u32_val, const_void_ptr) } -> std::same_as<void>;

    { cmd.set_viewport(viewport) } -> std::same_as<void>;
    { cmd.set_scissor(scissor) } -> std::same_as<void>;

    { cmd.draw(u32_val, u32_val, u32_val, u32_val) } -> std::same_as<void>;
    { cmd.draw_indexed(u32_val, u32_val, u32_val, u32_val, u32_val) } -> std::same_as<void>;
    { cmd.draw_indexed_indirect(buffer, u64_val, u32_val, u32_val) } -> std::same_as<void>;
    { cmd.dispatch(u32_val, u32_val, u32_val) } -> std::same_as<void>;

    { cmd.transition_buffer(buffer, resource_state) } -> std::same_as<void>;
    { cmd.transition_texture(texture, resource_state) } -> std::same_as<void>;
    { cmd.transition_texture(texture, resource_state, u32_val, u32_val, u32_val, u32_val) } -> std::same_as<void>;
    { cmd.flush_transitions() } -> std::same_as<void>;

    { cmd.pipeline_barrier(std::span<const BufferBarrier>{}, std::span<const TextureBarrier>{}) } -> std::same_as<void>;

    { cmd.copy_buffer(buffer, buffer, std::span<const BufferCopyRegion>{}) } -> std::same_as<void>;
    { cmd.copy_texture(std::span<const TextureCopyRegion>{}) } -> std::same_as<void>;
    { cmd.copy_buffer_to_texture(buffer, std::span<const BufferTextureCopyRegion>{}) } -> std::same_as<void>;
    { cmd.copy_texture_to_buffer(buffer, std::span<const BufferTextureCopyRegion>{}) } -> std::same_as<void>;
    {
      cmd.blit_texture(texture, resource_state, texture, resource_state, std::span<const TextureBlitRegion>{}, bool_val)
    } -> std::same_as<void>;
  };

  template<typename T>
  concept IsContext =
      requires(T ctx, typename T::CmdListType *cmd_list, u32 u32_val, u64 u64_val, bool bool_val, Buffer buffer,
               Texture texture, Sampler sampler, Shader shader, Pipeline pipeline, BindingLayout binding_layout,
               DescriptorTable descriptor_table, Fence fence, const ComputePipelineDesc &compute_desc,
               const GraphicsPipelineDesc &graphics_desc, std::span<const BufferDesc> buffer_descs,
               std::span<Buffer> out_buffers, std::span<const Buffer> buffers,
               std::span<const TextureDesc> texture_descs, std::span<Texture> out_textures, std::span<Texture> textures,
               std::span<const SamplerDesc> sampler_descs, std::span<Sampler> out_samplers, std::span<Sampler> samplers,
               std::span<Fence> out_fences, std::span<const Fence> fences, std::span<const u8> data_span,
               std::span<u8> mut_data_span, std::span<const BindingLayoutEntry> binding_layout_entries,
               std::span<DescriptorTable> out_descriptor_tables, std::span<DescriptorTable> descriptor_tables,
               std::span<const BufferTextureCopyRegion> buffer_texture_copy_regions,
               std::span<const DescriptorUpdate> descriptor_updates) {
        typename T::CmdListType;
        requires IsCommandList<typename T::CmdListType>;

        { ctx.wait_idle() } -> std::same_as<void>;

        { ctx.begin_frame() } -> std::same_as<std::pair<typename T::CmdListType *, u32>>;
        { ctx.end_frame(cmd_list) } -> std::same_as<bool>;

        { ctx.create_buffers(buffer_descs, out_buffers) } -> std::convertible_to<bool>;
        { ctx.destroy_buffers(buffers) } -> std::same_as<void>;

        { ctx.create_textures(texture_descs, out_textures) } -> std::convertible_to<bool>;
        { ctx.destroy_textures(textures) } -> std::same_as<void>;

        { ctx.create_compute_pipeline(compute_desc) } -> std::same_as<Result<Pipeline>>;
        { ctx.create_graphics_pipeline(graphics_desc) } -> std::same_as<Result<Pipeline>>;
        { ctx.destroy_pipeline(pipeline) } -> std::same_as<void>;

        { ctx.create_samplers(sampler_descs, out_samplers) } -> std::convertible_to<bool>;
        { ctx.destroy_samplers(samplers) } -> std::same_as<void>;

        { ctx.create_fences(out_fences, bool_val) } -> std::convertible_to<bool>;
        { ctx.destroy_fences(fences) } -> std::same_as<void>;
        { ctx.wait_for_fences(fences, bool_val, u64_val) } -> std::same_as<bool>;
        { ctx.reset_fences(fences) } -> std::same_as<bool>;

        { ctx.create_shader(data_span) } -> std::same_as<Result<Shader>>;
        { ctx.destroy_shader(shader) } -> std::same_as<void>;

        { ctx.create_binding_layout(binding_layout_entries) } -> std::same_as<Result<BindingLayout>>;
        { ctx.destroy_binding_layout(binding_layout) } -> std::same_as<void>;

        { ctx.create_descriptor_tables(binding_layout, out_descriptor_tables) } -> std::convertible_to<bool>;
        { ctx.destroy_descriptor_tables(descriptor_tables) } -> std::same_as<void>;
        { ctx.update_descriptor_tables(descriptor_updates) } -> std::same_as<void>;

        { ctx.resize_swapchain(u32_val, u32_val) } -> std::same_as<Result<void>>;
        { ctx.get_back_buffer() } -> std::same_as<Texture>;

        { ctx.update_host_visible_buffer(buffer, u64_val, data_span) } -> std::same_as<void>;
        { ctx.read_host_visible_buffer(buffer, u64_val, mut_data_span) } -> std::same_as<void>;

        { ctx.update_texture(texture, data_span, buffer_texture_copy_regions) } -> std::convertible_to<bool>;
        { ctx.generate_mipmaps(texture) } -> std::convertible_to<bool>;

        { ctx.get_default_sampler() } -> std::same_as<Sampler>;
        { ctx.get_buffer_size(buffer) } -> std::same_as<u32>;
        { ctx.get_texture_info(texture) } -> std::same_as<TextureInfo>;

        {
          ctx.execute_immediate_commands([](typename T::CmdListType * _) {})
        } -> std::convertible_to<bool>;
      };
} // namespace ia::gpu