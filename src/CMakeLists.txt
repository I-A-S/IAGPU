
add_subdirectory(gpu/)

if(IAGPU_ENABLE_PIPELINE_BAKER)
    add_subdirectory(pipeline_baker/)
endif()