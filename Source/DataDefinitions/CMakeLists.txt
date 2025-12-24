################################################################################
# Define project: Data Definitions
################################################################################
project(ScrewjankDataDefs)

file(GLOB_RECURSE DATA_DEFINITIONS_HEADERS CONFIGURE_DEPENDS "*.hpp")
file(GLOB_RECURSE DATA_DEFINITIONS_INLINE CONFIGURE_DEPENDS "*.inl")
file(GLOB_RECURSE DATA_DEFINITIONS_SOURCE CONFIGURE_DEPENDS "*.cpp")
file(GLOB_RECURSE DATA_DEFINITIONS_MODULES CONFIGURE_DEPENDS "*.cppm")

add_library(${PROJECT_NAME} ${DATA_DEFINITIONS_HEADERS} ${DATA_DEFINITIONS_INLINE} ${DATA_DEFINITIONS_SOURCE} )
target_sources(${PROJECT_NAME}
    PUBLIC
      FILE_SET CXX_MODULES FILES
        ${DATA_DEFINITIONS_MODULES}
)

################################################################################
# Link dependencies
################################################################################
target_link_libraries(${PROJECT_NAME} 
    PUBLIC 
        ScrewjankStd
        glaze::glaze
)

################################################################################
# Configure include directories
################################################################################
target_include_directories(${PROJECT_NAME} 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )