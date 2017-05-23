project(opengl_play)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_VERBOSE_MAKEFILE 1)

SET(CMAKE_CXX_FLAGS_RELEASE "-O2")
SET(CMAKE_CXX_FLAGS_DEBUG  "-O0 -ggdb")

#########################################################
# C++1y
#########################################################

#SET(COMPILER_OPT_LVL "-O2")

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++1z" COMPILER_SUPPORTS_CXX1Z)
CHECK_CXX_COMPILER_FLAG("-std=c++1y" COMPILER_SUPPORTS_CXX1Y)

if(COMPILER_SUPPORTS_CXX1Z)
    set(CPP_VER_FLAG "-std=c++1z")
elseif(COMPILER_SUPPORTS_CXX1Y)
    set(CPP_VER_FLAG "-std=c++1z")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

set(CMAKE_CXX_FLAGS "${CPP_VER_FLAG} ${COMPILER_OPT_LVL}")

#########################################################
# PTHREAD
#########################################################

find_package(Threads REQUIRED)

#########################################################
# FIND GLUT
#########################################################
find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIRS})
link_directories(${GLUT_LIBRARY_DIRS})
add_definitions(${GLUT_DEFINITIONS})
if(NOT GLUT_FOUND)
    message(ERROR " GLUT not found!")
endif(NOT GLUT_FOUND)

#########################################################
# FIND GLEW
#########################################################

find_package(GLEW REQUIRED)
if (GLEW_FOUND)
    include_directories(${GLEW_INCLUDE_DIRS})
    link_libraries(${GLEW_LIBRARIES})
endif()

#########################################################
# FIND OPENGL
#########################################################
find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})
link_directories(${OpenGL_LIBRARY_DIRS})
add_definitions(${OpenGL_DEFINITIONS})

if(NOT OPENGL_FOUND)
    message(ERROR " OPENGL not found!")
endif(NOT OPENGL_FOUND)

if(NOT OPENGL_GLU_FOUND)
    message(ERROR " GLU not found!")
endif(NOT OPENGL_GLU_FOUND)

#########################################################
# FIND GLFW
#########################################################

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory("./glfw/")


#########################################################
# FIND FREETYPE
#########################################################
find_package(Freetype REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIRS})
link_directories(${FREETYPE_LIBRARIES})

#########################################################
# FIND ASSIMP
#########################################################

find_package(ASSIMP REQUIRED)
if(ASSIMP_FOUND)
    include_directories(${ASSIMP_INCLUDE_DIR})
    link_directories( ${ASSIMP_LIBRARIES} )
    message(STATUS "Found asset importer library: ${ASSIMP_LIBRARIES}")
else()
    message(ERROR " ASSIMP not found!")
endif()

#########################################################
# Remaining stuff
#########################################################

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/")

file(GLOB SRC_LIST
    "*.hpp"
    "*.cpp"
    "./logger/*.hpp"
    "./logger/*.cpp"
)

add_executable(${PROJECT_NAME} ${SRC_LIST})

target_link_libraries(
    ${PROJECT_NAME}
    ${CMAKE_THREAD_LIBS_INIT}
    ${GLUT_LIBRARIES}
    ${OPENGL_glu_LIBRARY}
    ${OPENGL_gl_LIBRARY}
    ${GLEW_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    ${GLM_LIBRARY_DIRS}
    ${ASSIMP_LIBRARIES}
    glfw
    SOIL)



