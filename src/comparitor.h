#ifndef COMPARITOR_H
#define COMPARITOR_H

#include <string>
#include <string_view>
#include <filesystem>
extern "C" {
    #include <git2.h>
    #include <git2/pathspec.h>
};
#include <memory>
#include <stdexcept>
#include "gittreenode.h"
#include <fstream>
#include <set>
#include <matplot/matplot.h>

// http://blog.davidecoppola.com/2016/10/how-to-traverse-git-repository-using-libgit2-and-cpp/
// ^ For basic libgit usage
namespace gprc {

class Comparitor {
private:
    std::filesystem::path mRepoPath;
    struct git_repo_deleter { void operator()(git_repository* ptr){ git_repository_free(ptr); }};
    std::shared_ptr<git_repository> repo;

public:
    Comparitor(const std::filesystem::path& path = std::filesystem::current_path()) : mRepoPath{path} {
        // https://github.com/libgit2/libgit2/blob/master/tests/diff/tree.c
        // Open repository into repository object
        git_repository* r{nullptr};
        if (!git_repository_open(&r, mRepoPath.string().c_str()))
            repo = std::shared_ptr<git_repository>{r, [](git_repository* ptr){ git_repository_free(ptr); }};
        else
            throw std::runtime_error{"Failed to find specified git repository!"};
    }

    // Removed lines vs added lines
    struct PlotInfo {
        std::string filename;
        std::size_t removed;
        std::size_t added;
    };
    std::vector<PlotInfo> lineChanges;
    std::ofstream out;

    void setOutput(const std::filesystem::path& file) {
        out.open(file, std::ofstream::trunc);
    }

    std::filesystem::path repoPath() const { return mRepoPath; }

    void compare(const std::string_view& c1, const std::string_view& c2) {
        // auto t1{createTree(std::string{c1})}, t2{createTree(std::string{c2})};

        // if (t1 && t2) {
            std::cout << "Comparison thingy!" << std::endl;
            // std::cout << "Tree1:" << std::endl;
            // for (const auto& node : t1)
            //     if (node.isObject())
            //         std::cout << node.getNodePath() << std::endl << node << std::endl;
            out << "<html><head><style>" << R"(
        #code, #plots {
            display: inline-block;
            *display: inline;
            vertical-align: top;
        }
        #code {width: 70%;}
        #plots {width: 29%;})";
            out << "</style></head><body><div style=\"width:100%;height:100%\"><div id=\"code\"><pre>" << std::endl;
            auto file_cb = [](const git_diff_delta* delta, float progress, void* payload){
                auto& owner = *reinterpret_cast<Comparitor*>(payload);
                owner.lineChanges.push_back({delta->new_file.path, 0, 0});
                owner.out << delta->new_file.path << ": " << std::endl;
                return 0;
            };
            auto binary_cb = [](const git_diff_delta* delta, const git_diff_binary* binary, void* payload) {
                // std::cout << "binary_cb!: " << std::endl;
                return 0;
            };
            auto hunk_cb = [](const git_diff_delta* delta, const git_diff_hunk* hunk, void* payload) {
                // std::cout << "hunk_cb!: " << std::endl;
                return 0;
            };
            auto line_cb = [](const git_diff_delta* delta, const git_diff_hunk* hunk, const git_diff_line* line, void* payload) {
                // std::cout << "line_cb!: " << std::endl;
                auto& owner = *reinterpret_cast<Comparitor*>(payload);
                if (line->new_lineno < 0) {
                    owner.out << "<span style=\"color:red;\">";
                    if (!owner.lineChanges.empty())
                        owner.lineChanges.back().removed += line->content_len;
                } else if (line->old_lineno < 0) {
                    owner.out << "<span style=\"color:green;\">";
                    if (!owner.lineChanges.empty())
                        owner.lineChanges.back().added += line->content_len;
                } else
                    owner.out << "<span>";
                
                owner.out << line->old_lineno << "|" << line->new_lineno << ": " << std::string{line->content, line->content_len} << "</span>";
                return 0;
            };

            auto makeTree = [&](const std::string& rev) {
                git_object* obj{nullptr};
                git_tree* tree{nullptr};
                git_revparse_single(&obj, repo.get(), rev.c_str());
                git_object_peel(reinterpret_cast<git_object**>(&tree), obj, GIT_OBJECT_TREE);
                git_object_free(obj);
                return std::shared_ptr<git_tree>{tree, [](git_tree* t){ git_tree_free(t); }};
            };
            auto getDiff = [&](const std::shared_ptr<git_tree>& t1, const std::shared_ptr<git_tree>& t2) {
                git_diff* d{nullptr};
                if (auto err = git_diff_tree_to_tree(&d, repo.get(), t1.get(), t2.get(), NULL))
                    throw std::runtime_error{std::string{"Git diff failed with error code: "}.append(std::to_string(err))};
                return std::shared_ptr<git_diff>{d, [](git_diff* p){ git_diff_free(p); }};
            };



            lineChanges.clear();
            auto t1{makeTree(std::string{c1})}, t2{makeTree(std::string{c2})};
            auto diff = getDiff(t1, t2);
            if (auto err = git_diff_foreach(diff.get(), file_cb, binary_cb, hunk_cb, line_cb, this))
                throw std::runtime_error{std::string{"git_diff_foreach failed with error: "}.append(std::to_string(err))};

            out << "</pre></div>\n<div id=\"plots\">";
            // Make some plots:

            for (const auto& lines : lineChanges) {
                std::cout << "lines removed: " << lines.removed << ", lines added: " << lines.added << std::endl;
                std::vector<std::vector<double>> Y{
                    {static_cast<double>(lines.removed), 0}, {0, static_cast<double>(lines.added)}
                };

                auto f = matplot::gcf();
                // f->width(f->width() * 2);

                matplot::subplot(1, 1, 0);
                auto h = matplot::area(Y, 0.0, false);
                h[0]->face_color({0.25, 0.0, 1.0, 0.0});
                h[1]->face_color({0.25, 1.0, 0.0, 0.0});
                matplot::title(lines.filename);

                std::string filename{"diff_figures/"};
                filename += std::filesystem::path{lines.filename}.replace_extension(".svg").filename().string();
                matplot::save(filename);

                out << "<img src=\"" << filename << "\" alt=\"Symbol changes of diff\">" << std::endl;
            }

            out << "</div></div></body></html>";
        // }
    }
   

    GitTreeNode createTree(const std::string& rev) {
        git_object* obj{nullptr};
        git_tree* tree{nullptr};
        git_revparse_single(&obj, repo.get(), rev.c_str());
        git_object_peel(reinterpret_cast<git_object**>(&tree), obj, GIT_OBJECT_TREE);
        git_object_free(obj);
        return std::move(GitTreeNode{tree, repo, std::move(std::make_unique<std::filesystem::path>(mRepoPath))});
    }

    ~Comparitor() {}
};
}

#endif // COMPARITOR_H