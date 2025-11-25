#ifndef ARGS_H
#define ARGS_H

#include <argparse/argparse.hpp>

#include <expected>
#include <string>

struct ArgConfig {
    int theta_coalesce;
    int threshold;
    std::string img_path;
    std::string out_dir;
};

std::expected<ArgConfig, std::string> parse_args(int argc, char *argv[]);

#endif // ARGS_H
