# FluidSimulationOnWeekend

## 简介

一个实时流体模拟和渲染Demo，流体仿真部分实现了SPH算法，用OpenGL的计算着色器实现。渲染部分用了screen-space的方法，包括流体渲染、实时阴影、实时焦散等，并且加入了鼠标拖动交互。

技术贴：<https://zhuanlan.zhihu.com/p/637919956>
视频链接：<https://www.bilibili.com/video/BV1sj411D7hp/?spm_id_from=333.999.0.0&vd_source=e15e0d4b18f68edd4c847aa168b3f59d>

## 效果图

### 3D模拟

<img src="./figure/3dFluid.png" width="600"> 

<img src="./figure/3dResult.gif" width="600">

### 2D模拟及牛奶效果渲染

<img src="./figure/milk.gif" width="400">

## 三方库

- GLM
- GLFW
- GLAD
