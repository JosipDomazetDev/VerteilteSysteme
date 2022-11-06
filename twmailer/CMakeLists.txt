cmake_minimum_required(VERSION 3.23)
project(twmailer)

set(CMAKE_CXX_STANDARD 20)

add_executable(twmailer server/server-runner.cpp server/server.cpp  client/client-runner.cpp client/client.cpp)
# add_executable(twmailer server/server-runner.cpp server/server.cpp )
