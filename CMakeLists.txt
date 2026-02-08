cmake_minimum_required(VERSION 3.15)

project(circuit-breaker)


# note: this release version only works if in your cmakesettings.json your release build
# is specifically Release
# and not RelWithDebInfo (which visual studio defaults to)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:>")

# run this if using release
#set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  message("in debug mode rn")
  # run this if using debug version
  set(PHYSX_LIB ${PROJECT_SOURCE_DIR}/dependencies/physx/debug)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  message("in release mode rn")
  #set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
  # run this if using release
  set(PHYSX_LIB ${PROJECT_SOURCE_DIR}/dependencies/physx/release)
endif()

file(GLOB_RECURSE SOURCES "src/*")
add_executable(circuit-breaker ${SOURCES})

# PHYSX
# add folder 'phsyx' into project directory. Download from: https://drive.google.com/file/d/1pEJwnzxcU9dmLWXS2b9Mpow9S-b2PFo7/view?usp=sharing 
include_directories(${PROJECT_SOURCE_DIR}/dependencies/physx/include)
set(PHYSX_DIR ${PROJECT_SOURCE_DIR}/dependencies/physx)


# copy assets
file (COPY ${PROJECT_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})

#copy shaders
file (COPY ${PROJECT_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR})

#free type
add_subdirectory(dependencies/freetype) 
set(LIBRARIES ${LIBRARIES} freetype)

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




# run program once, on fail, copy openal32.dll to to executable directory
# fits like a glove

if (WIN32)

  # OPENAL
  add_subdirectory(dependencies/openal)
  set(LIBRARIES ${LIBRARIES} OpenAL::OpenAL)

  # copy required dlls
  file(COPY 
    ${PHYSX_LIB}/PhysX_64.dll
    ${PHYSX_LIB}/PhysXCommon_64.dll
    ${PHYSX_LIB}/PhysXFoundation_64.dll
    DESTINATION ${CMAKE_BINARY_DIR} )

  target_link_libraries(${PROJECT_NAME} PRIVATE 
    # physx
    ${PHYSX_LIB}/PhysXExtensions_static_64.lib
    ${PHYSX_LIB}/PhysX_64.lib
    ${PHYSX_LIB}/PhysXPvdSDK_static_64.lib
    ${PHYSX_LIB}/PhysXCommon_64.lib
    ${PHYSX_LIB}/PhysXFoundation_64.lib
    ${LIBRARIES}
  )

 add_custom_command(TARGET circuit-breaker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:OpenAL::OpenAL> $<TARGET_FILE_DIR:circuit-breaker> 
    )

  elseif(UNIX)
    target_link_libraries(${PROJECT_NAME} PRIVATE 
    # physx
    ${PHYSX_LIB}/libPhysXExtensions_static_64.a
    ${PHYSX_LIB}/libPhysX_static_64.a
    ${PHYSX_LIB}/libPhysXPvdSDK_static_64.a
    ${PHYSX_LIB}/libPhysXCommon_static_64.a
    ${PHYSX_LIB}/libPhysXFoundation_static_64.a
    ${LIBRARIES}

  )

endif()


# Copy all the shaders and tell the build system to re-run CMAKE if one of them changes
file(GLOB files shaders/*)
foreach(file ${files})
	get_filename_component(name ${file} NAME)
	configure_file(${file} shaders/${name})
endforeach()


