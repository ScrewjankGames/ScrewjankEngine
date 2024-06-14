find_package(Vulkan REQUIRED COMPONENTS glslc)
find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)

#==============================================================================
# COMPILE SHADERS
#

set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)

file(GLOB EngineShaders
	${Engine_Shader_Source_Dir}/*.vert
	${Engine_Shader_Source_Dir}/*.frag
	${Engine_Shader_Source_Dir}/*.comp
	${Engine_Shader_Source_Dir}/*.geom
	${Engine_Shader_Source_Dir}/*.tesc
	${Engine_Shader_Source_Dir}/*.tese
	${Engine_Shader_Source_Dir}/*.mesh
	${Engine_Shader_Source_Dir}/*.task
	${Engine_Shader_Source_Dir}/*.rgen
	${Engine_Shader_Source_Dir}/*.rchit
	${Engine_Shader_Source_Dir}/*.rmiss
	)

file(GLOB GameShaders
    ${Game_Shader_Source_Dir}/*.vert
    ${Game_Shader_Source_Dir}/*.frag
    ${Game_Shader_Source_Dir}/*.comp
    ${Game_Shader_Source_Dir}/*.geom
    ${Game_Shader_Source_Dir}/*.tesc
    ${Game_Shader_Source_Dir}/*.tese
    ${Game_Shader_Source_Dir}/*.mesh
    ${Game_Shader_Source_Dir}/*.task
    ${Game_Shader_Source_Dir}/*.rgen
    ${Game_Shader_Source_Dir}/*.rchit
    ${Game_Shader_Source_Dir}/*.rmiss
    )

add_custom_command(
	COMMAND
		${CMAKE_COMMAND} -E make_directory ${Game_Shader_Built_Data_Dir}
	OUTPUT ${Game_Shader_Built_Data_Dir}
	COMMENT "Creating ${Game_Shader_Built_Data_Dir}"
)

add_custom_command(
	COMMAND
		${CMAKE_COMMAND} -E make_directory ${Engine_Shader_Built_Data_Dir}
	OUTPUT ${Engine_Shader_Built_Data_Dir}
	COMMENT "Creating ${Engine_Shader_Built_Data_Dir}"
)

## fucking why can't I make cmake functions work
foreach(source IN LISTS EngineShaders)
    get_filename_component(FILENAME ${source} NAME)
    add_custom_command(
        COMMAND
            ${glslc_executable} ${source} -o ${Engine_Shader_Built_Data_Dir}/${FILENAME}.spv
        OUTPUT 
            ${Engine_Shader_Built_Data_Dir}/${FILENAME}.spv
        DEPENDS 
            ${source}
        COMMENT 
            "Compiling ${FILENAME}"
    )
    list(APPEND SPV_SHADERS ${Engine_Shader_Built_Data_Dir}/${FILENAME}.spv)
endforeach()

foreach(source IN LISTS GameShaders)
    get_filename_component(FILENAME ${source} NAME)
    add_custom_command(
        COMMAND
            ${glslc_executable} ${source} -o ${Game_Shader_Built_Data_Dir}/${FILENAME}.spv
        OUTPUT 
            ${Game_Shader_Built_Data_Dir}/${FILENAME}.spv
        DEPENDS 
            ${source}
        COMMENT 
            "Compiling ${FILENAME}"
    )
    list(APPEND SPV_SHADERS ${Game_Shader_Built_Data_Dir}/${FILENAME}.spv)
endforeach()

add_custom_target(Shaders ALL SOURCES ${EngineShaders} ${GameShaders} DEPENDS ${SPV_SHADERS})