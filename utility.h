#pragma once
#include <fstream>
#include <vector>
#include <iostream>
#include "user.h"
#include <nlohmann/json.hpp>
#include "post.h"

using namespace std;
using json = nlohmann::json;

vector<User> loadUsersFromFile(const string& filename = "../../users.json");
void saveUsersToFile(const vector<User>& users, const string& filename = "../../users.json");

User* findUserByUsername(vector<User>& users, const string& username);

std::vector<std::string> getFriendUsernames(const User& user);
std::vector<Post> getTimelinePosts(const std::string& username, const std::vector<User>& users, const std::vector<Post>& allPosts);

// Friend request management
struct FriendRequest {
    std::string from;
    std::string to;
    std::string status; // "pending", "accepted", "rejected"

    // Add comparison operator
    bool operator==(const FriendRequest& other) const {
        return from == other.from && 
               to == other.to && 
               status == other.status;
    }

    // Add a constructor for easy comparison
    FriendRequest(const std::string& f, const std::string& t, const std::string& s)
        : from(f), to(t), status(s) {}
};
std::vector<FriendRequest> loadFriendRequests(const std::string& filename = "../../friend_requests.json");
void saveFriendRequests(const std::vector<FriendRequest>& requests, const std::string& filename = "../../friend_requests.json");
// Friend request operations
void sendFriendRequest(const std::string& from, const std::string& to);
void acceptFriendRequest(const std::string& from, const std::string& to);
void rejectFriendRequest(const std::string& from, const std::string& to);
void cancelFriendRequest(const std::string& from, const std::string& to);
std::vector<std::string> getPendingRequestsForUser(const std::string& username);
std::vector<std::string> getSentRequestsByUser(const std::string& username);

// Enhanced mutual friends and suggestions using AVL tree operations
std::vector<std::string> getMutualFriends(const User& a, const User& b);
std::vector<std::string> getMutualFriendsEfficient(const User& a, const User& b);
std::vector<std::string> suggestFriends(const User& user, const std::vector<User>& allUsers);
std::vector<std::string> suggestFriendsByMutualCount(const User& user, const std::vector<User>& allUsers);
std::vector<std::string> getSecondDegreeConnections(const User& user, const std::vector<User>& allUsers);
std::vector<std::pair<std::string, int>> getFriendSuggestionsWithScores(const User& user, const std::vector<User>& allUsers);