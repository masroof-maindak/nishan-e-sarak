#include <nshn/hough.h>

#include <cmath>

std::expected<std::vector<PolarCoord>, std::string> hough_transform(const cv::Mat &img, const int theta_coalesce,
                                                                    const int threshold) {
    using std::vector;

    if (img.type() != CV_8UC1)
        return std::unexpected{"Hough transform expects a binary grayscale image."};

    const int votes_cols{180 / theta_coalesce};
    const int max_dist{static_cast<int>(std::hypot(img.rows, img.cols))};

    const size_t votes_rows{static_cast<size_t>(2 * max_dist)};
    vector<vector<int>> votes{votes_rows, vector<int>(votes_cols, 0)};

    vector<PolarCoord> lines;

    const int rows{img.rows};
    const int cols{img.cols};

    // Compute accumulator matrix
    for (int y = 0; y < rows; y++) {
        auto *mask_row = img.ptr<std::uint8_t>(y);
        for (int x = 0; x < cols; x++) {
            if (mask_row[x] != 0) { // edge exists
                for (int theta = 0; theta < votes_cols; theta += 1) {
                    // TODO: pre-compute trigonometric values
                    const int rho{static_cast<int>(x * std::cos(theta - 90) + y * std::sin(theta - 90)) + max_dist};
                    ++votes[rho][theta];
                }
            }
        }
    }

    // Threshold
    for (int y = 0; y < votes_cols; y++)
        for (int x = 0; x < votes_cols; x++)
            if (votes[y][x] > threshold)
                lines.emplace_back(std::make_pair(x - max_dist, y - 90));

    return lines;
}
