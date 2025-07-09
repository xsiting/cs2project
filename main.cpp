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



std::string loadFile(const std::string& relativePath) {
    std::filesystem::path fullPath = std::filesystem::current_path() / relativePath;
    std::cerr << "Trying to load: " << fullPath << std::endl;

    std::ifstream in(fullPath);
    if (!in.is_open()) {
        std::cerr << "Failed to open file.\n";
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


int main() {
    crow::SimpleApp app;

    std::cerr << "Current dir: " << std::filesystem::current_path() << "\n";

    
    CROW_ROUTE(app, "/signup").methods("GET"_method)([] {
        return crow::response(loadFile("../../static/signup.html"));
    });

    
    CROW_ROUTE(app, "/login").methods("GET"_method)([] {
        return crow::response(loadFile("../../static/login.html"));
    });

    
    CROW_ROUTE(app, "/signup").methods("POST"_method)([](const crow::request& req) {
        std::cerr << "/signup POST called\n";
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
        std::cerr << "/login POST called\n";
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

    CROW_ROUTE(app, "/dashboard").methods("GET"_method)([] {
    return crow::response(loadFile("../../static/dashboard.html"));
});



    app.bindaddr("0.0.0.0").port(18080) .multithreaded().run();
}
