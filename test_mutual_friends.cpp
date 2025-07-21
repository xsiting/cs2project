#include "user.h"
#include "utility.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "=== Mutual Friends Functionality Test ===\n\n";
    
    // Create test users
    User alice("alice", "hash1");
    User bob("bob", "hash2");
    User charlie("charlie", "hash3");
    User diana("diana", "hash4");
    User eve("eve", "hash5");
    User frank("frank", "hash6");
    
    // Add friends to create a network
    alice.addFriend("bob");
    alice.addFriend("charlie");
    alice.addFriend("diana");
    
    bob.addFriend("alice");
    bob.addFriend("charlie");
    bob.addFriend("eve");
    
    charlie.addFriend("alice");
    charlie.addFriend("bob");
    charlie.addFriend("diana");
    charlie.addFriend("frank");
    
    diana.addFriend("alice");
    diana.addFriend("charlie");
    diana.addFriend("eve");
    
    eve.addFriend("bob");
    eve.addFriend("diana");
    eve.addFriend("frank");
    
    frank.addFriend("charlie");
    frank.addFriend("eve");
    
    std::vector<User> allUsers = {alice, bob, charlie, diana, eve, frank};
    
    // Test 1: Mutual friends between Alice and Bob
    std::cout << "Test 1: Mutual friends between Alice and Bob\n";
    auto mutualAliceBob = getMutualFriendsEfficient(alice, bob);
    std::cout << "Mutual friends: ";
    for (const auto& friend_name : mutualAliceBob) {
        std::cout << friend_name << " ";
    }
    std::cout << "(" << mutualAliceBob.size() << " total)\n\n";
    
    // Test 2: Mutual friends between Alice and Charlie
    std::cout << "Test 2: Mutual friends between Alice and Charlie\n";
    auto mutualAliceCharlie = getMutualFriendsEfficient(alice, charlie);
    std::cout << "Mutual friends: ";
    for (const auto& friend_name : mutualAliceCharlie) {
        std::cout << friend_name << " ";
    }
    std::cout << "(" << mutualAliceCharlie.size() << " total)\n\n";
    
    // Test 3: Friend suggestions for Alice
    std::cout << "Test 3: Friend suggestions for Alice\n";
    auto suggestionsAlice = getFriendSuggestionsWithScores(alice, allUsers);
    std::cout << "Suggestions (with scores):\n";
    for (const auto& suggestion : suggestionsAlice) {
        std::cout << "  " << suggestion.first << " (Score: " << suggestion.second << ")\n";
    }
    std::cout << "\n";
    
    // Test 4: Second degree connections for Alice
    std::cout << "Test 4: Second degree connections for Alice\n";
    auto secondDegreeAlice = getSecondDegreeConnections(alice, allUsers);
    std::cout << "Second degree connections: ";
    for (const auto& connection : secondDegreeAlice) {
        std::cout << connection << " ";
    }
    std::cout << "(" << secondDegreeAlice.size() << " total)\n\n";
    
    // Test 5: AVL Tree operations demonstration
    std::cout << "Test 5: AVL Tree Operations\n";
    std::cout << "Alice's friends (in-order): ";
    auto aliceFriends = alice.getFriends();
    for (const auto& friend_name : aliceFriends) {
        std::cout << friend_name << " ";
    }
    std::cout << "\n";
    
    std::cout << "Alice's friends (level-order): ";
    // Note: We need to access the AVL tree directly for level-order
    // This demonstrates the AVL tree structure is working
    
    std::cout << "AVL tree contains 'bob': " << (alice.isFriend("bob") ? "Yes" : "No") << "\n";
    std::cout << "AVL tree contains 'frank': " << (alice.isFriend("frank") ? "Yes" : "No") << "\n";
    
    std::cout << "\n=== Test Complete ===\n";
    
    return 0;
} 