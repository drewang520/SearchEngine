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

add_executable(Config_test
        src/Configuration.cc
        src/test_config.cc)

add_executable(dicProducer 
        src/DicProducer.cc
        src/testDicProducer.cc
        src/Configuration.cc
    )

add_executable(pageLibPreprocessor_test
        src/Configuration.cc
        src/tinyxml2.cpp
        src/Optimize_pageLibPreprocessor.cc
        src/pageLibPreprocessor_test.cc
    )

add_executable(PageProducer_test
        src/Configuration.cc
        src/PageProducer.cc
        src/test_pageProducer.cc
        src/tinyxml2.cpp
    )

add_executable(WebPageSearch
        src/Configuration.cc
        src/WebPageQuery.cc
        src/test_WebPageSearch.cc
    )

add_executable(KeyCommander_test
        src/Configuration.cc
        src/Dictionary.cc
        src/KeyRecommander.cc
        src/test_KeyRecommander.cc
    )

add_executable(dictionary
        src/Dictionary.cc
        src/testDictionary.cc
        src/Configuration.cc
    )

add_executable(reactor
        src/Reactor/Acceptor.cc
        src/Reactor/Condition.cc
        src/Reactor/EventLoop.cc
        src/Reactor/InetAddress.cc
        src/Reactor/MutexLock.cc
        src/Reactor/Socket.cc
        src/Reactor/SocketIO.cc
        src/Reactor/TaskQueue.cc
        src/Reactor/TcpConnection.cc
        src/Reactor/TcpServer.cc
        src/Reactor/TestReactorv5.cc
        src/Reactor/Thread.cc
        src/Reactor/ThreadPool.cc
        src/Reactor/Acceptor.cc
        src/Reactor/Condition.cc
    )
