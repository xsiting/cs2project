#include "user.h"

// Default ctor
User::User() : username(""), passwordHash("") {}

// Param ctor
User::User(string u, string p) : username(u), passwordHash(p) {}

User::~User() {}

// Must match header: const qualifier!
string User::getUsername() const {
    return username;
}
string User::getPasswordHash() const {
    return passwordHash;
}

// Add missing comma!
json User::to_json() const {
    return json{
        {"username", username},
        {"passwordHash", passwordHash}
    };
}

// Proper member definition (no 'static' here)
User User::from_json(const json& j) {
    return User(
        j.at("username").get<string>(),
        j.at("passwordHash").get<string>()
    );
}