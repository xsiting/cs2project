#pragma once
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <memory>
#include <queue>

using namespace std;
using json = nlohmann::json;

class AVLTreeNode {
public:
    std::string username;
    std::shared_ptr<AVLTreeNode> left, right;
    int height;
    AVLTreeNode(const std::string& uname) : username(uname), left(nullptr), right(nullptr), height(1) {}
};

class AVLTree {
private:
    std::shared_ptr<AVLTreeNode> root;
    std::shared_ptr<AVLTreeNode> insert(std::shared_ptr<AVLTreeNode> node, const std::string& username);
    std::shared_ptr<AVLTreeNode> remove(std::shared_ptr<AVLTreeNode> node, const std::string& username);
    bool contains(std::shared_ptr<AVLTreeNode> node, const std::string& username) const;
    void inOrder(std::shared_ptr<AVLTreeNode> node, std::vector<std::string>& result) const;
    void levelOrder(std::shared_ptr<AVLTreeNode> node, std::vector<std::string>& result) const;
    int getHeight(std::shared_ptr<AVLTreeNode> node) const;
    int getBalance(std::shared_ptr<AVLTreeNode> node) const;
    std::shared_ptr<AVLTreeNode> rotateRight(std::shared_ptr<AVLTreeNode> y);
    std::shared_ptr<AVLTreeNode> rotateLeft(std::shared_ptr<AVLTreeNode> x);
public:
    AVLTree();
    void insert(const std::string& username);
    void remove(const std::string& username);
    bool contains(const std::string& username) const;
    std::vector<std::string> inOrder() const;
    std::vector<std::string> levelOrder() const;
    int size() const;
    bool isEmpty() const;
};

class User
{
private:
    string username;
    string passwordHash;
    AVLTree friends;

public:
    User();
    User(string, string);
    virtual ~User();

    string getUsername() const;
    string getPasswordHash() const;
    std::vector<std::string> getFriends() const;
    void addFriend(const std::string& friendUsername);
    void removeFriend(const std::string& friendUsername);
    bool isFriend(const std::string& friendUsername) const;
    json to_json() const;
    static User from_json(const json&);

    // Add operator== for comparison with string
    bool operator==(const std::string& other) const {
        return username == other;
    }
    // Add operator== for comparison with User
    bool operator==(const User& other) const {
        return username == other.username;
    }
};