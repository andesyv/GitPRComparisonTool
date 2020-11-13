#include "gittreenode.h"

using namespace gprc;

GitTreeNode* GitTreeNode::append(std::string_view root) {
    // bool bRootNode = isRoot();
    if (isRoot() && root.empty()) {
        bLeaf = false;
        auto& node = *mChildren.emplace_back(std::make_shared<GitTreeNode>(*this));
        node.mParent = this;
        return &node;
    } else if (isRoot()) {
        auto it = begin() + 1;
        if (it != end()) {
            return (*it).append(root);
        }
    } else {
        std::string currentName{git_tree_entry_name(getTreeEntry())};
        std::string rootf{root};
        rootf.erase(rootf.find_last_of('/'));
        if (currentName == rootf) {
            // This suddenly became a branch node, thus it needs to create its own git_tree object.
            if (!mRepoObj.expired()) {
                bLeaf = false;
                git_tree* tree;
                if (git_tree_lookup(&tree, mRepoObj.lock().get(), &mOid))
                    throw std::runtime_error{"Failed to look up new subtree in leaf node creation."};
                mTreeObj.reset(tree, [](git_tree* p) { git_tree_free(p); });
            }

            auto& node = *mChildren.emplace_back(std::make_shared<GitTreeNode>(*this));
            node.mParent = this;
            return &node;
        } else {
            auto it = begin() + 1;
            if (it != end()) {
                return (*it).append(root);
            }
        }
    }
    return nullptr;
}

int GitTreeNode::construct_cb(const char *root, const git_tree_entry *entry, void *payload) {
    auto owner = static_cast<GitTreeNode*>(payload);
    // std::cout << "Root: " << root << std::endl;
    // std::string filename{git_tree_entry_name(entry)};
    // std::cout << "Filename: " << filename << std::endl;
    auto node = owner->append(root);
    if (node == nullptr)
        throw std::runtime_error{"Attempted to append to empty node (something went wrong with tree creation."};

    memcpy(&node->mOid, git_tree_entry_id(entry), sizeof(git_oid));
    // TODO: Fix this so that it continues to append on its last node instead of at the start (much less iteration required)
    // payload = node;
    return 0;
}

void GitTreeNode::construct() {
    if (git_tree_walk(mTreeObj.get(), git_treewalk_mode::GIT_TREEWALK_PRE, construct_cb, this))
        throw std::runtime_error{"Error while creating tree structure. :/"};
}

std::filesystem::path GitTreeNode::getNodePath() const {
    if (!isRoot()) {
        std::string filename{git_tree_entry_name(getTreeEntry())};
        return mParent->getNodePath() / filename;
    } else
        return *mRepoPath;
}

std::string GitTreeNode::getName() const {
    if (isRoot())
        return "root";
    else
        return git_tree_entry_name(getTreeEntry());
}

bool GitTreeNode::isObject() const {
    if (isRoot())
        return false;
    else if (!mRepoObj.expired()) {
        git_object* obj;
        if (git_object_lookup(&obj, mRepoObj.lock().get(), &mOid, git_object_t::GIT_OBJECT_ANY))
            return false;
        git_object_free(obj);
    }
    return true;
}

std::string GitTreeNode::print() const {
    if (!isLeaf())
        return "Folder";
    else {
        if (!mRepoObj.expired()) {
            git_object* obj;
            if (auto error = git_tree_entry_to_object(&obj, mRepoObj.lock().get(), getTreeEntry())) {
                return "undefined";
                // throw std::runtime_error{std::string{"Undefined tree entry. git_tree_entry_to_object Error code: "}.append(std::to_string(error))};
            }
            // std::cout << git_object_type2string(git_object_type(obj)) << std::endl;
            if (git_object_type(obj) == GIT_OBJECT_BLOB) {
                return std::string{
                    reinterpret_cast<const char*>(git_blob_rawcontent(reinterpret_cast<const git_blob*>(obj))),
                    static_cast<std::size_t>(git_blob_rawsize(reinterpret_cast<const git_blob*>(obj)))
                };
            }
            git_object_free(obj);
        }
    }

    return "undefined";
}

GitTreeNode::iterator& GitTreeNode::iterator::operator++() {
    // If the current node has a child node, go there.
    if (!mNode->mChildren.empty()) {
        mNode = mNode->mChildren.front().get();
        return *this;
    } else if (!mNode->isRoot()) {
        for (cItT currentIt; mNode->mParent != nullptr; mNode = mNode->mParent) {
            auto& pChildren = mNode->mParent->mChildren;

            // Note: Will always find something as it will on the least find itself.
            currentIt = std::find_if(pChildren.begin(), pChildren.end(), [&](auto ptr){
                return ptr.get() == mNode;
            });
        
            ++currentIt;
            // If we can traverse more on the current level, do that.
            if (currentIt != pChildren.end()) {
                mNode = currentIt->get();
                return *this;
            }
        }
    }

    // If we could'nt find anything and are at the top again, we done with iteration.
    mNode = nullptr;
    return *this;
}

GitTreeNode::iterator GitTreeNode::iterator::operator+(unsigned int inc) const {
    auto cp = *this;
    for (unsigned int i{0}; i < inc; ++i)
        ++cp;
    return cp;
}

std::unique_ptr<git_diff, GitTreeNode::git_diff_deleter> GitTreeNode::diff(const GitTreeNode& tree) {
    if (!mRepoObj.expired()) {
        git_diff* d{nullptr};
        if (auto err = git_diff_tree_to_tree(&d, mRepoObj.lock().get(), mTreeObj.get(), tree.mTreeObj.get(), NULL))
            throw std::runtime_error{std::string{"Git diff failed with error code: "}.append(std::to_string(err))};
        return std::unique_ptr<git_diff, git_diff_deleter>{d};
    }
    return {};
}