#ifndef W_CUBE_SPLINE_H
#define W_CUBE_SPLINE_H

#include <glm/glm.hpp>
#include <vector>

namespace Glb {
    class WCubicSpline2d {
    public:
        WCubicSpline2d() = delete;
        explicit WCubicSpline2d(float h);
        ~WCubicSpline2d();

        float Value(float distance);

        glm::vec2 Grad(glm::vec2 radius);
        
    private:
        float CalculateValue(float distance);

        glm::vec2 CalculateGrad(glm::vec2 radius);

    private:
        float mH;
        float mH2;
        float mSigma;
        glm::uvec2 mBufferSize;
        std::vector<std::vector<glm::vec2>> mGradBuffer;
        std::vector<float> mValueBuffer;
    };

    class WCubicSpline3d {
    public:
        WCubicSpline3d() = delete;
        explicit WCubicSpline3d(float h);
        ~WCubicSpline3d();
        
        float_t* GetData();
        uint32_t GetBufferSize();

    private:
        float CalculateValue(float distance);

        float CalculateGradFactor(float distance);

    private:
        float mH;
        float mH2;
        float mH3;
        float mSigma;
        uint32_t mBufferSize;
        std::vector<glm::vec2> mValueAndGradFactorBuffer;
    };

}

#endif // !W_CUBE_SPLINE_H
