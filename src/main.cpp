#include <iostream>
#include <cxxopts.hpp>

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

    auto format = result["format"].as<std::string>();

    

    return 0;
}