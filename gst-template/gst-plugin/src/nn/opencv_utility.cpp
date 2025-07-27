#include "opencv_utility.h"
#include "opencv_colormaps.h"
#include <array>
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <chrono>
#include "utility.h"

std::vector<cv::Mat> custom_colormap;

int initializeColormap()
{
    std::cout << __func__ << std::endl;

    constexpr size_t colormap_size = 10;
    custom_colormap.reserve(colormap_size);

    std::array<cv::Vec3b *, colormap_size> colormap_array = {
        colormap_whitehot,
        colormap_blackhot,
        colormap_rainbow,
        colormap_rainhc,
        colormap_ironbow,
        colormap_lava,
        colormap_arctic,
        colormap_glowbow,
        colormap_gradedfire,
        colormap_hottest};

    for (size_t i = 0; i < colormap_size; ++i)
    {
        cv::Mat colormap(256, 1, CV_8UC3);
        cv::Vec3b *colormap_ptr = colormap.ptr<cv::Vec3b>(0);
        std::copy(colormap_array[i], colormap_array[i] + 256, colormap_ptr);
        custom_colormap.push_back(std::move(colormap));
    }

    return 0;
}

void bgr2uyvy(const cv::Mat &bgrImage, cv::Mat &uyvyImage)
{
    if (bgrImage.empty() || bgrImage.type() != CV_8UC3) {
        throw std::invalid_argument("Input image must be non-empty and of type CV_8UC3 (BGR)");
    }

    int height = bgrImage.rows;
    int width = bgrImage.cols;

    uyvyImage.create(height, width, CV_8UC2);

    cv::parallel_for_(cv::Range(0, height), [&](const cv::Range &r) {
        for (int y = r.start; y < r.end; ++y) {
            const uchar* bgrRow = bgrImage.ptr<uchar>(y);
            uchar* uyvyRow = uyvyImage.ptr<uchar>(y);

            for (int x = 0; x < width; x += 2) {
                int b1 = bgrRow[3 * x + 0], g1 = bgrRow[3 * x + 1], r1 = bgrRow[3 * x + 2];
                int b2 = bgrRow[3 * (x + 1) + 0], g2 = bgrRow[3 * (x + 1) + 1], r2 = bgrRow[3 * (x + 1) + 2];

                uchar y1 = cv::saturate_cast<uchar>((77 * r1 + 150 * g1 + 29 * b1) >> 8);
                uchar y2 = cv::saturate_cast<uchar>((77 * r2 + 150 * g2 + 29 * b2) >> 8);

                uchar u = cv::saturate_cast<uchar>(((-38 * (r1 + r2) - 74 * (g1 + g2) + 112 * (b1 + b2)) >> 9) + 128);
                uchar v = cv::saturate_cast<uchar>(((112 * (r1 + r2) - 94 * (g1 + g2) - 18 * (b1 + b2)) >> 9) + 128);

                uyvyRow[2 * x + 0] = u;  // U
                uyvyRow[2 * x + 1] = y1; // Y0
                uyvyRow[2 * x + 2] = v;  // V
                uyvyRow[2 * x + 3] = y2; // Y1
            }
    } });
}

void bgr2yuy2(const cv::Mat &bgrImage, cv::Mat &yuy2Image)
{
    if (bgrImage.empty() || bgrImage.type() != CV_8UC3) {
        throw std::invalid_argument("Input image must be non-empty and of type CV_8UC3 (BGR)");
    }

    int width = bgrImage.cols;
    int height = bgrImage.rows;

    yuy2Image.create(height, width, CV_8UC2);

    cv::parallel_for_(cv::Range(0, height), [&](const cv::Range &range) {
        for (int y = range.start; y < range.end; ++y) {
            const uchar* bgrRow = bgrImage.ptr<uchar>(y);
            uchar* yuy2Row = yuy2Image.ptr<uchar>(y);

            for (int x = 0; x < width; x += 2) {
                int b1 = bgrRow[3 * x + 0], g1 = bgrRow[3 * x + 1], r1 = bgrRow[3 * x + 2];
                int b2 = bgrRow[3 * (x + 1) + 0], g2 = bgrRow[3 * (x + 1) + 1], r2 = bgrRow[3 * (x + 1) + 2];

                uchar y1 = cv::saturate_cast<uchar>((77 * r1 + 150 * g1 + 29 * b1) >> 8);
                uchar y2 = cv::saturate_cast<uchar>((77 * r2 + 150 * g2 + 29 * b2) >> 8);

                uchar u = cv::saturate_cast<uchar>(((-43 * (r1 + r2) - 84 * (g1 + g2) + 127 * (b1 + b2)) >> 9) + 128);
                uchar v = cv::saturate_cast<uchar>(((127 * (r1 + r2) - 106 * (g1 + g2) - 21 * (b1 + b2)) >> 9) + 128);

                yuy2Row[2 * x + 0] = y1;  // Y0
                yuy2Row[2 * x + 1] = u;   // U
                yuy2Row[2 * x + 2] = y2;  // Y1
                yuy2Row[2 * x + 3] = v;   // V
            }
    } });
}

void bgr2nv12(const cv::Mat &bgrImage, cv::Mat &nv12Image) {
    if (bgrImage.empty() || bgrImage.type() != CV_8UC3) {
        throw std::invalid_argument("Input image must be non-empty and of type CV_8UC3 (BGR)");
    }

    int width = bgrImage.cols;
    int height = bgrImage.rows;

    // Step 1: BGR → I420
    cv::Mat yuv_i420;
    cv::cvtColor(bgrImage, yuv_i420, cv::COLOR_BGR2YUV_I420);

    const uint8_t* y_plane = yuv_i420.data;
    const uint8_t* u_plane = y_plane + width * height;
    const uint8_t* v_plane = u_plane + (width / 2) * (height / 2);

    // Step 2: Allocate NV12 image
    nv12Image.create(height + height / 2, width, CV_8UC1);
    uint8_t* nv12_y = nv12Image.data;
    uint8_t* nv12_uv = nv12_y + width * height;

    // Step 3: Copy Y plane
    memcpy(nv12_y, y_plane, width * height);

    // Step 4: Interleave UV with parallel_for_
    cv::parallel_for_(cv::Range(0, height / 2), [&](const cv::Range& range) {
        for (int j = range.start; j < range.end; ++j) {
            const uint8_t* u_row = u_plane + j * (width / 2);
            const uint8_t* v_row = v_plane + j * (width / 2);
            uint8_t* uv_row = nv12_uv + j * width;

            for (int i = 0; i < width / 2; ++i) {
                uv_row[i * 2] = u_row[i];
                uv_row[i * 2 + 1] = v_row[i];
            }
        }
    });
}

bool convert_to_bgr(unsigned char *src, int width, int height, const char *format, cv::Mat &outputMat)
{
    if (!src || width <= 0 || height <= 0 || !format) {
        std::cerr << "Invalid input parameters" << std::endl;
        return false;
    }
    
    if (strcmp(format, "UYVY") == 0)
    {
        cv::Mat image(height, width, CV_8UC2, src);
        cv::cvtColor(image, outputMat, cv::COLOR_YUV2BGR_UYVY);
    }
    else if (strcmp(format, "YUY2") == 0)
    {
        cv::Mat image(height, width, CV_8UC2, src);
        cv::cvtColor(image, outputMat, cv::COLOR_YUV2BGR_YUY2);
    }
    else if (strcmp(format, "BGR") == 0)
    {
        outputMat = cv::Mat(height, width, CV_8UC3, src).clone();
    }
    else if (strcmp(format, "NV12") == 0)
    {
        cv::Mat image(height * 3 / 2, width, CV_8UC1, src);
        cv::cvtColor(image, outputMat, cv::COLOR_YUV2BGR_NV12);
    }
    else
    {
        return false;
    }

    return true;
}

bool convert_from_bgr(cv::Mat &inputMat, cv::Mat &outputMat, const char *format)
{
    if (strcmp(format, "UYVY") == 0)
    {
        bgr2uyvy(inputMat, outputMat);
    }
    else if (strcmp(format, "YUY2") == 0)
    {
        bgr2yuy2(inputMat, outputMat);
    }
    else if (strcmp(format, "BGR") == 0)
    {
        outputMat = inputMat;
    }
    else if (strcmp(format, "NV12") == 0)
    {
        bgr2nv12(inputMat, outputMat);
    }
    else
    {
        return false;
    }

    return true;
}

std::vector<cv::Scalar> generateRandomRGB(int count)
{
    std::vector<cv::Scalar> scalarGroups;
    scalarGroups.reserve(count);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);

    for (int i = 0; i < count; ++i)
    {
        int r = dis(gen);
        int g = dis(gen);
        int b = dis(gen);
        scalarGroups.emplace_back(r, g, b);
    }

    return scalarGroups;
}
