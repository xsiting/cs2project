#include "utility.h"
#include <queue>
#include <set>
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
    // Find user
    auto it = std::find_if(users.begin(), users.end(), [&](const User& u) { return u.getUsername() == username; });
    std::vector<std::string> friendNames;
    if (it != users.end()) {
        friendNames = it->getFriends();
    }
    friendNames.push_back(username); // include self
    // Use a vector to collect posts
    std::vector<Post> timeline;
    for (const auto& post : allPosts) {
        if (std::find(friendNames.begin(), friendNames.end(), post.author) != friendNames.end()) {
            timeline.push_back(post);
        }
    }
    // Sort by timestamp descending
    std::sort(timeline.begin(), timeline.end(), [](const Post& a, const Post& b) {
        return a.timestamp > b.timestamp;
    });
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
    auto itFrom = std::find_if(users.begin(), users.end(), [&](const User& u){return u.getUsername() == from;});
    auto itTo = std::find_if(users.begin(), users.end(), [&](const User& u){return u.getUsername() == to;});
    if (itFrom != users.end() && itTo != users.end()) {
        itFrom->addFriend(to);
        itTo->addFriend(from);
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
    std::cerr << "[DEBUG] Before remove_if in cancelFriendRequest, size: " << requests.size() << std::endl;
    auto it = std::remove_if(requests.begin(), requests.end(), [&](const FriendRequest& req){
        return req.from == from && req.to == to && req.status == "pending";
    });
    std::cerr << "[DEBUG] After remove_if, it - begin: " << (it - requests.begin()) << ", end - begin: " << (requests.end() - requests.begin()) << std::endl;
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
// Mutual friends
std::vector<std::string> getMutualFriends(const User& a, const User& b) {
    std::set<std::string> af(a.getFriends().begin(), a.getFriends().end());
    std::vector<std::string> result;
    for (const auto& f : b.getFriends()) {
        if (af.count(f)) result.push_back(f);
    }
    return result;
}
// Friend suggestions: users who are not friends but share the most mutual friends
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
    std::set<std::string> myFriends(myFriendsVec.begin(), myFriendsVec.end());
    std::map<std::string, int> suggestionCounts;
    for (const auto& other : allUsers) {
        if (other.getUsername() == user.getUsername() || myFriends.count(other.getUsername())) continue;
        std::vector<std::string> otherFriendsVec = other.getFriends();
        if (otherFriendsVec.empty()) continue;
        std::cerr << "[DEBUG] checking other user: " << other.getUsername() << ", friends size: " << otherFriendsVec.size() << " [";
        for (const auto& f : otherFriendsVec) std::cerr << f << ",";
        std::cerr << "]" << std::endl;
        int mutual = getMutualFriends(user, other).size();
        if (mutual > 0) suggestionCounts[other.getUsername()] = mutual;
    }
    std::cerr << "[DEBUG] suggestionCounts size: " << suggestionCounts.size() << std::endl;
    std::vector<std::pair<std::string, int>> sorted;
    for (const auto& p : suggestionCounts) sorted.push_back(p);
    std::cerr << "[DEBUG] sorted vector size: " << sorted.size() << std::endl;
    if (!sorted.empty()) {
        std::sort(sorted.begin(), sorted.end(), [](const auto& a, const auto& b){return a.second > b.second;});
    }
    std::vector<std::string> result;
    for (const auto& p : sorted) result.push_back(p.first);
    std::cerr << "[DEBUG] suggestFriends result size: " << result.size() << std::endl;
    return result;
}