#include "utility.h"

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