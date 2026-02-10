set(SRC_FILES
  "cpp/gpu.cpp"

  "cpp/vulkan/command_list_compute.cpp"
  "cpp/vulkan/command_list_core.cpp"
  "cpp/vulkan/command_list_graphics.cpp"
  "cpp/vulkan/context_compute.cpp"
  "cpp/vulkan/context_core.cpp"
  "cpp/vulkan/context_graphics.cpp"
  "cpp/vulkan/device.cpp"
)

add_library(IAGPU STATIC ${SRC_FILES})

target_include_directories(IAGPU PUBLIC 
  hpp/ # [IATODO] MAKE PRIVATE. DEBUG ONLY
  ${IAGPU_ROOT}/include
  "${CMAKE_BINARY_DIR}/generated/include/"
)

#target_include_directories(IAGPU PRIVATE hpp)

target_link_libraries(IAGPU PUBLIC
  IACrux

  # [IATODO]: Make dependent on IAGPU_ENABLE_BACKEND_VULKAN
  volk::volk
  spirv-reflect-static
  GPUOpen::VulkanMemoryAllocator
)