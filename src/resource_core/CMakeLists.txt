add_library(resource_core STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/resource_error.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/file_handle.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/file_io.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/resource_manager.cpp
)

target_link_libraries(resource_core PRIVATE project_warnings)
if (TARGET project_sanitizers)
        target_link_libraries(resource_core PRIVATE project_sanitizers)
endif ()

target_include_directories(resource_core
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
