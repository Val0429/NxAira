#pragma once
#ifndef FRAME_H
#define FRAME_H

#include <memory>

#include <nx/sdk/analytics/i_uncompressed_video_frame.h>

#include "fwd/opencv.h"
#include "util.h"

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace aira {

/**
 * Stores frame data and cv::Mat. Note, there is no copying of image data in the constructor.
 */
class Frame {
public:
    const int width;
    const int height;
    const int64_t timestampUs;
private:
    const nx::sdk::analytics::IUncompressedVideoFrame* frame;
    cv::Mat cvMat;

public:
    Frame(const nx::sdk::analytics::IUncompressedVideoFrame* frame);

    cv::Mat& getMat();
    std::vector<uchar> getBuffer();
    std::string getBase64String();

private:
    std::shared_ptr<spdlog::logger> logger;
};

} // namespace sample
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx

#endif