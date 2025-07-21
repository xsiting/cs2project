#pragma once
#include <string>
#include <vector>

class UserSearch {
private:
    struct Node {
        std::string username;
        Node* left;
        Node* right;
        
        Node(const std::string& name) : username(name), left(nullptr), right(nullptr) {}
    };
    
    Node* root;

    // Helper function to delete the tree
    void deleteTree(Node* node) {
        if (node) {
            deleteTree(node->left);
            deleteTree(node->right);
            delete node;
        }
    }

    // Helper function to insert a username
    void insertHelper(Node*& node, const std::string& username) {
        if (!node) {
            node = new Node(username);
            return;
        }
        if (username < node->username) {
            insertHelper(node->left, username);
        } else if (username > node->username) {
            insertHelper(node->right, username);
        }
    }

    // Helper function to search by prefix
    void searchByPrefixHelper(Node* node, const std::string& prefix, std::vector<std::string>& results) {
        if (!node) return;

        // Check if current node's username starts with prefix
        if (node->username.substr(0, prefix.length()) == prefix) {
            results.push_back(node->username);
        }

        // If prefix is less than current username, search left
        if (prefix <= node->username) {
            searchByPrefixHelper(node->left, prefix, results);
        }
        // If prefix's next possible string is greater than current username, search right
        searchByPrefixHelper(node->right, prefix, results);
    }

public:
    UserSearch() : root(nullptr) {}
    
    ~UserSearch() {
        deleteTree(root);
    }

    // Insert a username into the BST
    void insert(const std::string& username) {
        insertHelper(root, username);
    }

    // Search usernames by prefix
    std::vector<std::string> search(const std::string& prefix) {
        std::vector<std::string> results;
        if (!prefix.empty()) {
            searchByPrefixHelper(root, prefix, results);
        }
        return results;
    }

    // Clear the tree
    void clear() {
        deleteTree(root);
        root = nullptr;
    }
}; 