cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)

project(FluidSimulation)

# [third_party]设置三方库包含路径
set(THIRD_PARTY ${CMAKE_SOURCE_DIR}/../third_party)
set(GLAD_DIR ${THIRD_PARTY}/glad/include)
set(GLM_DIR ${THIRD_PARTY}/glm)
set(GLFW_DIR ${THIRD_PARTY}/glfw-3.3.8.bin.WIN64)
set(STB_DIR ${THIRD_PARTY}/stb)

# [common]
set(COMMON_INC_DIR ${CMAKE_SOURCE_DIR}/Common/Inc)
file(GLOB COMMON_INC_FILES ${COMMON_INC_DIR}/*.h)
file(GLOB COMMON_SRC_FILES ${CMAKE_SOURCE_DIR}/Common/Src/*.cpp)

# [Fluid2d]
set(FLUID_2D_INC_DIR ${CMAKE_SOURCE_DIR}/Fluid2d/Inc)
file(GLOB FLUID_2D_INC_FILES ${FLUID_2D_INC_DIR}/*.h)
file(GLOB FLUID_2D_SRC_FILES
    ${CMAKE_SOURCE_DIR}/Fluid2d/Src/*.cpp
    ${CMAKE_SOURCE_DIR}/Fluid2d/Main2d.cpp
)
file(GLOB FLUID_2D_SHADER_FILES
    ${CMAKE_SOURCE_DIR}/Fluid2d/*.vert
    ${CMAKE_SOURCE_DIR}/Fluid2d/*.frag
    ${CMAKE_SOURCE_DIR}/Fluid2d/*.geom
)

# [Fluid3d]
set(FLUID_3D_INC_DIR ${CMAKE_SOURCE_DIR}/Fluid3d/Inc)
file(GLOB FLUID_3D_INC_FILES ${FLUID_3D_INC_DIR}/*.h)
file(GLOB FLUID_3D_SRC_FILES
    ${CMAKE_SOURCE_DIR}/Fluid3d/Src/*.cpp
    ${CMAKE_SOURCE_DIR}/Fluid3d/Main3d.cpp
)
file(GLOB FLUID_2D_SHADER_FILES
    ${CMAKE_SOURCE_DIR}/Fluid3d/Shaders/*.vert
    ${CMAKE_SOURCE_DIR}/Fluid3d/Shaders/*.frag
    ${CMAKE_SOURCE_DIR}/Fluid3d/Shaders/*.geom
    ${CMAKE_SOURCE_DIR}/Fluid3d/Shaders/*.comp
)

# [FrameGeneration]
set(FRAME_GENERATION_INC_DIR ${CMAKE_SOURCE_DIR}/FrameGeneration/Framework/Inc)
file(GLOB FRAME_GENERATION_INC_FILES ${FRAME_GENERATION_INC_DIR}/*.h)
file(GLOB FRAME_GENERATION_SRC_FILES ${CMAKE_SOURCE_DIR}/FrameGeneration/Framework/Src/*.cpp)

# 全局包含目录
include_directories(${GLAD_DIR} ${GLM_DIR} ${GLFW_DIR}/include ${STB_DIR} ${COMMON_INC_DIR})

# -----------2D项目-----------
set(PROJECT_2D FluidSimulation2D)
# 生成exe
add_executable(${PROJECT_2D}
    ${COMMON_INC_FILES} 
    ${COMMON_SRC_FILES}
    ${FLUID_2D_INC_FILES} 
    ${FLUID_2D_SRC_FILES}
    ${FLUID_2D_SHADER_FILES}
    ${THIRD_PARTY}/glad/src/glad.c
    )
# 包含目录
target_include_directories(${PROJECT_2D} PUBLIC ${FLUID_2D_INC_DIR})
# [third_party]链接glfw静态库
target_link_libraries(${PROJECT_2D} 
    ${GLFW_DIR}/lib-vc2022/glfw3.lib
    )

# -----------3D项目-----------
set(PROJECT_3D FluidSimulation3D)
# 生成exe
add_executable(${PROJECT_3D} 
    ${COMMON_INC_FILES} 
    ${COMMON_SRC_FILES}
    ${FLUID_3D_INC_FILES} 
    ${FLUID_3D_SRC_FILES}
    ${THIRD_PARTY}/glad/src/glad.c
    ${FLUID_3D_SHADER_FILES}
    )
# 包含目录
target_include_directories(${PROJECT_3D} PUBLIC ${FLUID_3D_INC_DIR})
# [third_party]链接glfw静态库
target_link_libraries(${PROJECT_3D} 
    ${GLFW_DIR}/lib-vc2022/glfw3.lib
    )

# -----------FrameGeneration项目-----------
set(PROJECT_FG FrameGeneration)
add_executable(${PROJECT_FG}
    ${COMMON_SRC_FILES}
    ${FRAME_GENERATION_SRC_FILES}
    ${CMAKE_SOURCE_DIR}/FrameGeneration/MainFG.cpp
    ${THIRD_PARTY}/glad/src/glad.c
    )
target_include_directories(${PROJECT_FG} PUBLIC ${FRAME_GENERATION_INC_DIR})
target_link_libraries(${PROJECT_FG}
    ${GLFW_DIR}/lib-vc2022/glfw3.lib
    )


# [vs]设置显示层级
set(ALL_FILES
    ${COMMON_INC_FILES} 
    ${COMMON_SRC_FILES}
    ${FLUID_3D_INC_FILES} 
    ${FLUID_3D_SRC_FILES}
    ${FLUID_2D_INC_FILES} 
    ${FLUID_2D_SRC_FILES}
    ${FLUID_2D_SHADER_FILES}
    ${FLUID_3D_SHADER_FILES}
    ${FRAME_GENERATION_SRC_FILES}
    ${FRAME_GENERATION_INC_FILES}
    ${CMAKE_SOURCE_DIR}/FrameGeneration/MainFG.cpp
    ${FW_SRC_FILES}
    )
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${ALL_FILES})

# [vs]设置启动项目
set_property (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_FG})

