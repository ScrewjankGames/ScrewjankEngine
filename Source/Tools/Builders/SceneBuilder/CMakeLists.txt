################################################################################
# Define project: Texture Builder
################################################################################
project(SceneBuilder)

file(GLOB_RECURSE BUILDER_HEADERS CONFIGURE_DEPENDS "*.hpp")
file(GLOB_RECURSE BUILDER_INLINES CONFIGURE_DEPENDS "*.inl")
file(GLOB_RECURSE BUILDER_SOURCE CONFIGURE_DEPENDS "*.cpp")

add_executable(${PROJECT_NAME} ${BUILDER_HEADERS} ${BUILDER_INLINES} ${BUILDER_SOURCE})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${BUILDER_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${BUILDER_INLINES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${BUILDER_SOURCE})

set_target_properties(${PROJECT_NAME} 
    PROPERTIES 
        FOLDER Tools/Builders
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED YES
        ARCHIVE_OUTPUT_DIRECTORY "${Binaries_Directory}/Tools/Builders/${PROJECT_NAME}"
        RUNTIME_OUTPUT_DIRECTORY "${Binaries_Directory}/Tools/Builders/${PROJECT_NAME}"
)

################################################################################
# Link dependencies
################################################################################
target_link_libraries(${PROJECT_NAME} 
    PUBLIC 
        ScrewjankDataDefs
        ScrewjankSharedCore
        nlohmann_json
)

################################################################################
# Configure include directories
################################################################################
target_include_directories(${PROJECT_NAME} 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)