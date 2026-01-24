cmake_minimum_required(VERSION 3.15)

project(zip-zap-zop)

# run this if using debug version
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# run this if using release
# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")

# add_executable(zip-zap-zop main.cpp)
# add_executable(zip-zap-zop main.cpp )
add_executable(zip-zap-zop main.cpp "Shader.h" "Shader.cpp" "ShaderProgram.cpp" "ShaderProgram.h" "RenderingSystem.h" "RenderingSystem.cpp")


# PHYSX
# add folder 'phsyx' into project directory. Download from: https://drive.google.com/file/d/1pEJwnzxcU9dmLWXS2b9Mpow9S-b2PFo7/view?usp=sharing 

include_directories(${PROJECT_SOURCE_DIR}/dependencies/physx/include)
set(PHYSX_DIR ${PROJECT_SOURCE_DIR}/dependencies/physx)

# run this if using debug version
set(PHYSX_LIB ${PROJECT_SOURCE_DIR}/dependencies/physx/debug)

# run this if using release
# set(PHYSX_LIB ${PROJECT_SOURCE_DIR}/physx/release)

# copy required dlls (this shouldn't actually be required for linux)
# file(COPY 
# 	${PHYSX_LIB}/libPhysX_static_64.a
# 	${PHYSX_LIB}/libPhysXCommon_static_64.a
# 	${PHYSX_LIB}/libPhysXFoundation_static_64.a
# 	DESTINATION ${CMAKE_BINARY_DIR} )

#free type
add_subdirectory(dependencies/freetype) 
set(LIBRARIES ${LIBRARIES} freetype)

include_directories(${PROJECT_SOURCE_DIR}/dependencies/glad/include)
include_directories(${PROJECT_SOURCE_DIR}/dependencies/glfw-3.4/include)
# GLAD
add_subdirectory(dependencies/glad) 
set(LIBRARIES ${LIBRARIES} glad)

# GLFW
add_subdirectory(dependencies/glfw-3.4)
set(LIBRARIES ${LIBRARIES} glfw)

#GLM
include_directories(SYSTEM dependencies/glm-0.9.9.7/)

# OPENGL
find_package(OpenGL REQUIRED)
set(LIBRARIES ${LIBRARIES} ${OPENGL_gl_LIBRARY})

target_link_libraries(${PROJECT_NAME} PRIVATE 
  # physx
  ${PHYSX_LIB}/libPhysXExtensions_static_64.a
  ${PHYSX_LIB}/libPhysX_static_64.a
  ${PHYSX_LIB}/libPhysXPvdSDK_static_64.a
  ${PHYSX_LIB}/libPhysXCommon_static_64.a
  ${PHYSX_LIB}/libPhysXFoundation_static_64.a
  ${LIBRARIES}

)

# Definition for shader directory folder
add_compile_definitions(SHADER_DIR="${CMAKE_SOURCE_DIR}/shaders")
