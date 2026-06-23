################################################################################
# Define project: ScrewjankEngine
################################################################################
project(ScrewjankEngine)

file(GLOB_RECURSE RUNTIME_HEADERS CONFIGURE_DEPENDS "*.hpp")
file(GLOB_RECURSE RUNTIME_INLINES CONFIGURE_DEPENDS "*.inl")
file(GLOB_RECURSE RUNTIME_SOURCE CONFIGURE_DEPENDS "*.cpp")
file(GLOB_RECURSE RUNTIME_MODULES CONFIGURE_DEPENDS "*.cppm")

add_library(${PROJECT_NAME} ${RUNTIME_HEADERS} ${RUNTIME_INLINES} ${RUNTIME_SOURCE})
target_sources(${PROJECT_NAME}
    PUBLIC
      FILE_SET CXX_MODULES FILES
        ${RUNTIME_MODULES}
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${RUNTIME_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${RUNTIME_INLINES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${RUNTIME_SOURCE})

### For Single-config generators
if(SJ_GOLD_OPT)
target_compile_definitions(${PROJECT_NAME} PUBLIC SJ_GOLD)
endif()

################################################################################
# Link dependencies
################################################################################
target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
        SDL3::SDL3
    PUBLIC
        ScrewjankStd
        ScrewjankDataDefs
        spdlog::spdlog
        imgui
        implot
        glaze::glaze
)

################################################################################
# Configure include directories
################################################################################
target_include_directories(${PROJECT_NAME} 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${ENGINE_DEPENDENCIES_DIR}/imgui/
    )