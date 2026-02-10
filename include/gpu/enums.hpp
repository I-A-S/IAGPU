// IAGPU: IA GPU Hardware Interface
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

#include <crux/crux.hpp>

namespace ia::gpu
{

  enum class EBackendType
  {
    Auto = 0,
    Vulkan,
    WebGpu,
  };

  enum class EFormat
  {
    Undefined = 0,

    R8G8B8A8Unorm,
    R8G8B8A8Srgb,
    B8G8R8A8Srgb,
    B8G8R8A8Unorm,
    R32Uint,
    R32Float,
    R32G32Float,
    R32G32B32Float,
    R32G32B32A32Float,

    D16Unorm,
    D16UnormS8Uint,
    D24UnormS8Uint,
    D32Sfloat,
    D32SfloatS8Uint,

    Bc1RgbUnormBlock,
    Bc1RgbSrgbBlock,
    Bc1RgbaUnormBlock,
    Bc1RgbaSrgbBlock,
    Bc2UnormBlock,
    Bc2SrgbBlock,
    Bc3UnormBlock,
    Bc3SrgbBlock,
    Bc5UnormBlock,
    Bc5SnormBlock,
  };

  enum class ETextureType
  {
    Texture2D = 0,
    Texture3D,
    TextureCube,
    Texture2DArray,
  };

  enum class EShaderStage : u32
  {
    None = 0,
    Vertex = (1 << 0),
    Fragment = (1 << 1),
    Compute = (1 << 2),
    All = (1 << 0) | (1 << 1) | (1 << 2),
  };

  enum class EBufferUsage : u32
  {
    Vertex = (1 << 0),
    Index = (1 << 1),
    Uniform = (1 << 2),
    Storage = (1 << 3),
    Transfer = (1 << 4),
    Indirect = (1 << 5)
  };

  enum class EResourceState
  {
    Undefined = 0,
    TransferSrc,
    TransferDst,
    GeneralRead,
    GeneralWrite,
    ColorTarget,
    DepthTarget,
    Present,
  };

  enum class EDescriptorType
  {
    UniformBuffer = 0,
    StorageBuffer,
    SampledImage,
    StorageImage,
  };

  enum class EInputRate
  {
    Vertex = 0,
    Instance
  };

  enum class EPolygonMode
  {
    Fill = 0,
    Line,
    Point,
  };

  enum class ECullMode
  {
    None = 0,
    Back,
    Front,
  };

  enum class EBlendMode
  {
    Opaque = 0,
    Alpha,
    Premultiplied,
    Additive,
    Multiply,
    Modulate
  };

  enum class EPrimitiveType
  {
    PointList = 0,
    LineList,
    LineStrip,
    TriangleList,
    TriangleStrip,
  };

  enum class ELoadOp
  {
    Load = 0,
    Clear,
    DontCare
  };

  enum class EStoreOp
  {
    Store = 0,
    DontCare
  };

  inline auto is_depth_format(EFormat format) -> bool
  {
    return (format == EFormat::D16Unorm || format == EFormat::D16UnormS8Uint || format == EFormat::D24UnormS8Uint ||
            format == EFormat::D32Sfloat || format == EFormat::D32SfloatS8Uint);
  }

  inline auto is_compressed_format(EFormat format) -> bool
  {
    return format >= EFormat::Bc1RgbUnormBlock;
  }

  inline auto get_compressed_format_block_size(EFormat format) -> u32
  {
    if (!is_compressed_format(format))
    {
      return 0;
    }

    switch (format)
    {
    case EFormat::Bc1RgbUnormBlock:
    case EFormat::Bc1RgbSrgbBlock:
    case EFormat::Bc1RgbaUnormBlock:
    case EFormat::Bc1RgbaSrgbBlock:
      return 8;

    default:
      return 16;
    }
  }

  inline auto get_uncompressed_pixel_size(EFormat format) -> u32
  {
    switch (format)
    {
    case EFormat::R8G8B8A8Unorm:
    case EFormat::R8G8B8A8Srgb:
    case EFormat::B8G8R8A8Unorm:
    case EFormat::B8G8R8A8Srgb:
    case EFormat::R32Uint:
    case EFormat::R32Float:
    case EFormat::D32Sfloat:
    case EFormat::D24UnormS8Uint:
      return 4;

    case EFormat::R32G32Float:
    case EFormat::D32SfloatS8Uint:
      return 8;

    case EFormat::R32G32B32Float:
      return 12;

    case EFormat::R32G32B32A32Float:
      return 16;

    case EFormat::D16Unorm:
      return 2;

    default:
      return 0;
    }
  }

} // namespace ia::gpu