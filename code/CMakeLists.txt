cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)

project(FluidSimulation)

# [third_party]设置三方库包含路径
set(THIRD_PARTY ${CMAKE_SOURCE_DIR}/../third_party)
set(GLAD_DIR ${THIRD_PARTY}/glad/include)
set(GLM_DIR ${THIRD_PARTY}/glm-master)
set(GLFW_DIR ${THIRD_PARTY}/glfw-3.3.8-x64-build/include)
set(STB_DIR ${THIRD_PARTY}/stb)

# [common]设置头文件路径
set(COMMON_INC_DIR ${CMAKE_SOURCE_DIR}/Common/Inc)
set(COMMON_INC_FILES
    ${COMMON_INC_DIR}/ComputeShader.h
    ${COMMON_INC_DIR}/Shader.h
    ${COMMON_INC_DIR}/WCubicSpline.h
    ${COMMON_INC_DIR}/Global.h
    
    )
# [common]设置源文件路径
set(COMMON_SRC_DIR ${CMAKE_SOURCE_DIR}/Common/Src)
set(COMMON_SRC_FILES
    ${COMMON_SRC_DIR}/ComputeShader.cpp
    ${COMMON_SRC_DIR}/Shader.cpp
    ${COMMON_SRC_DIR}/WCubicSpline.cpp
    )

# [Fluid2d]设置头文件路径
set(FLUID_2D_INC_DIR ${CMAKE_SOURCE_DIR}/Fluid2d/Inc)
set(FLUID_2D_INC_FILES
    ${FLUID_2D_INC_DIR}/Parameters.h
    ${FLUID_2D_INC_DIR}/ParticalSystem.h
    ${FLUID_2D_INC_DIR}/RenderWidget.h
    ${FLUID_2D_INC_DIR}/Solver.h
    )
# [Fluid2d]设置源文件路径
set(FLUID_2D_SRC_DIR ${CMAKE_SOURCE_DIR}/Fluid2d/Src)
set(FLUID_2D_SRC_FILES
    ${FLUID_2D_SRC_DIR}/ParticalSystem.cpp
    ${FLUID_2D_SRC_DIR}/RenderWidget.cpp
    ${FLUID_2D_SRC_DIR}/Solver.cpp
    ${CMAKE_SOURCE_DIR}/Fluid2d/Main2d.cpp
    )
# [Fluid2d]设置Shader路径
set(FLUID_2D_SHADER_DIR ${CMAKE_SOURCE_DIR}/Fluid2d/Shaders)
set(FLUID_2D_SHADER_FILES
    ${FLUID_2D_SHADER_DIR}/DrawMilk.vert
    ${FLUID_2D_SHADER_DIR}/DrawMilk.frag
    ${FLUID_2D_SHADER_DIR}/DrawParticals.vert
    ${FLUID_2D_SHADER_DIR}/DrawParticals.frag
    ${FLUID_2D_SHADER_DIR}/DrawSdf.vert
    ${FLUID_2D_SHADER_DIR}/DrawSdf.geom
    ${FLUID_2D_SHADER_DIR}/DrawSdf.frag
    )

# [Fluid3d]设置头文件路径
set(FLUID_3D_INC_DIR ${CMAKE_SOURCE_DIR}/Fluid3d/Inc)
set(FLUID_3D_INC_FILES
    ${FLUID_3D_INC_DIR}/BilateralFilter.h
    ${FLUID_3D_INC_DIR}/Material.h
    ${FLUID_3D_INC_DIR}/Parameters3d.h
    ${FLUID_3D_INC_DIR}/ParticalSystem3d.h
    ${FLUID_3D_INC_DIR}/RenderCamera.h
    ${FLUID_3D_INC_DIR}/RenderWidget.h
    ${FLUID_3D_INC_DIR}/SkyBox.h
    )
# [Fluid3d]设置源文件路径
set(FLUID_3D_SRC_DIR ${CMAKE_SOURCE_DIR}/Fluid3d/Src)
set(FLUID_3D_SRC_FILES
    ${FLUID_3D_SRC_DIR}/BilateralFilter.cpp
    ${FLUID_3D_SRC_DIR}/Material.cpp
    ${FLUID_3D_SRC_DIR}/ParticalSystem3d.cpp
    ${FLUID_3D_SRC_DIR}/RenderCamera.cpp
    ${FLUID_3D_SRC_DIR}/RenderWidget.cpp
    ${FLUID_3D_SRC_DIR}/SkyBox.cpp
    ${CMAKE_SOURCE_DIR}/Fluid3d/Main3d.cpp
    )
# [Fluid3d]设置Shader路径
set(FLUID_3D_SHADER_DIR ${CMAKE_SOURCE_DIR}/Fluid3d/Shaders)
set(FLUID_3D_SHADER_FILES
    ${FLUID_3D_SHADER_DIR}/BlurZ.comp
    ${FLUID_3D_SHADER_DIR}/ComputeParticals.comp
    ${FLUID_3D_SHADER_DIR}/DrawColor3d.frag
    ${FLUID_3D_SHADER_DIR}/DrawColor3d.vert
    ${FLUID_3D_SHADER_DIR}/DrawFluidColor.frag
    ${FLUID_3D_SHADER_DIR}/DrawFluidColor.vert
    ${FLUID_3D_SHADER_DIR}/DrawModel.frag
    ${FLUID_3D_SHADER_DIR}/DrawModel.vert
    ${FLUID_3D_SHADER_DIR}/PointSprite.geom
    ${FLUID_3D_SHADER_DIR}/PointSprite.vert
    ${FLUID_3D_SHADER_DIR}/PointSpriteThickness.frag
    ${FLUID_3D_SHADER_DIR}/PointSpriteZValue.frag
    ${FLUID_3D_SHADER_DIR}/ScreenQuad.frag
    ${FLUID_3D_SHADER_DIR}/ScreenQuad.vert
    ${FLUID_3D_SHADER_DIR}/SkyBox.frag
    ${FLUID_3D_SHADER_DIR}/SkyBox.vert
    )

# 全局包含目录
include_directories(${GLAD_DIR} ${GLM_DIR} ${GLFW_DIR} ${STB_DIR} ${COMMON_INC_DIR})

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
    ${CMAKE_SOURCE_DIR}/../third_party/glfw-3.3.8-x64-build/lib_vs2022_64_release/glfw3.lib
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
    ${CMAKE_SOURCE_DIR}/../third_party/glfw-3.3.8-x64-build/lib_vs2022_64_release/glfw3.lib
    )

set(ALL_FILES
    ${COMMON_INC_FILES} 
    ${COMMON_SRC_FILES}
    ${FLUID_3D_INC_FILES} 
    ${FLUID_3D_SRC_FILES}
    ${FLUID_2D_INC_FILES} 
    ${FLUID_2D_SRC_FILES}
    ${FLUID_2D_SHADER_FILES}
    ${FLUID_3D_SHADER_FILES}
    )
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${ALL_FILES})

# [vs]设置启动项目
set_property (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_3D})



