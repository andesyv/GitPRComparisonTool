#ifndef COMPARITOR_H
#define COMPARITOR_H

#include <git2.h>
#include <string>
#include <string_view>
#include <filesystem>
#include <git2/revparse.h>
#include <memory>
#include <stdexcept>

// http://blog.davidecoppola.com/2016/10/how-to-traverse-git-repository-using-libgit2-and-cpp/
// ^ For basic libgit usage


// class GitTree {
// public:
//     GitTree(git_repository& repo, const char* rev) {
//         git_object* obj{nullptr};
//         git_revparse_single(&obj, &repo, rev);
//         git_object_peel(&obj, obj, GIT_OBJECT_TREE);
//     }
// };

class Comparitor {
private:
    std::filesystem::path repoPath;
    struct git_repo_deleter { void operator()(git_repository* ptr){ git_repository_free(ptr); }};
    std::unique_ptr<git_repository, git_repo_deleter> repo;

public:
    Comparitor(const std::filesystem::path& path = std::filesystem::current_path()) : repoPath{path} {
        // https://github.com/libgit2/libgit2/blob/master/tests/diff/tree.c
        // Open repository into repository object
        git_repository* r{nullptr};
        if (!git_repository_open(&r, repoPath.string().c_str()))
            repo.reset(r);
        else
            throw std::runtime_error{"Failed to find specified git repository!"};
    }

    void compare(const std::string_view& c1, const std::string_view& c2) {
        auto t1{getTree(std::string{c1})}, t2{getTree(std::string{c2})};

        if (t1 && t2) {
            std::cout << "Comparison thingy!" << std::endl;
            
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

    std::shared_ptr<git_tree> getTree(const std::string& rev) {
        git_object* obj{nullptr};
        git_tree* tree{nullptr};
        git_revparse_single(&obj, repo.get(), rev.c_str());
        git_object_peel(reinterpret_cast<git_object**>(&tree), obj, GIT_OBJECT_TREE);
        git_object_free(obj);
        return {tree, [](git_tree* p){ git_tree_free(p); }};
    }

    ~Comparitor() {}
};

#endif // COMPARITOR_H