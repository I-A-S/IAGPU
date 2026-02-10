include(FetchContent)

FetchContent_Declare(
    volk
    GIT_REPOSITORY https://github.com/zeux/volk.git
    GIT_TAG        master 
    SYSTEM
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(volk)

FetchContent_Declare(
    VulkanMemoryAllocator
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG        master  
    SYSTEM
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(VulkanMemoryAllocator)

FetchContent_Declare(
    spirv-reflect
    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Reflect.git
    GIT_TAG        main 
    SYSTEM
    EXCLUDE_FROM_ALL
)
set(SPIRV_REFLECT_EXECUTABLE OFF CACHE BOOL "" FORCE)
set(SPIRV_REFLECT_STATIC_LIB ON  CACHE BOOL "" FORCE)
set(SPIRV_REFLECT_INSTALL    OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(spirv-reflect)

if(IAGPU_ENABLE_PIPELINE_BAKER)
    FetchContent_Declare(
        slang_prebuilt
        URL "https://github.com/shader-slang/slang/releases/download/v2026.1/slang-2026.1-linux-x86_64.zip"
    )
    FetchContent_MakeAvailable(slang_prebuilt)

    add_library(slang SHARED IMPORTED)

    set(SLANG_INCLUDE_DIR "${slang_prebuilt_SOURCE_DIR}/include")
    set(SLANG_LIB_PATH    "${slang_prebuilt_SOURCE_DIR}/lib/libslang.so")

    set_target_properties(slang PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${SLANG_INCLUDE_DIR}"
        IMPORTED_LOCATION             "${SLANG_LIB_PATH}"
    )
endif()

if(IAGPU_BUILD_SAMPLES)

    FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG        main    
        SYSTEM
        EXCLUDE_FROM_ALL
    )
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(SDL_TEST_LIBRARY  OFF CACHE BOOL "" FORCE)
    set(SDL_EXAMPLES      OFF CACHE BOOL "" FORCE)
    set(SDL_TESTS         OFF CACHE BOOL "" FORCE)
    set(SDL_INSTALL_TESTS OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(SDL3)
    
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm
        GIT_TAG        master
        SYSTEM
        EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(glm)

endif()

FetchContent_Declare(
    IAPlatformOps
    GIT_REPOSITORY https://github.com/I-A-S/IAPlatformOps
    GIT_TAG        main
    OVERRIDE_FIND_PACKAGE
)

FetchContent_Declare(
    IATest
    GIT_REPOSITORY https://github.com/I-A-S/IATest
    GIT_TAG        main
    OVERRIDE_FIND_PACKAGE
)

FetchContent_MakeAvailable(IAPlatformOps IATest)
