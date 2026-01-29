# CMake entry point
cmake_minimum_required (VERSION 3.0)
project (TP_Moteur_Base)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)


if( CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR )
    message( FATAL_ERROR "Please select another Build Directory ! (and give it a clever name, like bin_Visual2012_64bits/)" )
endif()
if( CMAKE_SOURCE_DIR MATCHES " " )
	message( "Your Source Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()
if( CMAKE_BINARY_DIR MATCHES " " )
	message( "Your Build Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()



# Compile external dependencies 
add_subdirectory (external)

# On Visual 2005 and above, this module can set the debug working directory
cmake_policy(SET CMP0026 OLD)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/external/rpavlik-cmake-modules-fe2273")
include(CreateLaunchers)
include(MSVCMultipleProcessCompile) # /MP

if(INCLUDE_DISTRIB)
	add_subdirectory(distrib)
endif(INCLUDE_DISTRIB)



include_directories(
		external/glfw-3.1.2/include/
		external/glm-0.9.7.1/
		external/glew-1.13.0/include/
		external/stb_image
		external/assimp-5.2.5/include
		${CMAKE_BINARY_DIR}/external/assimp-5.2.5/include/
	.
)

set(ALL_LIBS
	${OPENGL_LIBRARY}
	glfw
	GLEW_1130
		assimp
)

add_definitions(
	-DTW_STATIC
	-DTW_NO_LIB_PRAGMA
	-DTW_NO_DIRECT3D
	-DGLEW_STATIC
	-D_CRT_SECURE_NO_WARNINGS
)

# TP1
add_executable(TP_Moteur_Base
		src/main.cpp
		common/shader.cpp
		common/shader.hpp
		common/controls.cpp
		common/controls.hpp
		common/texture.cpp
		common/texture.hpp
		common/objloader.cpp
		common/objloader.hpp
		common/vboindexer.cpp
		common/vboindexer.hpp

		external/stb_image/stb_image.cpp

		resources/shaders/vertex_shader.glsl
		resources/shaders/fragment_shader.glsl
		common/camera.hpp
		common/camera.cpp
		common/Time.hpp
		common/Time.cpp
		common/model.hpp
		common/mesh.hpp
		common/mesh.cpp
		common/model.cpp
)
target_link_libraries(TP_Moteur_Base
	${ALL_LIBS}
)
# Xcode and Visual working directories
set_target_properties(TP_Moteur_Base PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/")
create_target_launcher(TP_Moteur_Base WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/")




SOURCE_GROUP(common REGULAR_EXPRESSION ".*/common/.*" )
SOURCE_GROUP(shaders REGULAR_EXPRESSION ".*/.*shader$" )


if (NOT ${CMAKE_GENERATOR} MATCHES "Xcode" )
add_custom_command(
   TARGET TP_Moteur_Base POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/TP_Moteur_Base${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_CURRENT_SOURCE_DIR}/"
)
elseif (${CMAKE_GENERATOR} MATCHES "Xcode" )

endif (NOT ${CMAKE_GENERATOR} MATCHES "Xcode" )

