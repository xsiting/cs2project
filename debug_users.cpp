#include "user.h"
#include "utility.h"
#include <iostream>

int main() {
    std::cout << "=== User Debug Information ===\n\n";
    
    auto users = loadUsersFromFile();
    std::cout << "Total users in system: " << users.size() << "\n\n";
    
    for (const auto& user : users) {
        std::cout << "User: " << user.getUsername() << "\n";
        auto friends = user.getFriends();
        std::cout << "  Friends (" << friends.size() << "): ";
        if (friends.empty()) {
            std::cout << "None";
        } else {
            for (const auto& friend_name : friends) {
                std::cout << friend_name << " ";
            }
        }
        std::cout << "\n\n";
    }
    
    return 0;
} 