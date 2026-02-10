set(SRC_FILES
  "cpp/gpu.cpp"
)

add_library(IAGPU STATIC ${SRC_FILES})

target_include_directories(IAGPU PUBLIC ${IAGPU_ROOT}/include)
target_include_directories(IAGPU PRIVATE hpp)

target_link_libraries(IAGPU PUBLIC
  IACrux

  # [IATODO]: Make dependent on IAGPU_ENABLE_BACKEND_VULKAN
  volk::volk
  spirv-reflect-static
  GPUOpen::VulkanMemoryAllocator
)