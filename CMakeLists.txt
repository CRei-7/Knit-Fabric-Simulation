cmake_minimum_required(VERSION 3.0.0)
project(physics_simulation_software VERSION 0.1.0)

find_package(OpenGL REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/include)


# Add source files
file(GLOB_RECURSE SOURCE_FILES 
	${CMAKE_SOURCE_DIR}/src/*.c
	${CMAKE_SOURCE_DIR}/src/*.cpp)

# add executable
add_executable(physics_simulation_software ${SOURCE_FILES})

target_link_libraries(physics_simulation_software 
    glfw
    OpenGL::GL
)

# # Link libraries
# if(WIN32)
#     target_link_libraries(${PROJECT_NAME} 
#         ${CMAKE_SOURCE_DIR}/lib/libglfw3dll.a
#         opengl32
#     )
# else()
#     find_package(OpenGL REQUIRED)
#     find_library(GLFW_LIBRARY glfw)
#     target_link_libraries(${PROJECT_NAME} 
#         ${OPENGL_LIBRARIES}
#         ${GLFW_LIBRARY}
#         dl
#     )
# endif()

# # Copy DLL files to build directory on Windows
# if(WIN32)
#     add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#         COMMAND ${CMAKE_COMMAND} -E copy_if_different
#         "${CMAKE_SOURCE_DIR}/lib/glfw3.dll"
#         $<TARGET_FILE_DIR:${PROJECT_NAME}>
#     )
# endif()