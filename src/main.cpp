#include <iostream>
#include <cxxopts.hpp>
#include "cistring.h"

int main(int argc, char *argv[]) {
    cxxopts::Options options("GitPRComp", "Git comparison tool for commit ranges");

    options.add_options()
        ("f,format", "Output format", cxxopts::value<std::string>()->default_value("html"))
        ("h,help", "Print usage");

    const auto result = options.parse(argc, argv);

    if (result.count("help")) {
      std::cout << options.help() << std::endl;
      return 0;
    }

    auto format = gprc::ci_string{result["format"].as<std::string>()};
    if (format == "html") {

    } else if (format == "pdf") {

    } else {

    }


    return 0;
}