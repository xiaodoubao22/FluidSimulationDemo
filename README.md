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

## 参考资料

[[1] TaiChi图形课程资料](https://github.com/taichiCourse01/taichiCourse01/blob/main/material/10_fluid_lagrangian.pdf)

[[2] Smoothed particle hydrodynamics: theory and application to non-spherical stars](https://academic.oup.com/mnras/article/181/3/375/988212?login=false)

[[3]【SPH】邻域搜索中的紧凑哈希（compact hash）算法讲解](https://blog.csdn.net/weixin_43940314/article/details/125789060)

[[4] Direct3D_Effects.pdf](https://developer.download.nvidia.cn/presentations/2010/gdc/Direct3D_Effects.pdf)

[[5] Interactive Image-Space Techniques for Approximating Caustics](http://cwyman.org/papers/i3d06_imgSpaceCaustics.pdf)

[[6] Chapter 37. A Toolkit for Computation on GPUs](https://developer.nvidia.com/gpugems/gpugems/part-vi-beyond-triangles/chapter-37-toolkit-computation-gpus)

[[7] 液体渲染：一种屏幕空间方法](https://zhuanlan.zhihu.com/p/38280537)

[[8] 基于粒子表示的实时流体模拟与渲染（下）](https://zhuanlan.zhihu.com/p/413812754)

[[9] SPH 3D流体模拟及其卡通化渲染](https://zhuanlan.zhihu.com/p/95102715)


