#include "FPSCounter.h"
#include <sstream>
#include <iomanip>

namespace Fw {
    FPSCounter::FPSCounter() 
        : mLastTime(std::chrono::high_resolution_clock::now())
        , mFrameCount(0)
        , mFPS(0.0f)
        , mStartTime(std::chrono::high_resolution_clock::now())
    {
    }

    void FPSCounter::Tick() {
        auto currentTime = std::chrono::high_resolution_clock::now();
        mFrameCount++;

        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - mLastTime).count();
        
        if (elapsed >= 1000) {
            mFPS = static_cast<float>(mFrameCount) * 1000.0f / static_cast<float>(elapsed);
            mFrameCount = 0;
            mLastTime = currentTime;
        }
    }

    float FPSCounter::GetFPS() const {
        return mFPS;
    }

    int FPSCounter::GetFrameCount() const {
        return mFrameCount;
    }

    std::string FPSCounter::GetTitleWithFPS(const std::string& baseTitle) const {
        std::ostringstream oss;
        oss << baseTitle << " | FPS: " << std::fixed << std::setprecision(1) << mFPS;
        return oss.str();
    }
}
