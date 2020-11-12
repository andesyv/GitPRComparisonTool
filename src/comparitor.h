#ifndef COMPARITOR_H
#define COMPARITOR_H

#include <string>
#include <string_view>
#include <filesystem>
extern "C" {
    #include <git2.h>
    #include <git2/pathspec.h>
    // #include <git2/repository.h> // git_repository
    // #include <git2/tree.h> // git_tree functions
    // #include <git2/revparse.h>
};
#include <memory>
#include <stdexcept>
#include "gittreenode.h"

// http://blog.davidecoppola.com/2016/10/how-to-traverse-git-repository-using-libgit2-and-cpp/
// ^ For basic libgit usage

namespace gprc {

class Comparitor {
private:
    std::filesystem::path repoPath;
    struct git_repo_deleter { void operator()(git_repository* ptr){ git_repository_free(ptr); }};
    std::shared_ptr<git_repository> repo;

public:
    Comparitor(const std::filesystem::path& path = std::filesystem::current_path()) : repoPath{path} {
        // https://github.com/libgit2/libgit2/blob/master/tests/diff/tree.c
        // Open repository into repository object
        git_repository* r{nullptr};
        if (!git_repository_open(&r, repoPath.string().c_str()))
            repo = std::shared_ptr<git_repository>{r, [](git_repository* ptr){ git_repository_free(ptr); }};
        else
            throw std::runtime_error{"Failed to find specified git repository!"};
    }

    void compare(const std::string_view& c1, const std::string_view& c2) {
        auto t1{createTree(std::string{c1})}, t2{createTree(std::string{c2})};

        if (t1 && t2) {
            std::cout << "Comparison thingy!" << std::endl;
            std::cout << "Tree1:" << std::endl;
            for (const auto& node : t1)
                if (node.isObject())
                    std::cout << node.getNodePath() << std::endl << node << std::endl;
            // std::cout << "Tree2:" << std::endl;
            // for (const auto& node : t2)
            //     std::cout << node.getNodePath() << std::endl;
        }

        // // Create revision walker to iterate through repository
        // git_revwalk * walker = nullptr;
        // git_revwalk_new(&walker, repo.get());

        // // iteration rules
        // git_revwalk_sorting(walker, GIT_SORT_NONE);

        // git_revwalk_push_head(walker);

        // git_oid oid;

        // // while(!git_revwalk_next(&oid, walker))
        // // {
        // //     git_commit * commit = nullptr;
        // //     git_commit_lookup(&commit, repo.get(), &oid);

        // //     std::cout << git_oid_tostr_s(&oid) << " " << git_commit_summary(commit) << std::endl;

        // //     git_commit_free(commit);
        // // }

        // git_revwalk_free(walker);
    }

    

    GitTreeNode createTree(const std::string& rev) {
        git_object* obj{nullptr};
        git_tree* tree{nullptr};
        git_revparse_single(&obj, repo.get(), rev.c_str());
        git_object_peel(reinterpret_cast<git_object**>(&tree), obj, GIT_OBJECT_TREE);
        git_object_free(obj);
        return std::move(GitTreeNode{tree, repo, std::move(std::make_unique<std::filesystem::path>(repoPath))});
    }

    ~Comparitor() {}
};
}

#endif // COMPARITOR_H