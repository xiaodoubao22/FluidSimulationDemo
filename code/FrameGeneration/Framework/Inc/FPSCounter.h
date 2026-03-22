#ifndef FPS_COUNTER_H
#define FPS_COUNTER_H

#include <string>
#include <chrono>

namespace Fw {
    class FPSCounter {
    public:
        FPSCounter();

        void Tick();
        float GetFPS() const;
        int GetFrameCount() const;
        std::string GetTitleWithFPS(const std::string& baseTitle) const;

    private:
        std::chrono::high_resolution_clock::time_point mLastTime;
        int mFrameCount;
        float mFPS;
        std::chrono::high_resolution_clock::time_point mStartTime;
    };
}

#endif
