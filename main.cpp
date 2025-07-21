#include "crow_all.h"
#include "user.h"
#include "utility.h"            
#include <fstream>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <unordered_map>
#include <random>
#include "post.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;
#include <cctype>

std::unordered_map<std::string, std::string> sessionMap;

// Function to generate a random alphanumeric token
std::string generateToken(size_t length = 32) {
    static const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<> dist(0, sizeof(charset) - 2);

    std::string token;
    for (size_t i = 0; i < length; ++i)
        token += charset[dist(rng)];
    return token;
}

// Helper to get the absolute path to the static directory
std::string getStaticPath(const std::string& filename) {
    // Always resolve relative to the project root static/ directory
    std::filesystem::path cwd = std::filesystem::current_path();
    std::filesystem::path staticDir;
    // Try to find the static directory up to 3 levels up
    for (int i = 0; i < 4; ++i) {
        if (std::filesystem::exists(cwd / "static" / filename)) {
            staticDir = cwd / "static" / filename;
            break;
        }
        cwd = cwd.parent_path();
    }
    return staticDir.string();
}

std::string loadFile(const std::string& filename) {
    std::string path = getStaticPath(filename);
    std::cerr << "Trying to load: " << path << std::endl;
    std::ifstream in(path);
    if (!in.is_open()) {
        std::cerr << "Failed to open file: " << path << "\n";
        return "<h1>File Not Found</h1>";
    }
    std::ostringstream contents;
    contents << in.rdbuf();
    return contents.str();
}



std::string hashPassword(const std::string& password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password.c_str(), password.size(), hash);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];

    return ss.str();
}


std::pair<std::string, std::string> parseForm(const std::string& body) {
    std::string username, password;
    auto pos_user = body.find("username=");
    auto pos_pass = body.find("&password=");

    if (pos_user != std::string::npos && pos_pass != std::string::npos) {
        username = body.substr(pos_user + 9, pos_pass - (pos_user + 9));
        password = body.substr(pos_pass + 10);

        std::replace(username.begin(), username.end(), '+', ' ');
        std::replace(password.begin(), password.end(), '+', ' ');
    }

    return {username, password};
}

// Helper to extract username from session cookie
std::string getUsernameFromSession(const crow::request& req) {
    auto cookie = req.get_header_value("Cookie");
    auto pos = cookie.find("session=");
    if (pos == std::string::npos) return "";
    auto end = cookie.find(';', pos);
    std::string token = cookie.substr(pos + 8, end == std::string::npos ? std::string::npos : end - (pos + 8));
    if (sessionMap.count(token)) return sessionMap[token];
    return "";
}

// Helper to decode URL-encoded form data
std::string urlDecode(const std::string& str) {
    std::string ret;
    char ch;
    int i, ii;
    for (i = 0; i < str.length(); i++) {
        if (str[i] == '%') {
            sscanf(str.substr(i + 1, 2).c_str(), "%x", &ii);
            ch = static_cast<char>(ii);
            ret += ch;
            i = i + 2;
        } else if (str[i] == '+') {
            ret += ' ';
        } else {
            ret += str[i];
        }
    }
    return ret;
}


int main() {
    crow::SimpleApp app;

    std::cout << "\n==============================\n";
    std::cout << "Server starting on http://localhost:18080/" << std::endl;
    std::cout << "==============================\n";
    std::cerr << "Current dir: " << std::filesystem::current_path() << "\n";

    // Serve static files (CSS, JS, images, etc.)
    CROW_ROUTE(app, "/static/<string>")
    ([](const crow::request& req, crow::response& res, std::string filename){
        std::string path = getStaticPath(filename);
        std::ifstream in(path, std::ios::binary);
        if (!in) {
            res.code = 404;
            res.end();
            return;
        }
        std::ostringstream contents;
        contents << in.rdbuf();
        res.write(contents.str());
        res.end();
    });

    CROW_ROUTE(app, "/signup").methods("GET"_method)([] {
        std::cerr << "[DEBUG] /signup GET called\n";
        return crow::response(loadFile("signup.html"));
    });
    CROW_ROUTE(app, "/login").methods("GET"_method)([] {
        std::cerr << "[DEBUG] /login GET called\n";
        return crow::response(loadFile("login.html"));
    });

    
    CROW_ROUTE(app, "/signup").methods("POST"_method)([](const crow::request& req) {
        std::cerr << "[DEBUG] /signup POST called\n";
        std::cerr << "Body: " << req.body << "\n";

        auto [username, password] = parseForm(req.body);
        if (username.empty() || password.empty()) {
            return crow::response(400, "Missing credentials");
        }

        auto users = loadUsersFromFile();
        if (findUserByUsername(users, username)) {
            return crow::response(400, "Username already exists");
        }

        std::string hashed = hashPassword(password);
        users.push_back(User(username, hashed));
        saveUsersToFile(users);

        crow::response res(303);
        res.add_header("Location", "/login");
        return res;
    });



    CROW_ROUTE(app, "/login").methods("POST"_method)([](const crow::request& req) {
        std::cerr << "[DEBUG] /login POST called\n";
        std::cerr << "Body: " << req.body << "\n";

        auto [username, password] = parseForm(req.body);
        if (username.empty() || password.empty()) {
            return crow::response(400, "Missing credentials");
        }

        auto users = loadUsersFromFile();
        User* user = findUserByUsername(users, username);

        if (!user || user->getPasswordHash() != hashPassword(password)) {
            return crow::response(401, "Invalid credentials");
        }
        std::string token = generateToken();
        sessionMap[token] = username;
        

    crow::response res(303);
    std::cerr << "Token is " << token << endl;
    res.add_header("Set-Cookie", "session=" + token + "; Path=/; HttpOnly");
    res.add_header("Location", "/dashboard");  // Redirect after login
    return res;
    });

    CROW_ROUTE(app, "/post").methods("POST"_method)([](const crow::request& req) {
        std::cerr << "[DEBUG] /post POST called\n";
        std::string username = getUsernameFromSession(req);
        if (username.empty()) return crow::response(401, "Not logged in");
        auto body = req.body;
        auto pos = body.find("text=");
        if (pos == std::string::npos) return crow::response(400, "Missing text");
        std::string text = body.substr(pos + 5);
        text = urlDecode(text);
        auto posts = loadPostsFromFile();
        std::string id = generateToken(8);
        std::string timestamp = getCurrentTimestamp();
        posts.emplace_back(id, username, text, timestamp);
        savePostsToFile(posts);
        crow::response res(303);
        res.add_header("Location", "/dashboard");
        return res;
    });

    CROW_ROUTE(app, "/timeline").methods("GET"_method)([](const crow::request& req) {
        std::cerr << "[DEBUG] /timeline GET called\n";
        std::string username = getUsernameFromSession(req);
        if (username.empty()) return crow::response(401, "Not logged in");
        auto users = loadUsersFromFile();
        auto allPosts = loadPostsFromFile();
        auto timeline = getTimelinePosts(username, users, allPosts);
        std::ostringstream html;
        for (const auto& post : timeline) {
            html << "<div class='post'><h2>" << post.author << "</h2><p>" << post.text << "</p><d>" << post.timestamp << "</d>";
            
            // Add like button and count
            bool isLiked = post.isLikedBy(username);
            html << "<div class='post-actions'>";
            html << "<button class='like-btn " << (isLiked ? "liked" : "") << "' data-post-id='" << post.id << "'>";
            html << "<i class='fa-solid fa-heart'></i> <span class='like-count'>" << post.getLikeCount() << "</span>";
            html << "</button>";
            html << "</div>";
            
            if (post.author == username) {
                html << "<form method='POST' action='/delete_post' style='display:inline;float:right;margin-left:8px;'><input type='hidden' name='id' value='" << post.id << "'><button type='submit' style='background:#e74c3c;color:white;border:none;border-radius:4px;padding:2px 8px;'>Delete</button></form>";
            }
            html << "</div>";
        }
        return crow::response(html.str());
    });

    CROW_ROUTE(app, "/like_post").methods("POST"_method)([](const crow::request& req) {
        std::cerr << "[DEBUG] /like_post POST called\n";
        std::string username = getUsernameFromSession(req);
        if (username.empty()) return crow::response(401, "Not logged in");
        
        auto pos = req.body.find("post_id=");
        if (pos == std::string::npos) return crow::response(400, "Missing post_id");
        std::string postId = req.body.substr(pos + 8);
        
        auto posts = loadPostsFromFile();
        auto it = std::find_if(posts.begin(), posts.end(), [&](const Post& p) { return p.id == postId; });
        
        if (it != posts.end()) {
            it->addLike(username);
            savePostsToFile(posts);
            
            json response;
            response["liked"] = true;
            response["like_count"] = it->getLikeCount();
            return crow::response(response.dump());
        }
        
        return crow::response(404, "Post not found");
    });

    CROW_ROUTE(app, "/unlike_post").methods("POST"_method)([](const crow::request& req) {
        std::cerr << "[DEBUG] /unlike_post POST called\n";
        std::string username = getUsernameFromSession(req);
        if (username.empty()) return crow::response(401, "Not logged in");
        
        auto pos = req.body.find("post_id=");
        if (pos == std::string::npos) return crow::response(400, "Missing post_id");
        std::string postId = req.body.substr(pos + 8);
        
        auto posts = loadPostsFromFile();
        auto it = std::find_if(posts.begin(), posts.end(), [&](const Post& p) { return p.id == postId; });
        
        if (it != posts.end()) {
            it->removeLike(username);
            savePostsToFile(posts);
            
            json response;
            response["liked"] = false;
            response["like_count"] = it->getLikeCount();
            return crow::response(response.dump());
        }
        
        return crow::response(404, "Post not found");
    });

    CROW_ROUTE(app, "/delete_post").methods("POST"_method)([](const crow::request& req) {
        std::cerr << "[DEBUG] /delete_post POST called\n";
        std::string username = getUsernameFromSession(req);
        if (username.empty()) return crow::response(401, "Not logged in");
        auto body = req.body;
        auto pos = body.find("id=");
        if (pos == std::string::npos) return crow::response(400, "Missing id");
        std::string id = body.substr(pos + 3);
        auto posts = loadPostsFromFile();
        auto it = std::remove_if(posts.begin(), posts.end(), [&](const Post& p) { return p.id == id && p.author == username; });
        posts.erase(it, posts.end());
        savePostsToFile(posts);
        crow::response res(303);
        res.add_header("Location", "/dashboard");
        return res;
    });

    CROW_ROUTE(app, "/dashboard").methods("GET"_method)([](const crow::request& req) {
        std::cerr << "[DEBUG] /dashboard GET called\n";
        std::string username = getUsernameFromSession(req);
        if (username.empty()) {
            crow::response res(303, "<h1>Redirecting...</h1>");
            res.add_header("Location", "/login");
            return res;
        }
        std::string html = loadFile("dashboard.html");
        // Optionally, could inject username here
        return crow::response(html);
    });

    CROW_ROUTE(app, "/friends").methods("GET"_method)([](const crow::request& req) {
        std::cerr << "[DEBUG] /friends GET called\n";
        std::string username = getUsernameFromSession(req);
        if (username.empty()) return crow::response(401, "Not logged in");
        auto users = loadUsersFromFile();
        User* user = findUserByUsername(users, username);
        std::ostringstream html;
        if (user) {
            for (const auto& f : user->getFriends()) {
                html << "<div>" << f << "</div>";
            }
        }
        return crow::response(html.str());
    });
    CROW_ROUTE(app, "/pending_requests").methods("GET"_method)([](const crow::request& req) {
        std::cerr << "[DEBUG] /pending_requests GET called\n";
        std::string username = getUsernameFromSession(req);
        if (username.empty()) return crow::response(401, "Not logged in");
        auto pending = getPendingRequestsForUser(username);
        std::ostringstream html;
        for (const auto& from : pending) {
            html << "<div>" << from << " <button class='accept-request' data-from='" << from << "'>Accept</button> <button class='reject-request' data-from='" << from << "'>Reject</button></div>";
        }
        return crow::response(html.str());
    });
    CROW_ROUTE(app, "/sent_requests").methods("GET"_method)([](const crow::request& req) {
        std::cerr << "[DEBUG] /sent_requests GET called\n";
        std::string username = getUsernameFromSession(req);
        if (username.empty()) return crow::response(401, "Not logged in");
        auto sent = getSentRequestsByUser(username);
        std::ostringstream html;
        for (const auto& to : sent) {
            html << "<div>" << to << " <button class='cancel-request' data-to='" << to << "'>Cancel</button></div>";
        }
        return crow::response(html.str());
    });
    CROW_ROUTE(app, "/friend_suggestions").methods("GET"_method)([](const crow::request& req) {
        std::cerr << "[DEBUG] /friend_suggestions GET called\n";
        try {
            std::string username = getUsernameFromSession(req);
            if (username.empty()) return crow::response(401, "Not logged in");
            auto users = loadUsersFromFile();
            User* user = findUserByUsername(users, username);
            std::ostringstream html;
            if (user) {
                auto suggestions = suggestFriends(*user, users);
                for (const auto& s : suggestions) {
                    html << "<div class='suggestion-item'>";
                    html << "<span class='suggestion-username'>" << s << "</span>";
                    html << "<button class='send-request-btn' data-username='" << s << "'>";
                    html << "<i class='fa-solid fa-user-plus'></i> Send Request";
                    html << "</button>";
                    html << "</div>";
                }
            }
            return crow::response(html.str());
        } catch (const std::exception& ex) {
            std::cerr << "[EXCEPTION] in /friend_suggestions: " << ex.what() << std::endl;
            return crow::response(500, ex.what());
        } catch (...) {
            std::cerr << "[EXCEPTION] in /friend_suggestions: unknown error" << std::endl;
            return crow::response(500, "Unknown error");
        }
    });
    CROW_ROUTE(app, "/send_friend_request").methods("POST"_method)([](const crow::request& req) {
        std::cerr << "[DEBUG] /send_friend_request POST called\n";
        std::string username = getUsernameFromSession(req);
        std::cerr << "[DEBUG] /send_friend_request called by: " << username << "\n";
        if (username.empty()) {
            std::cerr << "[DEBUG] Not logged in\n";
            return crow::response(401, "Not logged in");
        }
        auto pos = req.body.find("to=");
        if (pos == std::string::npos) {
            std::cerr << "[DEBUG] Missing 'to' in body: " << req.body << "\n";
            return crow::response(400, "Missing 'to'");
        }
        std::string to = req.body.substr(pos + 3);
        // Trim whitespace
        to.erase(0, to.find_first_not_of(" \t\n\r"));
        to.erase(to.find_last_not_of(" \t\n\r") + 1);
        std::cerr << "[DEBUG] Sending friend request to: '" << to << "'\n";
        sendFriendRequest(username, to);
        return crow::response(200);
    });
    CROW_ROUTE(app, "/accept_friend_request").methods("POST"_method)([](const crow::request& req) {
        std::cerr << "[DEBUG] /accept_friend_request POST called\n";
        std::string username = getUsernameFromSession(req);
        if (username.empty()) return crow::response(401, "Not logged in");
        auto pos = req.body.find("from=");
        if (pos == std::string::npos) return crow::response(400, "Missing 'from'");
        std::string from = req.body.substr(pos + 5);
        acceptFriendRequest(from, username);
        return crow::response(200);
    });
    CROW_ROUTE(app, "/reject_friend_request").methods("POST"_method)([](const crow::request& req) {
        std::cerr << "[DEBUG] /reject_friend_request POST called\n";
        std::string username = getUsernameFromSession(req);
        if (username.empty()) return crow::response(401, "Not logged in");
        auto pos = req.body.find("from=");
        if (pos == std::string::npos) return crow::response(400, "Missing 'from'");
        std::string from = req.body.substr(pos + 5);
        rejectFriendRequest(from, username);
        return crow::response(200);
    });
    CROW_ROUTE(app, "/cancel_friend_request").methods("POST"_method)([](const crow::request& req) {
        std::cerr << "[DEBUG] /cancel_friend_request POST called\n";
        std::string username = getUsernameFromSession(req);
        if (username.empty()) return crow::response(401, "Not logged in");
        auto pos = req.body.find("to=");
        if (pos == std::string::npos) return crow::response(400, "Missing 'to'");
        std::string to = req.body.substr(pos + 3);
        cancelFriendRequest(username, to);
        return crow::response(200);
    });

    // New endpoints for mutual friends functionality
    CROW_ROUTE(app, "/mutual_friends/<string>").methods("GET"_method)([](const crow::request& req, std::string otherUser) {
        std::cerr << "[DEBUG] /mutual_friends GET called for user: " << otherUser << std::endl;
        std::string username = getUsernameFromSession(req);
        if (username.empty()) return crow::response(401, "Not logged in");
        
        auto users = loadUsersFromFile();
        User* currentUser = findUserByUsername(users, username);
        User* targetUser = findUserByUsername(users, otherUser);
        
        if (!currentUser || !targetUser) {
            return crow::response(404, "User not found");
        }
        
        auto mutualFriends = getMutualFriendsEfficient(*currentUser, *targetUser);
        
        json response;
        response["mutual_friends"] = mutualFriends;
        response["count"] = mutualFriends.size();
        response["user1"] = username;
        response["user2"] = otherUser;
        
        return crow::response(response.dump());
    });

    CROW_ROUTE(app, "/friend_suggestions_enhanced").methods("GET"_method)([](const crow::request& req) {
        std::cerr << "[DEBUG] /friend_suggestions_enhanced GET called\n";
        std::string username = getUsernameFromSession(req);
        if (username.empty()) return crow::response(401, "Not logged in");
        
        auto users = loadUsersFromFile();
        User* currentUser = findUserByUsername(users, username);
        
        if (!currentUser) {
            return crow::response(404, "User not found");
        }
        
        auto suggestions = getFriendSuggestionsWithScores(*currentUser, users);
        
        json response;
        json suggestionsArray = json::array();
        for (const auto& suggestion : suggestions) {
            json suggestionObj;
            suggestionObj["username"] = suggestion.first;
            suggestionObj["score"] = suggestion.second;
            suggestionsArray.push_back(suggestionObj);
        }
        response["suggestions"] = suggestionsArray;
        
        return crow::response(response.dump());
    });

    CROW_ROUTE(app, "/second_degree_connections").methods("GET"_method)([](const crow::request& req) {
        std::cerr << "[DEBUG] /second_degree_connections GET called\n";
        std::string username = getUsernameFromSession(req);
        if (username.empty()) return crow::response(401, "Not logged in");
        
        auto users = loadUsersFromFile();
        User* currentUser = findUserByUsername(users, username);
        
        if (!currentUser) {
            return crow::response(404, "User not found");
        }
        
        auto secondDegree = getSecondDegreeConnections(*currentUser, users);
        
        json response;
        response["second_degree_connections"] = secondDegree;
        response["count"] = secondDegree.size();
        
        return crow::response(response.dump());
    });

    CROW_ROUTE(app, "/friends_analysis").methods("GET"_method)([](const crow::request& req) {
        std::cerr << "[DEBUG] /friends_analysis GET called\n";
        std::string username = getUsernameFromSession(req);
        if (username.empty()) return crow::response(401, "Not logged in");
        
        auto users = loadUsersFromFile();
        User* currentUser = findUserByUsername(users, username);
        
        if (!currentUser) {
            return crow::response(404, "User not found");
        }
        
        auto friends = currentUser->getFriends();
        auto secondDegree = getSecondDegreeConnections(*currentUser, users);
        auto suggestions = getFriendSuggestionsWithScores(*currentUser, users);
        
        json response;
        response["total_friends"] = friends.size();
        response["second_degree_connections"] = secondDegree.size();
        response["friend_suggestions"] = suggestions.size();
        
        // Get top 5 suggestions with scores
        json topSuggestions = json::array();
        for (int i = 0; i < std::min(5, static_cast<int>(suggestions.size())); ++i) {
            json suggestion;
            suggestion["username"] = suggestions[i].first;
            suggestion["score"] = suggestions[i].second;
            topSuggestions.push_back(suggestion);
        }
        response["top_suggestions"] = topSuggestions;
        
        return crow::response(response.dump());
    });

    CROW_ROUTE(app, "/logout").methods("GET"_method)([](const crow::request& req) {
        std::cerr << "[DEBUG] /logout GET called\n";
        auto cookie = req.get_header_value("Cookie");
        auto pos = cookie.find("session=");
        if (pos != std::string::npos) {
            auto end = cookie.find(';', pos);
            std::string token = cookie.substr(pos + 8, end == std::string::npos ? std::string::npos : end - (pos + 8));
            sessionMap.erase(token);
        }
        
        crow::response res(303);
        res.add_header("Set-Cookie", "session=; Path=/; HttpOnly; Max-Age=0");
        res.add_header("Location", "/login");
        return res;
    });

    CROW_ROUTE(app, "/current_user").methods("GET"_method)([](const crow::request& req) {
        std::cerr << "[DEBUG] /current_user GET called\n";
        std::string username = getUsernameFromSession(req);
        if (username.empty()) return crow::response(401, "Not logged in");
        
        json response;
        response["username"] = username;
        return crow::response(response.dump());
    });

    app.bindaddr("0.0.0.0").port(18080).multithreaded().run();
}
