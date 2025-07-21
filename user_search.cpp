#include "user_search.h"
#include "user.h"
#include <memory>

// Global search tree instance
static UserSearchTree userSearchTree;

void updateSearchIndex(const std::vector<User>& users) {
    userSearchTree.clear();
    for (const auto& user : users) {
        userSearchTree.insert(user.getUsername());
    }
}

std::vector<std::string> searchUsers(const std::string& prefix) {
    return userSearchTree.searchByPrefix(prefix);
} 