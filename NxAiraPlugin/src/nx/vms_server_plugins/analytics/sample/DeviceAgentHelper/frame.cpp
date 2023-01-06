#include "frame.h"

#ifdef max
#undef max
#undef min
#endif
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "util.h"
#include "spdlog/spdlog.h"
//#include <libbase64.h>
#include "ValBase64.hpp"
//#include <boost/program_options.hpp>
// #include <boost/filesystem.hpp>

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace aira {

Frame::Frame(const nx::sdk::analytics::IUncompressedVideoFrame* frame):
    width(frame->width()),
    height(frame->height()),
    timestampUs(frame->timestampUs()),
    frame(frame),
    logger(CreateLogger("Frame"))
    {
}

cv::Mat& Frame::getMat() {
    if (cvMat.empty()) {
        cvMat = cv::Mat(
        /*_rows*/ frame->height(),
        /*_cols*/ frame->width(),
        /*_type*/ CV_8UC3, //< BGR color space (default for OpenCV).
        /*_data*/ (void*) frame->data(0),
        /*_step*/ (size_t) frame->lineSize(0));
    }
    return cvMat;
}

std::vector<uchar> Frame::getBuffer() {
    auto& mat = getMat();

    std::vector<uchar> buf;
    std::vector<int> param(2);
    param[0] = cv::IMWRITE_JPEG_QUALITY;
    param[1] = 80;

    cv::imencode(".jpg", mat, buf, param);
    // cv::imwrite("D:\\test.jpg", mat, param);

    return buf;
}

std::string Frame::getBase64String() {
    const auto& buf = getBuffer();
    std::string base64_string = ValBase64::Encode<std::string>(buf, "");
    return base64_string;
}

} // namespace sample
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx