################################################################################
# Define project-wide locations
################################################################################
# Assets
set(Game_Asset_Dir ${CMAKE_SOURCE_DIR}/Assets/)
set(Game_Built_Data_Dir ${CMAKE_SOURCE_DIR}/Data/)

# Shaders
set(Game_Shader_Source_Dir ${CMAKE_SOURCE_DIR}/Source/Shaders/ )

# Build Data
set(Engine_Shader_Built_Data_Dir ${Game_Built_Data_Dir}/Engine/Shaders/ )
set(Game_Shader_Built_Data_Dir ${Game_Built_Data_Dir}/Shaders/ )

################################################################################
# Setup symlinks
################################################################################
add_custom_target(RefreshSymlinks ALL
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${Game_Built_Data_Dir} ${CMAKE_BINARY_DIR}/Data
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${Game_Project_File} ${CMAKE_BINARY_DIR}/${Game_Project_Name}.sj_proj

)