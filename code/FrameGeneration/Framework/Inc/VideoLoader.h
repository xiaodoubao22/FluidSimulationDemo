#ifndef VIDEO_LOADER_H
#define VIDEO_LOADER_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

#include <string>
#include <vector>

namespace Fw {
    struct VideoFrame {
        uint8_t* data[4] = { nullptr };
        int linesize[4] = { 0 };
        int width = 0;
        int height = 0;
        double pts = 0.0;
    };

    class VideoLoader {
    public:
        VideoLoader();
        ~VideoLoader();

        bool Open(const std::string& filepath);
        void Close();
        bool ReadFrame(VideoFrame& frame);
        void Seek(double timestamp);
        bool IsOpened() const { return mIsOpened; }
        int GetWidth() const { return mWidth; }
        int GetHeight() const { return mHeight; }
        double GetFrameRate() const { return mFrameRate; }
        int GetTotalFrames() const { return mTotalFrames; }
        int GetCurrentFrame() const { return mCurrentFrame; }
        void ReleaseFrame(VideoFrame& frame);

    private:
        bool mIsOpened = false;
        AVFormatContext* mFormatCtx = nullptr;
        AVCodecContext* mCodecCtx = nullptr;
        AVPacket* mPacket = nullptr;
        AVFrame* mFrame = nullptr;
        AVFrame* mRGBFrame = nullptr;
        SwsContext* mSwsCtx = nullptr;
        int mVideoStreamIndex = -1;
        int mWidth = 0;
        int mHeight = 0;
        double mFrameRate = 0.0;
        int64_t mDuration = 0;
        std::vector<uint8_t> mRGBBuffer;
        bool mEOF = false;
        int mTotalFrames = 0;
        int mCurrentFrame = 0;
    };
}

#endif
