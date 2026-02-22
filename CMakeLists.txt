cmake_minimum_required(VERSION 3.15)

project(circuit-breaker)

include_directories(${PROJECT_SOURCE_DIR}/src)

# run this if using debug version
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# run this if using release
#set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")
file(GLOB_RECURSE SOURCES "src/*")
file(GLOB_RECURSE SNIPPETS "snippets/*")
add_executable(circuit-breaker ${SOURCES} ${SNIPPETS})

# PHYSX
# add folder 'phsyx' into project directory. Download from: https://drive.google.com/file/d/1pEJwnzxcU9dmLWXS2b9Mpow9S-b2PFo7/view?usp=sharing 

include_directories(${PROJECT_SOURCE_DIR}/dependencies/physx/include)
set(PHYSX_DIR ${PROJECT_SOURCE_DIR}/dependencies/physx)

# RapidJSON
# add folder 'rapidjson' into 'physx/include' project directory. Download from: https://drive.google.com/file/d/1-qeJBIDXejTcsveHHSgDjzXxQQXB2zJs/view?usp=sharing

# run this if using debug version
set(PHYSX_LIB ${PROJECT_SOURCE_DIR}/dependencies/physx/debug)

# run this if using release
#set(PHYSX_LIB ${PROJECT_SOURCE_DIR}/dependencies/physx/release)

# copy assets
file (COPY ${PROJECT_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})

#copy shaders
file (COPY ${PROJECT_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR})

# GLAD
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(dependencies/glad) 
set(LIBRARIES ${LIBRARIES} glad)

# ASSIMP
add_subdirectory(dependencies/assimp)
set(LIBRARIES ${LIBRARIES} assimp)

#free type
add_subdirectory(dependencies/freetype) 
set(LIBRARIES ${LIBRARIES} freetype)


# GLFW
add_subdirectory(dependencies/glfw-3.4)
set(LIBRARIES ${LIBRARIES} glfw)

#GLM
include_directories(SYSTEM dependencies/glm-0.9.9.7/)

# OPENGL
find_package(OpenGL REQUIRED)
set(LIBRARIES ${LIBRARIES} ${OPENGL_gl_LIBRARY})



if (WIN32)

  # OPENAL
  add_subdirectory(dependencies/openal)
  set(LIBRARIES ${LIBRARIES} OpenAL::OpenAL)

  # copy required dlls
  file(COPY 
    ${PHYSX_LIB}/PhysX_64.dll
    ${PHYSX_LIB}/PhysXCommon_64.dll
    ${PHYSX_LIB}/PhysXFoundation_64.dll
    ${PHYSX_LIB}/PhysXCooking_64.dll
    DESTINATION ${CMAKE_BINARY_DIR} )

  target_link_libraries(${PROJECT_NAME} PRIVATE 
    # physx
    ${PHYSX_LIB}/PhysXExtensions_static_64.lib
    ${PHYSX_LIB}/PhysX_64.lib
    ${PHYSX_LIB}/PhysXPvdSDK_static_64.lib
    ${PHYSX_LIB}/PhysXCommon_64.lib
    ${PHYSX_LIB}/PhysXFoundation_64.lib
    ${PHYSX_LIB}/PhysXCooking_64.lib
    ${PHYSX_LIB}/PhysXVehicle2_static_64.lib
    ${LIBRARIES}
  )

# copies openal32.dll after building openal and running the executable
 add_custom_command(TARGET circuit-breaker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:OpenAL::OpenAL> $<TARGET_FILE_DIR:circuit-breaker> 
    )

  elseif(UNIX)
    find_package(OpenAL REQUIRED)
    set(LIBRARIES ${LIBRARIES} OpenAL::OpenAL)
    target_link_libraries(${PROJECT_NAME} PRIVATE 
    # physx
    ${PHYSX_LIB}/libPhysXVehicle2_static_64.a
    ${PHYSX_LIB}/libPhysXCooking_static_64.a
    ${PHYSX_LIB}/libPhysXExtensions_static_64.a
    ${PHYSX_LIB}/libPhysX_static_64.a
    ${PHYSX_LIB}/libPhysXPvdSDK_static_64.a
    ${PHYSX_LIB}/libPhysXCommon_static_64.a
    ${PHYSX_LIB}/libPhysXFoundation_static_64.a
    ${LIBRARIES}
    # target_link_libraries(${PROJECT_NAME} PRIVATE 
    # # physx
    # ${PHYSX_LIB}/libPhysXExtensions_static_64.a
    # ${PHYSX_LIB}/libPhysX_static_64.a
    # ${PHYSX_LIB}/libPhysXPvdSDK_static_64.a
    # ${PHYSX_LIB}/libPhysXCommon_static_64.a
    # ${PHYSX_LIB}/libPhysXFoundation_static_64.a
    # ${PHYSX_LIB}/libPhysXCooking_static_64.a
    # ${PHYSX_LIB}/libPhysXVehicle2_static_64.a
    # ${LIBRARIES}
  )


endif()


# Copy all the shaders and tell the build system to re-run CMAKE if one of them changes
file(GLOB files shaders/*)
foreach(file ${files})
	get_filename_component(name ${file} NAME)
	configure_file(${file} shaders/${name})
endforeach()


