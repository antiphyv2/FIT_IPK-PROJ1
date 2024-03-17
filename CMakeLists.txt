cmake_minimum_required(VERSION 3.10)

# Set project name and version
project(MyProject VERSION 1.0)


# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the executable target
add_executable(ipk24chat-client proj1.cpp proj1.hpp socket.cpp socket.hpp messages.cpp messages.hpp)
