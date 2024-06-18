################################################################################
# Define project: Data Definitions
################################################################################
project(ScrewjankDataDefs)

file(GLOB_RECURSE DATA_DEFINITIONS_HEADERS CONFIGURE_DEPENDS "*.hpp")
file(GLOB_RECURSE DATA_DEFINITIONS_INLINE CONFIGURE_DEPENDS "*.inl")
file(GLOB_RECURSE DATA_DEFINITIONS_SOURCE CONFIGURE_DEPENDS "*.cpp")



add_library(${PROJECT_NAME} ${DATA_DEFINITIONS_HEADERS} ${DATA_DEFINITIONS_INLINE} ${DATA_DEFINITIONS_SOURCE} )

set_target_properties(${PROJECT_NAME} 
    PROPERTIES 
        FOLDER Shared/
)

################################################################################
# Link dependencies
################################################################################
target_link_libraries(${PROJECT_NAME} 
    PUBLIC 
        STB 
        ScrewjankSharedCore
)

################################################################################
# Configure include directories
################################################################################
target_include_directories(${PROJECT_NAME} 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )