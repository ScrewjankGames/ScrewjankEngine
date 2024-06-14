################################################################################
# Define project: Data Definitions
################################################################################
project(DataDefinitions)

file(GLOB_RECURSE DATA_DEFINITIONS_HEADERS CONFIGURE_DEPENDS "*.hpp")

add_library(${PROJECT_NAME} INTERFACE ${DATA_DEFINITIONS_HEADERS})

set_target_properties(${PROJECT_NAME} 
    PROPERTIES 
        FOLDER Shared/
)

################################################################################
# Link dependencies
################################################################################

################################################################################
# Configure include directories
################################################################################
target_include_directories(${PROJECT_NAME} 
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )