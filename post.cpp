#include "post.h"
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::vector<Post> loadPostsFromFile() {
    std::vector<Post> posts;
    std::ifstream in("posts.json");
    if (!in.is_open()) return posts;
    json j;
    in >> j;
    for (const auto& item : j) {
        posts.emplace_back(
            item["id"].get<std::string>(),
            item["author"].get<std::string>(),
            item["text"].get<std::string>(),
            item["timestamp"].get<std::string>()
        );
    }
    return posts;
}

void savePostsToFile(const std::vector<Post>& posts) {
    json j = json::array();
    for (const auto& post : posts) {
        j.push_back({
            {"id", post.id},
            {"author", post.author},
            {"text", post.text},
            {"timestamp", post.timestamp}
        });
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