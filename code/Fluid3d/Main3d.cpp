#include <iostream>
#include "RenderWidget.h"
#include <thread>
#include "Global.h"
#include "ParticalSystem3d.h"

struct test {
    float a[200];
};

int main() {

    Fluid3d::ParticalSystem3D ps;
    ps.SetContainerSize(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.6, 0.6, 0.6));
    ps.AddFluidBlock(glm::vec3(0.2, 0.2, 0.25), glm::vec3(0.2, 0.2, 0.3), glm::vec3(-1.0, 0.0, -3.0), 0.01 * 0.7);
    //ps.AddFluidBlock(glm::vec3(0.0, 0.2, 0.15), glm::vec3(0.15, 0.15, 0.2), glm::vec3(0.0, -1.0, -1.0), 0.01 * 0.85);
    //ps.AddFluidBlock(glm::vec3(0.1, 0.0, 0.05), glm::vec3(0.1, 0.1, 0.3), glm::vec3(-1.0, 1.0, -0.5), 0.01 * 0.85);
    ps.UpdateData();
    std::cout << "partical num = " << ps.mParticalInfos.size() << std::endl;
    
    Fluid3d::RenderWidget renderer;
    renderer.Init();
    renderer.UploadUniforms(ps);

    while (!renderer.ShouldClose()) {
        renderer.ProcessInput();    // 处理输入事件
        
        for (int i = 0; i < 8; i++) {
            ps.UpdateData();
            renderer.UploadParticalInfo(ps);
            renderer.SolveParticals();
            renderer.DumpParticalInfo(ps);
        }
        renderer.Update();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        renderer.PollEvents();
    }
    
    return 0;
}