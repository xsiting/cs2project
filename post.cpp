#include "post.h"
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Like functionality implementation
void Post::addLike(const std::string& username) {
    if (!isLikedBy(username)) {
        likedBy.push_back(username);
    }
}

void Post::removeLike(const std::string& username) {
    auto it = std::find(likedBy.begin(), likedBy.end(), username);
    if (it != likedBy.end()) {
        likedBy.erase(it);
    }
}

bool Post::isLikedBy(const std::string& username) const {
    return std::find(likedBy.begin(), likedBy.end(), username) != likedBy.end();
}

int Post::getLikeCount() const {
    return static_cast<int>(likedBy.size());
}

std::vector<Post> loadPostsFromFile() {
    std::vector<Post> posts;
    std::ifstream in("posts.json");
    if (!in.is_open()) return posts;
    json j;
    in >> j;
    for (const auto& item : j) {
        Post post(
            item["id"].get<std::string>(),
            item["author"].get<std::string>(),
            item["text"].get<std::string>(),
            item["timestamp"].get<std::string>()
        );
        
        // Load likes if they exist (for backward compatibility)
        if (item.contains("likedBy")) {
            for (const auto& like : item["likedBy"]) {
                post.addLike(like.get<std::string>());
            }
        }
        
        posts.push_back(post);
    }
    return posts;
}

void savePostsToFile(const std::vector<Post>& posts) {
    json j = json::array();
    for (const auto& post : posts) {
        json postJson = {
            {"id", post.id},
            {"author", post.author},
            {"text", post.text},
            {"timestamp", post.timestamp},
            {"likedBy", post.likedBy}
        };
        j.push_back(postJson);
    }
    std::ofstream out("posts.json");
    out << std::setw(2) << j;
}

std::string getCurrentTimestamp() {
    std::time_t now = std::time(nullptr);
    std::tm* ptm = std::gmtime(&now);
    char buffer[32];
    std::strftime(buffer, 32, "%Y-%m-%dT%H:%M:%SZ", ptm);
    return std::string(buffer);
} 