#include "utility.h"
#include <queue>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

vector<User> loadUsersFromFile(const string& filename) {
    vector<User> users;
    ifstream inFile(filename);

    if (!inFile.is_open()) {
        cerr << "Could not open " << filename << ". Returning empty user list.\n";
        return users;
    }

    json data;
    inFile >> data;

    for (auto& item : data.items()) {
        users.push_back(User::from_json(item.value()));
    }

    return users;
}

void saveUsersToFile(const vector<User>& users, const string& filename) {
    
    std::cerr << "saveUsersToFile: saving " << users.size() 
          << " users to " << filename << std::endl;
    json data = json::object();

    for (const auto& user : users) {
        data[user.getUsername()] = user.to_json();
    }

    ofstream outFile(filename);
    if (!outFile.is_open()) {
        cerr << "Could not write to " << filename << "\n";
        return;
    }
    
    std::cerr << data.dump(4) << std::endl;

    outFile << data.dump(4);
}

User* findUserByUsername(vector<User>& users, const string& username) {
    for (auto& user : users) {
        if (user.getUsername() == username)
            return &user;
    }
    return nullptr;
}

std::vector<std::string> getFriendUsernames(const User& user) {
    return user.getFriends();
}

std::vector<Post> getTimelinePosts(const std::string& username, const std::vector<User>& users, const std::vector<Post>& allPosts) {
    // Find user using findUserByUsername
    User* user = findUserByUsername(const_cast<std::vector<User>&>(users), username);
    std::vector<std::string> friendNames;
    if (user) {
        friendNames = user->getFriends();
    }
    friendNames.push_back(username); // include self
    // Use a vector to collect posts
    std::vector<Post> timeline;
    for (const auto& post : allPosts) {
        if (std::find(friendNames.begin(), friendNames.end(), post.author) != friendNames.end()) {
            timeline.push_back(post);
        }
    }
    // Sort by timestamp descending using overloaded operator>
    std::sort(timeline.begin(), timeline.end(), std::greater<Post>());
    return timeline;
}

// Friend request management
std::vector<FriendRequest> loadFriendRequests(const std::string& filename) {
    std::vector<FriendRequest> requests;
    std::ifstream in(filename);
    if (!in.is_open()) return requests;
    json j;
    in >> j;
    for (const auto& item : j) {
        requests.push_back({item["from"], item["to"], item["status"]});
    }
    return requests;
}
void saveFriendRequests(const std::vector<FriendRequest>& requests, const std::string& filename) {
    json j = json::array();
    for (const auto& req : requests) {
        j.push_back({{"from", req.from}, {"to", req.to}, {"status", req.status}});
    }
    std::ofstream out(filename);
    out << std::setw(2) << j;
}
void sendFriendRequest(const std::string& from, const std::string& to) {
    auto requests = loadFriendRequests();
    for (const auto& req : requests) {
        if (req.from == from && req.to == to && req.status == "pending") return; // already sent
        else if (req.from == to && req.to == from && req.status == "accepted") {
            std::cerr << "[DEBUG] Friend request already accepted between " << from << " and " << to << std::endl;
            return; // already friends
        }
    }
    requests.push_back({from, to, "pending"});
    saveFriendRequests(requests);
}
void acceptFriendRequest(const std::string& from, const std::string& to) {
    auto requests = loadFriendRequests();
    for (auto& req : requests) {
        if (req.from == from && req.to == to && req.status == "pending") {
            req.status = "accepted";
            break;
        }
    }
    saveFriendRequests(requests);
    // Add each other as friends
    auto users = loadUsersFromFile();
    User* fromUser = findUserByUsername(users, from);
    User* toUser = findUserByUsername(users, to);
    if (fromUser && toUser) {
        fromUser->addFriend(to);
        toUser->addFriend(from);
        saveUsersToFile(users);
    }
}
void rejectFriendRequest(const std::string& from, const std::string& to) {
    auto requests = loadFriendRequests();
    for (auto& req : requests) {
        if (req.from == from && req.to == to && req.status == "pending") {
            req.status = "rejected";
            break;
        }
    }
    saveFriendRequests(requests);
}
void cancelFriendRequest(const std::string& from, const std::string& to) {
    auto requests = loadFriendRequests();
    std::cerr << "[DEBUG] Before remove in cancelFriendRequest, size: " << requests.size() << std::endl;
    
    // Create a FriendRequest object to match against
    FriendRequest toRemove(from, to, "pending");
    
    auto it = std::remove(requests.begin(), requests.end(), toRemove);
    std::cerr << "[DEBUG] After remove, it - begin: " << (it - requests.begin()) << ", end - begin: " << (requests.end() - requests.begin()) << std::endl;
    requests.erase(it, requests.end());
    std::cerr << "[DEBUG] After erase, size: " << requests.size() << std::endl;
    saveFriendRequests(requests);
}
std::vector<std::string> getPendingRequestsForUser(const std::string& username) {
    auto requests = loadFriendRequests();
    std::vector<std::string> result;
    for (const auto& req : requests) {
        if (req.to == username && req.status == "pending") result.push_back(req.from);
    }
    return result;
}
std::vector<std::string> getSentRequestsByUser(const std::string& username) {
    auto requests = loadFriendRequests();
    std::vector<std::string> result;
    for (const auto& req : requests) {
        if (req.from == username && req.status == "pending") result.push_back(req.to);
    }
    return result;
}
// Mutual friends (no std::set)
std::vector<std::string> getMutualFriends(const User& a, const User& b) {
    auto aFriends = a.getFriends();
    auto bFriends = b.getFriends();
    std::vector<std::string> result;
    for (const auto& f : bFriends) {
        if (std::find(aFriends.begin(), aFriends.end(), f) != aFriends.end()) {
            result.push_back(f);
        }
    }
    return result;
}

// Enhanced mutual friends using AVL tree operations
std::vector<std::string> getMutualFriendsEfficient(const User& a, const User& b) {
    auto aFriends = a.getFriends();
    std::vector<std::string> mutualFriends;
    
    // Use AVL tree's efficient search for each friend of user b
    for (const auto& friendName : aFriends) {
        if (b.isFriend(friendName)) {
            mutualFriends.push_back(friendName);
        }
    }
    
    return mutualFriends;
}

// Friend suggestions: users who are not friends but share the most mutual friends (no std::set or std::map)
std::vector<std::string> suggestFriends(const User& user, const std::vector<User>& allUsers) {
    std::cerr << "[DEBUG] suggestFriends called for user: " << user.getUsername() << std::endl;
    std::vector<std::string> myFriendsVec = user.getFriends();
    if (myFriendsVec.empty()) {
        std::cerr << "[DEBUG] myFriendsVec is empty, skipping suggestions.\n";
        return {};
    }
    std::cerr << "[DEBUG] myFriends size: " << myFriendsVec.size() << " [";
    for (const auto& f : myFriendsVec) std::cerr << f << ",";
    std::cerr << "]" << std::endl;
    std::vector<std::pair<std::string, int>> suggestionCounts;
    for (const auto& other : allUsers) {
        if (other.getUsername() == user.getUsername()) continue;
        // Check if already a friend using AVL tree search
        if (user.isFriend(other.getUsername())) continue;
        std::vector<std::string> otherFriendsVec = other.getFriends();
        if (otherFriendsVec.empty()) continue;
        std::cerr << "[DEBUG] checking other user: " << other.getUsername() << ", friends size: " << otherFriendsVec.size() << " [";
        for (const auto& f : otherFriendsVec) std::cerr << f << ",";
        std::cerr << "]" << std::endl;
        int mutual = getMutualFriendsEfficient(user, other).size();
        if (mutual > 0) suggestionCounts.push_back({other.getUsername(), mutual});
    }
    std::cerr << "[DEBUG] suggestionCounts size: " << suggestionCounts.size() << std::endl;
    if (!suggestionCounts.empty()) {
        std::sort(suggestionCounts.begin(), suggestionCounts.end(), [](const auto& a, const auto& b){return a.second > b.second;});
    }
    std::vector<std::string> result;
    for (const auto& p : suggestionCounts) result.push_back(p.first);
    std::cerr << "[DEBUG] suggestFriends result size: " << result.size() << std::endl;
    return result;
}

// Enhanced friend suggestions based on mutual friend count using AVL tree operations
std::vector<std::string> suggestFriendsByMutualCount(const User& user, const std::vector<User>& allUsers) {
    std::vector<std::pair<std::string, int>> suggestions = getFriendSuggestionsWithScores(user, allUsers);
    std::vector<std::string> result;
    for (const auto& suggestion : suggestions) {
        result.push_back(suggestion.first);
    }
    return result;
}

// Get second-degree connections (friends of friends) using AVL tree level-order traversal
std::vector<std::string> getSecondDegreeConnections(const User& user, const std::vector<User>& allUsers) {
    std::vector<std::string> secondDegree;
    std::vector<std::string> myFriends = user.getFriends();
    
    // For each friend, get their friends using AVL tree operations
    for (const auto& friendName : myFriends) {
        auto it = std::find_if(allUsers.begin(), allUsers.end(), 
                              [&](const User& u) { return u.getUsername() == friendName; });
        if (it != allUsers.end()) {
            auto friendsOfFriend = it->getFriends();
            for (const auto& friendOfFriend : friendsOfFriend) {
                // Don't include self or direct friends
                if (friendOfFriend != user.getUsername() && !user.isFriend(friendOfFriend)) {
                    // Avoid duplicates
                    if (std::find(secondDegree.begin(), secondDegree.end(), friendOfFriend) == secondDegree.end()) {
                        secondDegree.push_back(friendOfFriend);
                    }
                }
            }
        }
    }
    
    return secondDegree;
}

// Get friend suggestions with scores using AVL tree operations
std::vector<std::pair<std::string, int>> getFriendSuggestionsWithScores(const User& user, const std::vector<User>& allUsers) {
    std::vector<std::pair<std::string, int>> suggestions;
    
    for (const auto& other : allUsers) {
        if (other.getUsername() == user.getUsername()) continue;
        
        // Skip if already friends
        if (user.isFriend(other.getUsername())) continue;
        
        // Calculate mutual friends count using efficient AVL operations
        int mutualCount = getMutualFriendsEfficient(user, other).size();
        
        // Calculate second-degree connection bonus
        auto secondDegree = getSecondDegreeConnections(user, allUsers);
        bool isSecondDegree = std::find(secondDegree.begin(), secondDegree.end(), other.getUsername()) != secondDegree.end();
        
        // Score calculation: mutual friends + bonus for second-degree connections
        int score = mutualCount;
        if (isSecondDegree) score += 1; // Bonus for being a friend of a friend
        
        if (score > 0) {
            suggestions.push_back({other.getUsername(), score});
        }
    }
    
    // Sort by score (descending)
    std::sort(suggestions.begin(), suggestions.end(), 
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    return suggestions;
}