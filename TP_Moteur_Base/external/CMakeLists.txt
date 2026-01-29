if(MSVC AND NOT "${MSVC_VERSION}" LESS 1400)
	add_definitions( "/MP" )
endif()


add_definitions(
	-DTW_STATIC
	-DTW_NO_LIB_PRAGMA
	-DTW_NO_DIRECT3D
	-DGLEW_STATIC
	-D_CRT_SECURE_NO_WARNINGS
)

### GLFW ###
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)

add_subdirectory (glfw-3.1.2)
add_subdirectory (assimp-5.2.5)

if(TARGET assimp)
	target_compile_options(assimp PRIVATE -Wno-deprecated-declarations -Wno-error)
endif()

# 1. 设置 Assimp 优化选项 (必须在 add_subdirectory 之前设置)
# 关闭单元测试 (极大地加快编译速度)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
# 关闭命令行工具
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
# 关闭导出功能 (我们只需要读取模型，不需要保存)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
# 编译为静态库 (避免 .dll/.dylib 路径问题)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# 强制开启编译自带 ZLIB，防止 Windows 上报 "zlib not found" 错误
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)

set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)



include_directories(
	glfw-3.1.2/include/GLFW/
	glew-1.13.0/include/
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
set(OPENGL_LIBRARY
	${OPENGL_LIBRARY}
	-lGL -lGLU -lXrandr -lXext -lX11 -lrt
	${CMAKE_DL_LIBS}
	${GLFW_LIBRARIES}
)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
set(OPENGL_LIBRARY
	${OPENGL_LIBRARY}
	${CMAKE_DL_LIBS}
	${GLFW_LIBRARIES}
)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

### GLEW ###

set(GLEW_SOURCE
	glew-1.13.0/src/glew.c
)

set(GLEW_HEADERS
)


add_library( GLEW_1130 STATIC
	${GLEW_SOURCE}
	${GLEW_INCLUDE}
)

target_link_libraries(GLEW_1130
	${OPENGL_LIBRARY}
	${EXTRA_LIBS}
)


