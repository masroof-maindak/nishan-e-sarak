#ifndef HOUGH_H
#define HOUGH_H

#include <opencv2/opencv.hpp>

#include <expected>
#include <utility>
#include <vector>

using PolarCoord = std::pair<int, int>;

/*
 * @param img 8UC1
 * @param theta_coalesce number of angles to coalesce into one theta-box of the accumulator i.e votes matrix
 * @param threshold number of intersections before that point gets classified as a line
 */
std::expected<std::vector<PolarCoord>, std::string> hough_transform(const cv::Mat &img, const int theta_coalesce,
                                                                    const int threshold);

#endif // HOUGH_H
