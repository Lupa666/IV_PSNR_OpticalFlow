#include "xUtilsOCV.h"

namespace PMBB_NAMESPACE {
    void xUtilsOCV::xPlane2Mat(xPlane<flt32>& picture_input, cv::Mat& picture_output) {
        assert(picture_output.size().width == picture_input.getWidth() && picture_output.size().height == picture_input.getHeight());
        for (int x = 0; x < picture_input.getWidth(); x++) {
            for (int y = 0; y < picture_input.getHeight(); y++)
            {
                picture_output.at<float>(y, x) = *picture_input.getAddr(int32V2(x, y));
            }
        }
    }

    void xUtilsOCV::Mat2xPlane(cv::Mat& picture_input, xPlane<flt32V2>& picture_output) {
        assert(picture_input.size().width == picture_output.getWidth() && picture_input.size().height == picture_output.getHeight());
        for (int x = 0; x < picture_output.getWidth(); x++) {
            for (int y = 0; y < picture_output.getHeight(); y++)
            {
                picture_output.accessPel(int32V2(x, y))[0] = picture_input.at<flt32>(y, 0 + x * 2);
                picture_output.accessPel(int32V2(x, y))[1] = picture_input.at<flt32>(y, 1 + x * 2);
            }
        }
    }

    void xUtilsOCV::Mat2xPlane(cv::Mat& picture_input, xPlane<flt32>& picture_output) {
        assert(picture_input.size().width == picture_output.getWidth() && picture_input.size().height == picture_output.getHeight());
        for (int x = 0; x < picture_output.getWidth(); x++) {
            for (int y = 0; y < picture_output.getHeight(); y++)
            {
                picture_output.accessPel(int32V2(x, y)) = picture_input.at<flt32>(y, x);
            }
        }
    }

    bool xUtilsOCV::IsSamexPic(xPicP& one, xPicP& two)
    {
        if (one.getSize() != two.getSize()) return false;
        int32 one_stride = one.getStride();
        int32 two_stride = two.getStride();
        uint16* one_addr = one.getAddr(eCmp::C0);
        uint16* two_addr = two.getAddr(eCmp::C0);
        for (int32 y = 0; y < one.getHeight(); y++)
        {
            for (int32 x = 0; x < one.getWidth(); x++)
            {
                if (one_addr[x] != two_addr[x]) return false;
            }
            one_addr += one_stride;
            two_addr += two_stride;
        }
        return true;

        return false;
    }

    void xUtilsOCV::xPic2Mat(xPicP& picture_input, cv::Mat& picture_output, int channels = 3) {
        assert(picture_output.size().width == picture_input.getWidth() && picture_output.size().height == picture_input.getHeight());
        assert(channels == picture_output.channels());
        for (int x = 0; x < picture_input.getWidth(); x++) {
            for (int y = 0; y < picture_input.getHeight(); y++)
            {
                for (int z = 0; z < channels; z++) {
                    picture_output.at<uint16>(y, z + (x * channels)) = *picture_input.getAddr(int32V2(x, y), eCmp(z));
                }

            }
        }
        return;
    }

    void xUtilsOCV::Mat2xPic(cv::Mat& picture_input, xPicP& picture_output, int channels = 3) {
        assert(picture_input.size().width == picture_output.getWidth() && picture_input.size().height == picture_output.getHeight());
        assert(channels == picture_input.channels());
        for (int x = 0; x < picture_input.size().width; x++) {
            for (int y = 0; y < picture_input.size().height; y++)
            {
                for (int z = 0; z < channels; z++) {
                    *picture_output.getAddr(int32V2(x, y), eCmp(z)) = (uint16)picture_input.at<uint16>(y, z + (x * channels));
                }
            }
        }
        return;
    }
}