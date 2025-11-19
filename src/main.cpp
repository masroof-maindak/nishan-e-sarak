#include <nshn/args.h>
#include <nshn/io.h>

#include <knr/canny.h>
#include <knr/gauss.h>
#include <opencv2/opencv.hpp>

#include <print>
#include <stdlib.h>

constexpr float sigma{1.4f};
constexpr float T{0.3f};

int main(int argc, char *argv[]) {
    // --- Config ---
    const auto args_expected = parse_args(argc, argv);
    if (!args_expected.has_value()) {
        std::println(stderr, "Failed to parse args: {}", args_expected.error());
        return EXIT_FAILURE;
    }
    const ArgConfig args{args_expected.value()};

    const std::string img_name{std::filesystem::path{args.img_path}.stem()};

    // --- Load Image ---
    const auto img_expected{load_image(args.img_path)};
    if (!img_expected.has_value()) {
        std::println(stderr, "Failed to load image: {}", img_expected.error());
        return EXIT_FAILURE;
    }
    const cv::Mat img{img_expected.value()};

    // --- Remove noise via gaussian ---
    const int filt_size{kd::compute_filter_size(sigma, T)};
    const auto gauss_filt_expected{kd::generate_gaussian_filter(filt_size, sigma)};
    if (!gauss_filt_expected.has_value()) {
        std::println(stderr, "Failed to generate Gaussian filter: {}", gauss_filt_expected.error());
        return EXIT_FAILURE;
    }
    const cv::Mat gauss_filt{gauss_filt_expected.value()};
    cv::Mat img_smoothed;
    cv::filter2D(img, img_smoothed, img.depth(), gauss_filt);

    // --- Convert to HSV ---
    cv::Mat hsv_smoothed{};
    cv::cvtColor(img_smoothed, hsv_smoothed, cv::COLOR_BGR2HSV);

    // --- Filter out non-white/yellow pixels ---
    const int rows{hsv_smoothed.rows};
    const int cols{hsv_smoothed.cols};
    cv::Mat hsv_filtered{hsv_smoothed.size(), CV_8UC3, cv::Scalar{0}};
    hsv_filtered.create(hsv_smoothed.size(), CV_8UC3);

    for (int y = 0; y < rows; y++) {
        const auto *src_row = hsv_smoothed.ptr<cv::Vec3b>(y);
        auto *dst_row       = hsv_filtered.ptr<cv::Vec3b>(y);
        for (int x = 0; x < rows; x++) {
            auto px = src_row[x];
            if ((px[0] > 53 && px[1] > 100 && px[2] > 100) || // yellow lower
                (px[0] < 63 && px[1] < 255 && px[2] < 255) || // yellow upper
                (px[0] < 63 && px[1] < 30 && px[2] > 230))    // white upper
                dst_row[x] = px;
        }
    }

    // --- Canny ---
    cv::Mat img_gray{};
    cv::cvtColor(img, img_gray, cv::COLOR_BGR2GRAY);
    auto thresh_mag_expected{kd::canny_edge_detector(img_name, img_gray, {sigma, T, 50, 90, args.out_dir}, false)};
    if (!thresh_mag_expected.has_value()) {
        std::println(stderr, "Failed to run canny: {}", thresh_mag_expected.error());
        return EXIT_FAILURE;
    }
    auto thresh_mag{thresh_mag_expected.value()};

    // Keep only the edges that correspond to white or yellow in the filtered HSV image.
    // All others can safely be made zero.

    auto edge_detections_save_detected{save_image(thresh_mag, args.out_dir, img_name, "edges")};

    for (int y = 0; y < rows; y++) {
        const auto *hsv_filtered_row = hsv_filtered.ptr<cv::Vec3b>(y);
        auto *mask_row               = thresh_mag.ptr<std::uint8_t>(y);
        for (int x = 0; x < rows; x++) {
            const auto px = hsv_filtered_row[x];
            bool black_px{px[0] == 0 && px[1] == 0 && px[2] == 0};
            if (black_px)
                mask_row[x] = 0;
        }
    }

    auto filt_edges_img_save_expected{save_image(thresh_mag, args.out_dir, img_name, "edges-filtered")};

    // --- Region of Interest
    // CHECK: what do I even do in here???

    // --- TODO: Hough Transform ---

    // --- TODO: Filter out lines w/ mostly horizontal slopes

    // --- TODO: Linear Regression ---

    return EXIT_SUCCESS;
}
