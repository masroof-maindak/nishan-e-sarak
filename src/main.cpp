#include <nshn/args.h>
#include <nshn/hough.h>
#include <nshn/io.h>

#include <knr/canny.h>
#include <knr/gauss.h>
#include <opencv2/opencv.hpp>

#include <print>
#include <stdlib.h>

constexpr float sigma{1.4f};
constexpr float T{0.3f};

cv::Mat draw_lines_onto_image(const cv::Mat &img, std::vector<PolarCoord> lines) {
    cv::Mat ret{img.clone()};

    for (PolarCoord line : lines) {
        auto [rho, theta]{line};
        cv::Point p1, p2;

        const double theta_rad{theta * CV_PI / 180.0};
        const double a{std::cos(theta_rad)}, b{std::sin(theta_rad)};
        const double x0{a * rho}, y0{b * rho};

        p1.x = cvRound(x0 + 600 * (-b));
        p1.y = cvRound(y0 + 600 * (a));
        p2.x = cvRound(x0 - 600 * (-b));
        p2.y = cvRound(y0 - 600 * (a));

        cv::line(ret, p1, p2, cv::Scalar(0, 255, 0), 2, cv::LINE_AA);
    }

    return ret;
}

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
        for (int x = 0; x < cols; x++) {
            auto px = src_row[x];
            if ((px[0] > 40 && px[1] > 100 && px[2] > 100) || // yellow lower
                (px[0] < 60 && px[1] < 255 && px[2] < 255) || // yellow upper
                (px[1] < 23 && px[2] > 230))                  // white upper
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
    auto edge_mat{thresh_mag_expected.value()};

    auto edge_detections_save_detected{save_image(edge_mat, args.out_dir, img_name, "edges")};

    /*
     * CHECK: this sometimes wipes out very _blatant_ lanes, I suspect because the edge boundary is just outside that of
     * the lane itself. Perhaps we should look at neighbours while performing this step?
     */
    cv::Mat edge_mat_filtered{edge_mat.clone()};

    // if this pixel is black in the filtered HSV image, it means that it's not a part of a lane marker (as those would
    // be yellow or white)
    for (int y = 0; y < rows; y++) {
        const auto *hsv_filtered_row = hsv_filtered.ptr<cv::Vec3b>(y);
        auto *mask_row               = edge_mat_filtered.ptr<std::uint8_t>(y);
        for (int x = 0; x < cols; x++) {
            const auto px = hsv_filtered_row[x];
            bool black_px{px[0] == 0 && px[1] == 0 && px[2] == 0};
            if (black_px)
                mask_row[x] = 0;
        }
    }

    auto filt_edges_img_save_expected{save_image(edge_mat_filtered, args.out_dir, img_name, "edges-filtered")};

    // --- Region of Interest
    // TODO: strip down edge_mat_filtered till only those edges contained within a trapezoid at the bottom remain

    // --- Hough Transform ---
    // TODO: get theta_coalesce and threshold via CLI args
    const auto lines_expected{hough_transform(edge_mat_filtered, 1, 125)};
    if (!lines_expected.has_value()) {
        std::println(stderr, "Failed to run hough transform: {}", lines_expected.error());
        return EXIT_FAILURE;
    }
    auto lines{lines_expected.value()};

    const cv::Mat img_annotated{draw_lines_onto_image(img, lines)};

    auto lines_img_save_expected{save_image(img_annotated, args.out_dir, img_name, "lines")};

    // --- Filter out lines w/ mostly horizontal slopes ---
    for (auto it = lines.begin(); it != lines.end();) {
        auto [rho, theta]{*it};

        // TODO: detect horizontal lines
        bool line_is_horiz{};

        if (line_is_horiz)
            it = lines.erase(it);
        else
            it++;
    }

    // --- TODO: Linear Regression ---

    return EXIT_SUCCESS;
}
