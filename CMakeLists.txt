cmake_minimum_required(VERSION 3.10)
project(crow_login_system)

set(CMAKE_CXX_STANDARD 17)

# Add OpenSSL headers and libs manually
include_directories(${CMAKE_SOURCE_DIR}/openssl/include)
include_directories(${CMAKE_SOURCE_DIR}/asio/include)

# Add nlohmann_json header path (header-only)
include_directories(${CMAKE_SOURCE_DIR}/third_party)

link_directories(${CMAKE_SOURCE_DIR}/openssl/lib)

# Create executable
add_executable(crow_login_system main.cpp user.cpp utility.cpp post.cpp)

# Link OpenSSL manually
target_link_libraries(crow_login_system libssl libcrypto)