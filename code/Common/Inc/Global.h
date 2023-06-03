#ifndef GLOBAL_H
#define GLOBAL_H
#include <chrono>
#include <random>

namespace Glb {
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

}


#endif // !GLOBAL_H




