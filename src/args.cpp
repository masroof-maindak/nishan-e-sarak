#include <nshn/args.h>

std::expected<ArgConfig, std::string> parse_args(int argc, char *argv[]) {
	argparse::ArgumentParser prog("nshn", "v2025-11-14a",
								  argparse::default_arguments::help);

	ArgConfig args{};

	prog.add_argument("-i")
		.required()
		.help("specify the input image")
		.store_into(args.img_path);

	prog.add_argument("-o", "--output-dir")
		.required()
		.help("specify the output dir")
		.store_into(args.out_dir);

	try {
		prog.parse_args(argc, argv);
	} catch (const std::exception &err) {
		auto errmsg{std::format("{}\n\n{}", err.what(), prog.usage())};
		return std::unexpected(errmsg);
	}

	return args;
}
