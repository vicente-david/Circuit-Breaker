cmake_minimum_required(VERSION 3.15)

project(zip-zap-zop)

# run this if using debug version
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# run this if using release
# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")

add_executable(zip-zap-zop main.cpp)


# PHYSX
# add folder 'phsyx' into project directory. Download from: https://drive.google.com/file/d/1pEJwnzxcU9dmLWXS2b9Mpow9S-b2PFo7/view?usp=sharing 

include_directories(${PROJECT_SOURCE_DIR}/dependencies/physx/include)
set(PHYSX_DIR ${PROJECT_SOURCE_DIR}/dependencies/physx)

# run this if using debug version
set(PHYSX_LIB ${PROJECT_SOURCE_DIR}/dependencies/physx/debug)

# run this if using release
# set(PHYSX_LIB ${PROJECT_SOURCE_DIR}/physx/release)

# copy required dlls
file(COPY 
	${PHYSX_LIB}/libPhysX_static_64.a
	${PHYSX_LIB}/libPhysXCommon_static_64.a
	${PHYSX_LIB}/libPhysXFoundation_static_64.a
	DESTINATION ${CMAKE_BINARY_DIR} )

target_link_libraries(${PROJECT_NAME} PRIVATE 
  # physx
  ${PHYSX_LIB}/libPhysXExtensions_static_64.a
  ${PHYSX_LIB}/libPhysX_static_64.a
  ${PHYSX_LIB}/libPhysXPvdSDK_static_64.a
  ${PHYSX_LIB}/libPhysXCommon_static_64.a
  ${PHYSX_LIB}/libPhysXFoundation_static_64.a
)
