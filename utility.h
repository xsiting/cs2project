#pragma once
#include <fstream>
#include <vector>
#include <iostream>
#include "user.h"
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

vector<User> loadUsersFromFile(const string& filename = "../../users.json");

void saveUsersToFile(const vector<User>& users, const string& filename = "../../users.json");

User* findUserByUsername(vector<User>& users, const string& username);