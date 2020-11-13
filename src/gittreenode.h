#ifndef GITTREENODE_H
#define GITTREENODE_H

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
#include <utility>
#include <vector>

namespace gprc {
class Comparitor;

// @see https://git-scm.com/book/en/v2/Git-Internals-Git-Objects#_tree_objects
class GitTreeNode {
friend class Comparitor;
private:
    std::shared_ptr<git_tree> mTreeObj;
    // std::weak_ptr<git_tree> mParentTreeObj;
    std::weak_ptr<git_repository> mRepoObj;
    git_oid mOid;
    std::vector<std::shared_ptr<GitTreeNode>> mChildren;
    GitTreeNode* mParent{nullptr};
    std::unique_ptr<std::filesystem::path> mRepoPath;
    bool bLeaf{true};

    GitTreeNode* append(std::string_view root);

    static int construct_cb(const char *root, const git_tree_entry *entry, void *payload);
    // Construct a tree structure
    void construct();
    
public:
    // GitTreeNode(std::shared_ptr<git_tree>&& tree) : mTreeObj{std::move(tree), [](git_tree* p) { git_tree_free(p); }} {
    //     construct();
    // }
    GitTreeNode(git_tree* tree, const std::weak_ptr<git_repository>& repo, std::unique_ptr<std::filesystem::path>&& path)
     : mTreeObj{tree, [](git_tree* p) { git_tree_free(p); }}, mRepoObj{repo}, mRepoPath{std::move(path)}, bLeaf{true} {
        construct();
    }
    GitTreeNode(GitTreeNode& other) : mTreeObj{other.mTreeObj}, mRepoObj{other.mRepoObj} {}
    GitTreeNode(GitTreeNode&& other)
     : mTreeObj{std::move(other.mTreeObj)}, mRepoObj{std::move(other.mRepoObj)}, mOid{other.mOid},
       mChildren{std::move(other.mChildren)}, mParent{other.mParent}, mRepoPath{std::move(other.mRepoPath)}, bLeaf{other.bLeaf}
    {
        // Redirect all child's parents nodes.
        for (auto& child : mChildren)
            child->mParent = this;
    }

    operator bool() const { return static_cast<bool>(mTreeObj); }
    bool isRoot() const { return mParent == nullptr; }
    bool isLeaf() const { return bLeaf; }
    const git_tree_entry* getTreeEntry() const { return git_tree_entry_byid(isLeaf() ? mTreeObj.get() : mParent->mTreeObj.get(), &mOid); }

    // GitTreeNode& find(std::string_view name) {
    //     for (auto& child : mChildren) {

    //     }
    // }

    std::filesystem::path getNodePath() const;
    std::string getName() const;

    bool isObject() const;
    std::string print() const;

    friend std::ostream& operator<<(std::ostream& out, const GitTreeNode& node) {
        return out << node.print();
    }

    class iterator {
    private:
        GitTreeNode* mNode{nullptr};
        typedef typename std::vector<std::shared_ptr<GitTreeNode>>::iterator cItT;

    public:
        iterator(GitTreeNode* ptr = nullptr) : mNode{ptr} {}
        iterator& operator++();
        iterator operator+(unsigned int inc) const;

        GitTreeNode& operator*() { return *mNode; }

        bool operator==(const iterator& rhs) const { return mNode == rhs.mNode; }
        bool operator!=(const iterator& rhs) const { return mNode != rhs.mNode; }
    };

    iterator begin() { return iterator{this}; }
    iterator end() { return iterator{nullptr}; }

    struct git_diff_deleter { void operator()(git_diff* p) { git_diff_free(p); }};
    std::unique_ptr<git_diff, git_diff_deleter> diff(const GitTreeNode& tree);

    auto operator/ (const GitTreeNode& rhs) { return this->diff(rhs); }

    // Get's all diffs between trees
    static auto diffs(GitTreeNode& t1, GitTreeNode& t2) {
        if (!t1.mRepoObj.expired() && !t2.mRepoObj.expired()) {
            std::vector<std::shared_ptr<git_tree>> trees[2];
            for (auto& node : t1)
                trees[0].push_back(node.mTreeObj);
            for (auto& node : t2)
                trees[1].push_back(node.mTreeObj);

            // Remove duplicates
            trees[0].erase(std::unique(trees[0].begin(), trees[0].end()), trees[0].end());
            trees[1].erase(std::unique(trees[1].begin(), trees[1].end()), trees[1].end());

            std::vector<std::unique_ptr<git_diff, git_diff_deleter>> ds;
            for (std::size_t i{0}; i < trees[0].size() && i < trees[1].size(); ++i) {
                git_diff* d;
                if (git_diff_tree_to_tree(&d, t1.mRepoObj.lock().get(), trees[0].at(i).get(), trees[0].at(i).get(), NULL))
                    continue;
                ds.emplace_back(d);
            }
            return std::move(ds);
        }
        return std::vector<std::unique_ptr<git_diff, git_diff_deleter>>{};
    }
};
}

#endif // GITTREENODE_H