#pragma once
#include <string>
#include <vector>

struct Post {
    std::string id;
    std::string author;
    std::string text;
    std::string timestamp;

    Post(const std::string& id, const std::string& author, const std::string& text, const std::string& timestamp)
        : id(id), author(author), text(text), timestamp(timestamp) {}
    Post() = default;
};

std::vector<Post> loadPostsFromFile();
void savePostsToFile(const std::vector<Post>& posts);
std::string getCurrentTimestamp();
