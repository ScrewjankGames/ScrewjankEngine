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
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED YES
        ARCHIVE_OUTPUT_DIRECTORY "${SJ_ARCHIVE_OUTPUT_DIRECTORY}/Shared/"
        LIBRARY_OUTPUT_DIRECTORY "${SJ_LIBRARY_OUTPUT_DIRECTORY}/Shared/"
        RUNTIME_OUTPUT_DIRECTORY "${SJ_RUNTIME_OUTPUT_DIRECTORY}/Shared/"
)

################################################################################
# Link dependencies
################################################################################
target_link_libraries(${PROJECT_NAME} 
    PUBLIC 
        ScrewjankSharedCore
        ScrewjankMath
)

################################################################################
# Configure include directories
################################################################################
target_include_directories(${PROJECT_NAME} 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )