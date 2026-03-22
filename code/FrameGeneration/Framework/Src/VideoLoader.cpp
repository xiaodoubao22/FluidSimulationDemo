#include "VideoLoader.h"
#include <iostream>

namespace Fw {
    VideoLoader::VideoLoader() {
        mPacket = av_packet_alloc();
        mFrame = av_frame_alloc();
        mRGBFrame = av_frame_alloc();
    }

    VideoLoader::~VideoLoader() {
        Close();
        av_frame_free(&mFrame);
        av_frame_free(&mRGBFrame);
        av_packet_free(&mPacket);
    }

    bool VideoLoader::Open(const std::string& filepath) {
        if (mIsOpened) {
            Close();
        }

        if (avformat_open_input(&mFormatCtx, filepath.c_str(), nullptr, nullptr) != 0) {
            std::cerr << "Failed to open video file: " << filepath << std::endl;
            return false;
        }

        if (avformat_find_stream_info(mFormatCtx, nullptr) < 0) {
            std::cerr << "Failed to find stream info" << std::endl;
            return false;
        }

        mVideoStreamIndex = -1;
        for (unsigned int i = 0; i < mFormatCtx->nb_streams; i++) {
            if (mFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                mVideoStreamIndex = i;
                break;
            }
        }

        if (mVideoStreamIndex == -1) {
            std::cerr << "Failed to find video stream" << std::endl;
            return false;
        }

        AVStream* stream = mFormatCtx->streams[mVideoStreamIndex];
        const AVCodec* codec = avcodec_find_decoder(stream->codecpar->codec_id);
        if (!codec) {
            std::cerr << "Failed to find decoder" << std::endl;
            return false;
        }

        mCodecCtx = avcodec_alloc_context3(codec);
        avcodec_parameters_to_context(mCodecCtx, stream->codecpar);

        if (avcodec_open2(mCodecCtx, codec, nullptr) < 0) {
            std::cerr << "Failed to open codec" << std::endl;
            return false;
        }

        mWidth = mCodecCtx->width;
        mHeight = mCodecCtx->height;
        mDuration = mFormatCtx->duration;

        if (stream->avg_frame_rate.den != 0) {
            mFrameRate = av_q2d(stream->avg_frame_rate);
        } else {
            mFrameRate = 25.0;
        }

        if (mDuration > 0 && mFrameRate > 0) {
            mTotalFrames = static_cast<int>(mDuration * mFrameRate / AV_TIME_BASE);
        } else {
            mTotalFrames = 0;
        }
        mCurrentFrame = 0;

        int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, mWidth, mHeight, 1);
        mRGBBuffer.resize(numBytes);
        av_image_fill_arrays(mRGBFrame->data, mRGBFrame->linesize, mRGBBuffer.data(), AV_PIX_FMT_RGB24, mWidth, mHeight, 1);

        mSwsCtx = sws_getContext(mWidth, mHeight, mCodecCtx->pix_fmt, mWidth, mHeight, AV_PIX_FMT_RGB24, SWS_BILINEAR, nullptr, nullptr, nullptr);

        mIsOpened = true;
        mEOF = false;

        std::cout << "Video info: " << mWidth << "x" << mHeight 
                   << ", FPS: " << mFrameRate 
                   << ", Total frames: " << mTotalFrames << std::endl;
        return true;
    }

    void VideoLoader::Close() {
        if (mSwsCtx) {
            sws_freeContext(mSwsCtx);
            mSwsCtx = nullptr;
        }
        if (mCodecCtx) {
            avcodec_free_context(&mCodecCtx);
            mCodecCtx = nullptr;
        }
        if (mFormatCtx) {
            avformat_close_input(&mFormatCtx);
            mFormatCtx = nullptr;
        }
        mIsOpened = false;
    }

    bool VideoLoader::ReadFrame(VideoFrame& frame) {
        if (!mIsOpened) return false;

        frame.width = mWidth;
        frame.height = mHeight;

        int ret;

        while (true) {
            ret = avcodec_receive_frame(mCodecCtx, mFrame);
            if (ret == 0) {
                mCurrentFrame++;
                sws_scale(mSwsCtx, mFrame->data, mFrame->linesize, 0, mHeight, mRGBFrame->data, mRGBFrame->linesize);
                frame.data[0] = mRGBFrame->data[0];
                frame.linesize[0] = mRGBFrame->linesize[0];
                frame.pts = mFrame->pts * av_q2d(mFormatCtx->streams[mVideoStreamIndex]->time_base);
                return true;
            }

            if (ret == AVERROR_EOF) {
                return false;
            }

            if (ret != AVERROR(EAGAIN)) {
                return false;
            }

            if (mEOF) {
                return false;
            }

            av_packet_unref(mPacket);
            ret = av_read_frame(mFormatCtx, mPacket);
            if (ret < 0) {
                if (ret == AVERROR_EOF) {
                    mEOF = true;
                    avcodec_send_packet(mCodecCtx, nullptr);
                    continue;
                }
                return false;
            }

            if (mPacket->stream_index != mVideoStreamIndex) {
                continue;
            }

            ret = avcodec_send_packet(mCodecCtx, mPacket);
            if (ret < 0 && ret != AVERROR(EAGAIN)) {
                continue;
            }
        }
    }

    void VideoLoader::Seek(double timestamp) {
        if (!mIsOpened || mDuration <= 0) return;
        
        avcodec_flush_buffers(mCodecCtx);
        avformat_flush(mFormatCtx);
        mEOF = false;
        
        AVStream* stream = mFormatCtx->streams[mVideoStreamIndex];
        int64_t ts = static_cast<int64_t>(timestamp * stream->time_base.den / stream->time_base.num);
        av_seek_frame(mFormatCtx, mVideoStreamIndex, ts, AVSEEK_FLAG_BACKWARD);
    }

    void VideoLoader::ReleaseFrame(VideoFrame& frame) {
    }
}
