#pragma once
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>


using namespace std;
using json = nlohmann::json;

class User
{
private:
    string username;
    string passwordHash;

public:
    User();
    User(string, string);
    virtual ~User();

    string getUsername() const;
    string getPasswordHash() const;
    json to_json() const;
    static User from_json(const json&);
};