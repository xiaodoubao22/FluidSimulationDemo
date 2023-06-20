#include <iostream>
#include "RenderWidget.h"
#include "ParticalSystem.h"
#include <thread>
#include "Global.h"
#include "Solver.h"

int main() {
    
    RenderWidget renderer;
    renderer.Init();

    Fluid2d::ParticalSystem ps;
    ps.SetContainerSize(glm::vec2(-1.0, -1.0), glm::vec2(2.0, 2.0));
    ps.AddFluidBlock(glm::vec2(-0.2, -0.2), glm::vec2(0.4, 0.4), glm::vec2(-2.0f, -10.0f), 0.01f * 0.7f);
    std::cout << "partical num = " << ps.mPositions.size() << std::endl;

    Fluid2d::Solver sv(ps);

    while (!renderer.ShouldClose()) {
        renderer.ProcessInput();    // 处理输入事件
        
        for (int i = 0; i < Fluid2d::Para::substep; i++) {    // 求解
            ps.SearchNeighbors();
            sv.Iterate();
        }

        renderer.LoadVertexes(ps);
        renderer.Update();
        renderer.PollEvents();
    }
    
    return 0;
}