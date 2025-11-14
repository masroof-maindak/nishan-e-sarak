#include <nshn/args.h>
#include <nshn/io.h>

#include <opencv2/opencv.hpp>

#include <print>
#include <stdlib.h>

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

    return 0;
}
