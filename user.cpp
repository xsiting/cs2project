#include "user.h"
#include <algorithm>

// Default ctor
User::User() : username(""), passwordHash("") {}

// Param ctor
User::User(string u, string p) : username(u), passwordHash(p), friends() {}

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
        {"passwordHash", passwordHash},
        {"friends", getFriends()}
    };
}

// Proper member definition (no 'static' here)
User User::from_json(const json& j) {
    User u(j.at("username").get<string>(), j.at("passwordHash").get<string>());
    if (j.contains("friends")) {
        for (const auto& f : j.at("friends")) {
            u.addFriend(f.get<string>());
        }
    }
    return u;
}

std::vector<std::string> User::getFriends() const {
    return friends.inOrder();
}
void User::addFriend(const std::string& friendUsername) {
    friends.insert(friendUsername);
}
void User::removeFriend(const std::string& friendUsername) {
    friends.remove(friendUsername);
}
bool User::isFriend(const std::string& friendUsername) const {
    return friends.contains(friendUsername);
}

// AVLTree implementation
AVLTree::AVLTree() : root(nullptr) {}

int AVLTree::getHeight(std::shared_ptr<AVLTreeNode> node) const {
    return node ? node->height : 0;
}

int AVLTree::getBalance(std::shared_ptr<AVLTreeNode> node) const {
    return node ? getHeight(node->left) - getHeight(node->right) : 0;
}

std::shared_ptr<AVLTreeNode> AVLTree::rotateRight(std::shared_ptr<AVLTreeNode> y) {
    auto x = y->left;
    auto T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = std::max(getHeight(y->left), getHeight(y->right)) + 1;
    x->height = std::max(getHeight(x->left), getHeight(x->right)) + 1;
    return x;
}

std::shared_ptr<AVLTreeNode> AVLTree::rotateLeft(std::shared_ptr<AVLTreeNode> x) {
    auto y = x->right;
    auto T2 = y->left;
    y->left = x;
    x->right = T2;
    x->height = std::max(getHeight(x->left), getHeight(x->right)) + 1;
    y->height = std::max(getHeight(y->left), getHeight(y->right)) + 1;
    return y;
}

std::shared_ptr<AVLTreeNode> AVLTree::insert(std::shared_ptr<AVLTreeNode> node, const std::string& username) {
    if (!node) return std::make_shared<AVLTreeNode>(username);
    if (username < node->username)
        node->left = insert(node->left, username);
    else if (username > node->username)
        node->right = insert(node->right, username);
    else
        return node;
    node->height = 1 + std::max(getHeight(node->left), getHeight(node->right));
    int balance = getBalance(node);
    if (balance > 1 && username < node->left->username)
        return rotateRight(node);
    if (balance < -1 && username > node->right->username)
        return rotateLeft(node);
    if (balance > 1 && username > node->left->username) {
        node->left = rotateLeft(node->left);
        return rotateRight(node);
    }
    if (balance < -1 && username < node->right->username) {
        node->right = rotateRight(node->right);
        return rotateLeft(node);
    }
    return node;
}

void AVLTree::insert(const std::string& username) {
    root = insert(root, username);
}

std::shared_ptr<AVLTreeNode> AVLTree::remove(std::shared_ptr<AVLTreeNode> node, const std::string& username) {
    if (!node) return node;
    if (username < node->username)
        node->left = remove(node->left, username);
    else if (username > node->username)
        node->right = remove(node->right, username);
    else {
        if (!node->left || !node->right) {
            node = node->left ? node->left : node->right;
        } else {
            auto temp = node->right;
            while (temp->left) temp = temp->left;
            node->username = temp->username;
            node->right = remove(node->right, temp->username);
        }
    }
    if (!node) return node;
    node->height = 1 + std::max(getHeight(node->left), getHeight(node->right));
    int balance = getBalance(node);
    if (balance > 1 && getBalance(node->left) >= 0)
        return rotateRight(node);
    if (balance > 1 && getBalance(node->left) < 0) {
        node->left = rotateLeft(node->left);
        return rotateRight(node);
    }
    if (balance < -1 && getBalance(node->right) <= 0)
        return rotateLeft(node);
    if (balance < -1 && getBalance(node->right) > 0) {
        node->right = rotateRight(node->right);
        return rotateLeft(node);
    }
    return node;
}

void AVLTree::remove(const std::string& username) {
    root = remove(root, username);
}

bool AVLTree::contains(std::shared_ptr<AVLTreeNode> node, const std::string& username) const {
    if (!node) return false;
    if (username == node->username) return true;
    if (username < node->username) return contains(node->left, username);
    return contains(node->right, username);
}

bool AVLTree::contains(const std::string& username) const {
    return contains(root, username);
}

void AVLTree::inOrder(std::shared_ptr<AVLTreeNode> node, std::vector<std::string>& result) const {
    if (!node) return;
    inOrder(node->left, result);
    result.push_back(node->username);
    inOrder(node->right, result);
}

std::vector<std::string> AVLTree::inOrder() const {
    std::vector<std::string> result;
    try {
        inOrder(root, result);
    } catch (...) {
        // Defensive: if anything goes wrong, return empty vector
        result.clear();
    }
    return result;
}