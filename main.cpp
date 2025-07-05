#include "crow_all.h"
#include <fstream>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>
#include <unordered_map>
#include <filesystem>
#include <algorithm>

// Util: Load a static HTML file
std::string loadFile(const std::string& relativePath) {
    std::filesystem::path fullPath = std::filesystem::current_path() / relativePath;
    std::cerr << "📄 Trying to load: " << fullPath << std::endl;

    std::ifstream in(fullPath);
    if (!in.is_open()) {
        std::cerr << "❌ Failed to open file.\n";
        return "<h1>File Not Found</h1>";
    }

    std::ostringstream contents;
    contents << in.rdbuf();
    return contents.str();
}

// Util: Save a user to file
void saveUserToFile(const std::string& username, const std::string& hashedPassword) {
    std::ofstream out("users.txt", std::ios::app);
    if (!out.is_open()) {
        std::cerr << "❌ Could not open users.txt for writing\n";
        return;
    }

    std::cerr << "📥 Writing to users.txt: " << username << "|" << hashedPassword << "\n";
    out << username << "|" << hashedPassword << "\n";
}

// Util: Load all users
std::unordered_map<std::string, std::string> loadUsersFromFile() {
    std::unordered_map<std::string, std::string> loadedUsers;
    std::ifstream in("users.txt");
    std::string line;

    while (std::getline(in, line)) {
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end()); // strip \r
        auto sep = line.find('|');
        if (sep != std::string::npos) {
            std::string user = line.substr(0, sep);
            std::string hash = line.substr(sep + 1);
            loadedUsers[user] = hash;
        }
}


    return loadedUsers;
}

// Util: SHA-256 password hashing
std::string hashPassword(const std::string& password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password.c_str(), password.size(), hash);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];

    return ss.str();
}

// Util: Parse x-www-form-urlencoded body manually
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

// ==================== MAIN ====================

int main() {
    crow::SimpleApp app;

    std::cerr << "📂 Current dir: " << std::filesystem::current_path() << "\n";

    // Serve signup page
    CROW_ROUTE(app, "/signup").methods("GET"_method)([] {
        return crow::response(loadFile("static/signup.html"));
    });

    // Serve login page
    CROW_ROUTE(app, "/login").methods("GET"_method)([] {
        return crow::response(loadFile("static/login.html"));
    });

    // Handle signup
    CROW_ROUTE(app, "/signup").methods("POST"_method)([](const crow::request& req) {
        std::cerr << "🚀 /signup POST called\n";
        std::cerr << "📦 Body: " << req.body << "\n";

        auto [username, password] = parseForm(req.body);

        if (username.empty() || password.empty()) {
            std::cerr << "❌ Missing credentials\n";
            return crow::response(400, "Missing credentials");
        }

        std::cerr << "🔐 Signup username: '" << username << "'\n";
        std::cerr << "🔐 Signup password (raw): '" << password << "'\n";

        auto users = loadUsersFromFile();
        if (users.find(username) != users.end()) {
            std::cerr << "❌ Username exists\n";
            return crow::response(400, "Username already exists");
        }

        std::string hashed = hashPassword(password);
        std::cerr << "🔐 Hashed password: " << hashed << "\n";

        saveUserToFile(username, hashed);
        std::cerr << "✅ Signup successful: " << username << "\n";

        return crow::response(200, "✅ Signup successful! Go to <a href=\"/login\">Login</a>");
    });

    // Handle login
    CROW_ROUTE(app, "/login").methods("POST"_method)([](const crow::request& req) {
        std::cerr << "🚀 /login POST called\n";
        std::cerr << "📦 Body: " << req.body << "\n";

        auto [username, password] = parseForm(req.body);

        if (username.empty() || password.empty()) {
            std::cerr << "❌ Missing credentials\n";
            return crow::response(400, "Missing credentials");
        }

        auto users = loadUsersFromFile();
        auto it = users.find(username);

        if (it == users.end() || it->second != hashPassword(password)) {
            std::cerr << "❌ Invalid login\n";
            return crow::response(401, "❌ Invalid credentials");
        }

        std::cerr << "✅ Login: " << username << "\n";
        return crow::response(200, "✅ Login successful!");
    });

    app.port(18080).multithreaded().run();
}
