# Mutual Friends Functionality with AVL Trees

This document describes the implementation of mutual friends functionality for the friendship management website using AVL trees for efficient operations.

## Features Implemented

### 1. Mutual Friends Computation
- **Function**: `getMutualFriendsEfficient(const User& a, const User& b)`
- **Algorithm**: Uses AVL tree search operations for O(log n) friend lookups
- **API Endpoint**: `GET /mutual_friends/<username>`
- **Returns**: JSON with mutual friends list, count, and user information

### 2. Friend Suggestions
- **Function**: `getFriendSuggestionsWithScores(const User& user, const std::vector<User>& allUsers)`
- **Algorithm**: 
  - Calculates mutual friend count using AVL tree operations
  - Considers second-degree connections (friends of friends)
  - Scores users based on mutual friends + second-degree bonus
- **API Endpoint**: `GET /friend_suggestions_enhanced`
- **Returns**: JSON with suggestions ranked by score

### 3. Second-Degree Connections
- **Function**: `getSecondDegreeConnections(const User& user, const std::vector<User>& allUsers)`
- **Algorithm**: Finds friends of friends using AVL tree traversal
- **API Endpoint**: `GET /second_degree_connections`
- **Returns**: JSON with second-degree connections list

### 4. Friends Network Analysis
- **Function**: `getFriendSuggestionsWithScores()` + additional analysis
- **API Endpoint**: `GET /friends_analysis`
- **Returns**: JSON with network statistics and top suggestions

## AVL Tree Implementation

### Enhanced AVL Tree Class
```cpp
class AVLTree {
    // Existing methods: insert, remove, contains, inOrder
    // New methods:
    std::vector<std::string> levelOrder() const;  // Level-order traversal
    int size() const;                             // Get tree size
    bool isEmpty() const;                         // Check if empty
};
```

### AVL Tree Operations Used
1. **In-Order Traversal**: Used for getting sorted friend lists
2. **Level-Order Traversal**: Used for breadth-first friend exploration
3. **Search Operations**: O(log n) friend existence checks
4. **Size Operations**: Efficient friend count retrieval

## API Endpoints

### 1. Mutual Friends Calculator
```
GET /mutual_friends/<username>
```
**Response:**
```json
{
  "mutual_friends": ["user1", "user2", "user3"],
  "count": 3,
  "user1": "current_user",
  "user2": "target_user"
}
```

### 2. Enhanced Friend Suggestions
```
GET /friend_suggestions_enhanced
```
**Response:**
```json
{
  "suggestions": [
    {"username": "user1", "score": 5},
    {"username": "user2", "score": 3},
    {"username": "user3", "score": 2}
  ]
}
```

### 3. Second Degree Connections
```
GET /second_degree_connections
```
**Response:**
```json
{
  "second_degree_connections": ["user1", "user2", "user3"],
  "count": 3
}
```

### 4. Friends Analysis
```
GET /friends_analysis
```
**Response:**
```json
{
  "total_friends": 10,
  "second_degree_connections": 15,
  "friend_suggestions": 8,
  "top_suggestions": [
    {"username": "user1", "score": 5},
    {"username": "user2", "score": 4}
  ]
}
```

## Frontend Integration

### Dashboard Features
1. **Mutual Friends Calculator**: Input field to calculate mutual friends with any user
2. **Friends Analysis**: Button to analyze network statistics
3. **Second Degree Connections**: Button to view friends of friends
4. **Enhanced Suggestions**: Improved friend suggestions with scores

### UI Components
- Sidebar sections for each feature
- Real-time results display
- Error handling and user feedback
- Responsive design with glassmorphism styling

## Algorithm Complexity

### Time Complexity
- **Mutual Friends**: O(n log m) where n = friends of user A, m = friends of user B
- **Friend Suggestions**: O(n² log m) where n = total users, m = average friends per user
- **Second Degree**: O(n log m) where n = friends, m = average friends per friend
- **AVL Tree Operations**: O(log n) for search, insert, delete

### Space Complexity
- **AVL Tree**: O(n) where n = number of friends
- **Mutual Friends**: O(min(n,m)) where n,m = friend counts
- **Suggestions**: O(n) where n = total users

## Testing

### Test File: `test_mutual_friends.cpp`
Compile and run to test the functionality:
```bash
g++ -o test_mutual_friends test_mutual_friends.cpp user.cpp utility.cpp -std=c++17
./test_mutual_friends
```

### Test Scenarios
1. Mutual friends between two users
2. Friend suggestions with scoring
3. Second-degree connections
4. AVL tree operations verification

## Usage Examples

### 1. Calculate Mutual Friends
```javascript
// Frontend JavaScript
fetch('/mutual_friends/alice')
  .then(res => res.json())
  .then(data => {
    console.log('Mutual friends:', data.mutual_friends);
    console.log('Count:', data.count);
  });
```

### 2. Get Friend Suggestions
```javascript
fetch('/friend_suggestions_enhanced')
  .then(res => res.json())
  .then(data => {
    data.suggestions.forEach(suggestion => {
      console.log(`${suggestion.username}: ${suggestion.score}`);
    });
  });
```

### 3. Analyze Network
```javascript
fetch('/friends_analysis')
  .then(res => res.json())
  .then(data => {
    console.log('Total friends:', data.total_friends);
    console.log('Second degree:', data.second_degree_connections);
  });
```

## Benefits of AVL Tree Implementation

1. **Efficient Search**: O(log n) friend existence checks
2. **Balanced Structure**: Automatic balancing for optimal performance
3. **Sorted Operations**: In-order traversal provides sorted friend lists
4. **Memory Efficient**: No duplicate storage of friend relationships
5. **Scalable**: Performance remains consistent as friend count grows

## Future Enhancements

1. **Caching**: Implement caching for frequently accessed mutual friend calculations
2. **Graph Algorithms**: Add more sophisticated graph traversal algorithms
3. **Recommendation Engine**: Implement machine learning-based friend suggestions
4. **Real-time Updates**: WebSocket integration for live friend activity
5. **Privacy Controls**: Granular privacy settings for mutual friend visibility 