#include <iostream>
#include <cxxopts.hpp>
#include "cistring.h"
extern "C" {
#include <git2/global.h> // Global libgit stuff
};
#include "comparitor.h"

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

    // Manually init libgit2 (order of operation would'nt allow for this in constructor)
    git_libgit2_init();
	// Make unique to handle cases where construction failes. If construction failes will instead return empty object and throw error.
	try {
		auto comparitor = std::make_unique<gprc::Comparitor>("C:/Users/andes/Documents/CppProjects/GitPRComparisonTool/");
		comparitor->compare("167b5e814b1fc5fbf2cf4a4a2a001ae52ed34654", "b7749efdaa61fdefd06806ff661102a844995cf9");
	}
	catch(std::runtime_error err) {
		std::cout << "Comparitor failed with: " << err.what() << std::endl;
		git_libgit2_shutdown();
		return 1;
	}

    std::cout << "Everything worked perfectly! :)" << std::endl;
    git_libgit2_shutdown();
    return 0;
}