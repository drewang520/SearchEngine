cmake_minimum_required(VERSION 3.10)
project(search_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

# file(GLOB_RECURSE SRC_FILES
#     ${PROJECT_SOURCE_DIR}/src/*.cc
#     ${PROJECT_SOURCE_DIR}/src/*.cpp
# )

add_executable(dicProducer 
        src/DicProducer.cc
        src/testDicProducer.cc)

add_executable(dictionary
        src/Dictionary.cc
        src/testDictionary.cc)

add_executable(reactor 
        src/Acceptor.cc
        src/Condition.cc
        src/EventLoop.cc
        src/InetAddress.cc
        src/MutexLock.cc
        src/Socket.cc
        src/SocketIO.cc
        src/TaskQueue.cc
        src/TcpConnection.cc
        src/TcpServer.cc
        src/TestReactorv5.cc
        src/Thread.cc
        src/ThreadPool.cc)

