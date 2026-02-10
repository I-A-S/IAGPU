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

#include <gpu/enums.hpp>

namespace ia::gpu
{
  typedef struct Context_T *Context;
  typedef struct Buffer_T *Buffer;
  typedef struct Texture_T *Texture;
  typedef struct Sampler_T *Sampler;
  typedef struct Shader_T *Shader;
  typedef struct Pipeline_T *Pipeline;
  typedef struct BindingLayout_T *BindingLayout;
  typedef struct DescriptorTable_T *DescriptorTable;
  typedef struct CommandListT *CommandList;
  typedef struct Fence_T *Fence;
  typedef struct Semaphore_T *Semaphore;

  typedef void *(*SurfaceCreationCallback)(void *instance_handle, void *user_data);

  struct ContextConfig
  {
    const char *app_name = "iagpu_app";
    u8 validation_enabled = 1;
    EBackendType backend_type = EBackendType::Auto;

    void *surface_creation_callback_user_data = nullptr;
    SurfaceCreationCallback surface_creation_callback = nullptr;
  };

  struct Rect2D
  {
    i32 x = 0;
    i32 y = 0;
    u32 w = 0;
    u32 h = 0;
  };

  struct Viewport
  {
    f32 x = 0.0f;
    f32 y = 0.0f;
    f32 w = 0.0f;
    f32 h = 0.0f;
    f32 min_depth = 0.0f;
    f32 max_depth = 1.0f;
  };

  struct BufferDesc
  {
    u64 size_bytes = 0;
    EBufferUsage usage = EBufferUsage::Uniform;
    u8 host_visible = 0;
    const char *debug_name = nullptr;
  };

  struct TextureDesc
  {
    u32 width = 0;
    u32 height = 0;
    u32 depth = 1;
    u32 mip_levels = 1;
    EFormat format = EFormat::Undefined;
    u32 array_layers = 1;
    ETextureType type = ETextureType::Texture2D;
    const char *debug_name = nullptr;
  };

  struct BufferCopyRegion
  {
    u64 src_offset = 0;
    u64 dst_offset = 0;
    u64 size = 0;
  };

  struct SamplerDesc
  {
    u8 linear_filter = 0;
    u8 repeat_uv = 0;
    const char *debug_name = nullptr;
  };

  struct BindingLayoutEntry
  {
    u32 binding = 0;
    u32 count = 1;
    EShaderStage visibility = EShaderStage::Compute;
    EDescriptorType type = EDescriptorType::UniformBuffer;
  };

  struct DescriptorUpdate
  {
    DescriptorTable table = {};
    u32 binding = 0;
    u32 array_element = 0;

    Buffer buffer = {};
    u64 buffer_offset = 0;
    u64 buffer_range = 0;

    Texture texture = {};
    Sampler sampler = {};

    bool skip_update = {};
  };

  struct TextureBarrier
  {
    Texture texture = {};
    u32 base_mip_level{};
    u32 mip_level_count{}; // 0 = all remaining
    u32 base_array_layer{};
    u32 array_layer_count{}; // 0 = all remaining
    EResourceState old_state = EResourceState::Undefined;
    EResourceState new_state = EResourceState::Undefined;
  };

  struct BufferBarrier
  {
    Buffer buffer = {};
    EResourceState old_state = EResourceState::Undefined;
    EResourceState new_state = EResourceState::Undefined;
  };

  struct VertexInputBinding
  {
    u32 binding = 0;
    u32 stride = 0;
    EInputRate input_rate = EInputRate::Vertex;
  };

  struct VertexInputAttribute
  {
    u32 location = 0;
    u32 binding = 0;
    EFormat format = EFormat::Undefined;
    u32 offset = 0;
  };

  struct TextureInfo
  {
    u32 width = 0;
    u32 height = 0;
    u32 depth = 0;
    u32 layer_count = 0;
    u32 level_count = 0;
    EFormat format = EFormat::Undefined;
  };

  struct ColorAttachment
  {
    Texture texture = {};
    Texture resolve_target = {};

    f32 clear_color[4] = {0.0f, 0.0f, 0.0f, 0.0f};

    ELoadOp load_op = ELoadOp::DontCare;
    EStoreOp store_op = EStoreOp::DontCare;

    void set_clear_color(float r, float g, float b, float a)
    {
      clear_color[0] = r;
      clear_color[1] = g;
      clear_color[2] = b;
      clear_color[3] = a;
    }
  };

  struct DepthAttachment
  {
    Texture texture = {};
    f32 clear_depth = 1.0f;

    ELoadOp load_op = ELoadOp::DontCare;
    EStoreOp store_op = EStoreOp::DontCare;
  };

  struct TextureBlitRegion
  {
    u32 src_mip_level = 0;
    u32 src_base_array_layer = 0;
    u32 src_layer_count = 1;
    i32 src_x = 0;
    i32 src_y = 0;
    i32 src_z = 0;
    u32 src_width = 1;
    u32 src_height = 1;
    u32 src_depth = 1;

    u32 dst_mip_level = 0;
    u32 dst_base_array_layer = 0;
    u32 dst_layer_count = 1;
    i32 dst_x = 0;
    i32 dst_y = 0;
    i32 dst_z = 0;
    u32 dst_width = 1;
    u32 dst_height = 1;
    u32 dst_depth = 1;
  };

  struct TextureCopyRegion
  {
    Texture src_texture = {};
    u32 src_mip_level = 0;
    u32 src_base_array_layer = 0;
    u32 src_layer_count = 1;
    i32 src_x = 0;
    i32 src_y = 0;
    i32 src_z = 0;

    Texture dst_texture = {};
    u32 dst_mip_level = 0;
    u32 dst_base_array_layer = 0;
    u32 dst_layer_count = 1;
    i32 dst_x = 0;
    i32 dst_y = 0;
    i32 dst_z = 0;

    u32 width = 1;
    u32 height = 1;
    u32 depth = 1;
  };

  struct BufferTextureCopyRegion
  {
    u64 buffer_offset = 0;
    u32 buffer_row_length = 0;   // 0 = tightly packed
    u32 buffer_image_height = 0; // 0 = tightly packed

    Texture texture = {};
    u32 mip_level = 0;
    u32 base_array_layer = 0;
    u32 layer_count = 1;
    i32 texture_x = 0;
    i32 texture_y = 0;
    i32 texture_z = 0;

    u32 width = 1;
    u32 height = 1;
    u32 depth = 1;
  };

  struct GraphicsPipelineDesc
  {
    Shader vertex_shader;
    Shader fragment_shader;

    BindingLayout layouts[8];
    u32 layout_count;

    const VertexInputBinding *input_bindings;
    u32 input_binding_count;

    const VertexInputAttribute *input_attributes;
    u32 input_attribute_count;

    EFormat color_formats[7];
    EFormat depth_format;
    u32 color_attachment_count;

    u8 push_constant_size;
    EShaderStage push_constant_stages;

    ECullMode cull_mode;
    EBlendMode blend_mode;
    EPolygonMode polygon_mode;
    EPrimitiveType primitive_type;

    GraphicsPipelineDesc()
    {
      vertex_shader = {};
      fragment_shader = {};

      for (Mut<i32> i = 0; i < 8; ++i)
        layouts[i] = nullptr;
      layout_count = 0;

      input_bindings = nullptr;
      input_binding_count = 0;
      input_attributes = nullptr;
      input_attribute_count = 0;

      for (Mut<i32> i = 0; i < 7; ++i)
        color_formats[i] = EFormat::Undefined;
      depth_format = EFormat::Undefined;
      color_attachment_count = 0;

      push_constant_size = 0;
      push_constant_stages = EShaderStage::All;

      cull_mode = ECullMode::Back;
      blend_mode = EBlendMode::Alpha;
      polygon_mode = EPolygonMode::Fill;
      primitive_type = EPrimitiveType::TriangleList;
    }

    GraphicsPipelineDesc &set_shaders(Shader vs, Shader fs)
    {
      vertex_shader = vs;
      fragment_shader = fs;
      return *this;
    }

    GraphicsPipelineDesc &set_layouts(BindingLayout *ptr, u32 count)
    {
      memcpy(layouts, ptr, sizeof(layouts[0]) * count);
      layout_count = count;
      return *this;
    }

    GraphicsPipelineDesc &add_layout(const BindingLayout &layout)
    {
      if (layout_count < 8)
      {
        layouts[layout_count] = layout;
        layout_count++;
      }
      return *this;
    }

    GraphicsPipelineDesc &set_vertex_input(VertexInputBinding *bindings, u32 b_count, VertexInputAttribute *attribs,
                                           u32 a_count)
    {
      input_bindings = bindings;
      input_binding_count = b_count;
      input_attributes = attribs;
      input_attribute_count = a_count;
      return *this;
    }

    GraphicsPipelineDesc &add_color_attachment(EFormat format)
    {
      if (color_attachment_count < 7)
      {
        color_formats[color_attachment_count] = format;
        color_attachment_count++;
      }
      return *this;
    }

    GraphicsPipelineDesc &set_depth_stencil(EFormat format)
    {
      depth_format = format;
      return *this;
    }

    GraphicsPipelineDesc &set_push_constants(u8 size, EShaderStage stages)
    {
      push_constant_size = size;
      push_constant_stages = stages;
      return *this;
    }

    GraphicsPipelineDesc &set_rasterization(ECullMode cull, EBlendMode blend, EPolygonMode poly = EPolygonMode::Fill,
                                            EPrimitiveType prim = EPrimitiveType::TriangleList)
    {
      cull_mode = cull;
      blend_mode = blend;
      polygon_mode = poly;
      primitive_type = prim;
      return *this;
    }
  };

  struct ComputePipelineDesc
  {
    Shader compute_shader = {};
    BindingLayout *layouts = nullptr;
    u32 layout_count = 0;

    ComputePipelineDesc &set_shader(Shader cs)
    {
      compute_shader = cs;
      return *this;
    }

    ComputePipelineDesc &set_layouts(BindingLayout *ptr, u32 count)
    {
      layouts = ptr;
      layout_count = count;
      return *this;
    }

    ComputePipelineDesc &set_layout(BindingLayout *ptr)
    {
      return set_layouts(ptr, 1);
    }
  };
} // namespace ia::gpu