################################################################################
# Define project: Model Builder
################################################################################
project(ScrewjankEditor)

file(GLOB_RECURSE Editor_Headers CONFIGURE_DEPENDS "*.hpp")
file(GLOB_RECURSE Editor_Source CONFIGURE_DEPENDS "*.cpp")
file(GLOB_RECURSE Editor_Modules CONFIGURE_DEPENDS "*.cppm")

add_library(${PROJECT_NAME} ${Editor_Headers} ${Editor_Source})
target_sources(${PROJECT_NAME}
    PUBLIC
      FILE_SET CXX_MODULES FILES
        ${Editor_Modules}
)
################################################################################
# Link dependencies
################################################################################
target_link_libraries(${PROJECT_NAME} 
    PUBLIC 
        ScrewjankEngine
)