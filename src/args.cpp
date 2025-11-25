#include <nshn/args.h>

std::expected<ArgConfig, std::string> parse_args(int argc, char *argv[]) {
    argparse::ArgumentParser prog("nshn", "v2025-11-25a", argparse::default_arguments::help);

    ArgConfig args{};

    prog.add_argument("-i").required().help("specify the input image").store_into(args.img_path);

    prog.add_argument("-o", "--output-dir").required().help("specify the output dir").store_into(args.out_dir);

    prog.add_argument("-c", "--theta-coalesce")
        .help("specify the number of angles to coalesce into one theta-box of the accumulator")
        .default_value(1)
        .scan<'i', int>()
        .store_into(args.theta_coalesce);

    prog.add_argument("-t", "--threshold")
        .help("specify the number of intersections before a point gets classified as a line")
        .default_value(125)
        .scan<'i', int>()
        .store_into(args.threshold);

    try {
        prog.parse_args(argc, argv);
    } catch (const std::exception &err) {
        auto errmsg{std::format("{}\n\n{}", err.what(), prog.usage())};
        return std::unexpected(errmsg);
    }

    if (args.theta_coalesce <= 0)
        return std::unexpected(std::format("theta-coalesce must be > 0: {}", args.theta_coalesce));

    if (args.threshold <= 0)
        return std::unexpected(std::format("threshold must be > 0: {}", args.threshold));

    return args;
}
