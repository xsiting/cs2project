#pragma once
#include <string>
#include <vector>
#include <algorithm>

struct Post {
    std::string id;
    std::string author;
    std::string text;
    std::string timestamp;
    std::vector<std::string> likedBy;

    Post(const std::string& id, const std::string& author, const std::string& text, const std::string& timestamp)
        : id(id), author(author), text(text), timestamp(timestamp), likedBy() {}
    Post() = default;
    
    // Like functionality
    void addLike(const std::string& username);
    void removeLike(const std::string& username);
    bool isLikedBy(const std::string& username) const;
    int getLikeCount() const;

    // Comparison operators for sorting by timestamp
    bool operator>(const Post& other) const {
        return timestamp > other.timestamp;
    }
    bool operator<(const Post& other) const {
        return timestamp < other.timestamp;
    }
};

std::vector<Post> loadPostsFromFile();
void savePostsToFile(const std::vector<Post>& posts);
std::string getCurrentTimestamp();
