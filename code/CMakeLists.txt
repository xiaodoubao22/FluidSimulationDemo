cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)

# 项目
project(FluidSimulation)

# [third_party]设置三方库包含路径
set(THIRD_PARTY ${CMAKE_SOURCE_DIR}/../third_party)
set(GLAD_DIR ${THIRD_PARTY}/glad/include)
set(GLM_DIR ${THIRD_PARTY}/glm-master)
set(GLFW_DIR ${THIRD_PARTY}/glfw-3.3.8-x64-build/include)

# [Fluid2d]设置头文件路径
set(FLUID_2D_INC_DIR ${CMAKE_SOURCE_DIR}/Fluid2d/Inc)
set(FLUID_2D_INC_FILES
    ${FLUID_2D_INC_DIR}/Global.h
    ${FLUID_2D_INC_DIR}/ParticalSystem.h
    ${FLUID_2D_INC_DIR}/RenderWidget.h
    ${FLUID_2D_INC_DIR}/Shader.h
    ${FLUID_2D_INC_DIR}/Solver.h
    ${FLUID_2D_INC_DIR}/WCubicSpline2d.h
    )
# [Fluid2d]设置源文件路径
set(FLUID_2D_SRC_DIR ${CMAKE_SOURCE_DIR}/Fluid2d/Src)
set(FLUID_2D_SRC_FILES
    ${FLUID_2D_SRC_DIR}/ParticalSystem.cpp
    ${FLUID_2D_SRC_DIR}/RenderWidget.cpp
    ${FLUID_2D_SRC_DIR}/Shader.cpp
    ${FLUID_2D_SRC_DIR}/Solver.cpp
    ${FLUID_2D_SRC_DIR}/WCubicSpline2d.cpp
    )

# 包含目录
include_directories(${GLAD_DIR} ${GLM_DIR} ${GLFW_DIR})
include_directories(${FLUID_2D_INC_DIR})

# 生成exe
add_executable(${PROJECT_NAME} 
    Main.cpp 
    ${FLUID_2D_INC_FILES} 
    ${FLUID_2D_SRC_FILES}
    ${THIRD_PARTY}/glad/src/glad.c
    )
# [vs]设置启动项目
set_property (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

# [third_party]链接glfw静态库
target_link_libraries(${PROJECT_NAME} 
    ${CMAKE_SOURCE_DIR}/../third_party/glfw-3.3.8-x64-build/lib_vs2022_64_release/glfw3.lib
    )

