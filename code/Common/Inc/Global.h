#ifndef GLOBAL_H
#define GLOBAL_H
#include <chrono>
#include <random>

namespace Glb {
    const float_t EPS = 1e-5;
    const glm::vec3 Z_AXIS = glm::vec3(0.0, 0.0, 1.0);
    const glm::vec3 COLOR_GREEN = glm::vec3(0.0, 1.0, 0.0);
    const glm::vec3 COLOR_RED = glm::vec3(1.0, 0.0, 0.0);
    const glm::vec3 COLOR_BLUE = glm::vec3(0.0, 0.0, 1.0);
    const std::vector<glm::vec3> ORIGIN_COLORS = { COLOR_RED, COLOR_GREEN, COLOR_BLUE };

    class Timer {
    private:
        std::chrono::system_clock::time_point mStartPoint;
    public:
        void Start() {
            mStartPoint = std::chrono::system_clock::now();
        }

        int GetTime() {
            auto dur = std::chrono::system_clock::now() - mStartPoint;
            return std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
        }
    };


    class RandomGenerator {
    private:
        std::random_device dev;
    public:
        float GetUniformRandom(float min = 0.0f, float max = 1.0f) {
            std::mt19937 rng(dev());
            std::uniform_real_distribution<float> dist(min, max); // distribution in range [min, max]
            return dist(rng);
        }
    };

    static glm::vec4 ProjToIntrinsic(glm::mat4 projection, float w, float h) {
        glm::vec4 intrinsic = {};

        float tanHalfFovx = 1.0 / projection[0][0];
        float tanHalfFovy = 1.0 / projection[1][1];
        intrinsic.x = 2.0 * tanHalfFovx / w;      // fxInv
        intrinsic.y = 2.0 * tanHalfFovy / h;     // fyInv
        intrinsic.z = w / 2.0;                    // cx
        intrinsic.w = h / 2.0;                    // cy

        return intrinsic;
    }

}


#endif // !GLOBAL_H




