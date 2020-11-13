#include <iostream>
#include <cxxopts.hpp>
#include "cistring.h"
extern "C" {
#include <git2/global.h> // Global libgit stuff
};
#include "comparitor.h"
#include <filesystem>

int main(int argc, char *argv[]) {
    cxxopts::Options options("GitPRComp", "Git comparison tool for commit ranges");

    options.add_options()
        ("f,format", "Output format", cxxopts::value<std::string>()->default_value("html"))
        ("r,repo", "Repo to use. If no repo is specified, it will use the current working directory instead.", cxxopts::value<std::string>())
        ("h,help", "Print usage");

    const auto result = options.parse(argc, argv);

    if (result.count("help")) {
      std::cout << options.help() << std::endl;
      return 0;
    }

    // Manually init libgit2 (order of operation would'nt allow for this in constructor)
    git_libgit2_init();
	// Make unique to handle cases where construction failes. If construction failes will instead return empty object and throw error.
	try {
		auto comparitor = (result.count("repo")) 
            ? std::make_unique<gprc::Comparitor>(std::filesystem::path{result["repo"].as<std::string>()})
            : std::make_unique<gprc::Comparitor>();

        auto format = gprc::ci_string{result["format"].as<std::string>()};
        if (format == "html") {
            comparitor->setOutput(comparitor->repoPath() / "diff.html");
        } else if (format == "pdf") {

        } else {

        }
        if (3 <= argc)
		    comparitor->compare(argv[1], argv[2]);
        else if (2 == argc)
            comparitor->compare(argv[1], "HEAD");
        else
            std::cout << "You need to specify 1 or 2 commits to compare!" << std::endl;
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