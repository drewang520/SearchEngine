cmake_minimum_required(VERSION 3.22)
project(search_engine LANGUAGES CXX)

# <----------- set C++ standard ------------>
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

# file(GLOB_RECURSE SRC_FILES
#     ${PROJECT_SOURCE_DIR}/src/*.cc
#     ${PROJECT_SOURCE_DIR}/src/*.cpp
# )
add_executable(client
        src/Configuration.cc
        src/ProtocolParser.cc
        src/clientTest.cc)

add_executable(Config_test
        src/Configuration.cc
        src/test_config.cc)

add_executable(dicProducer 
        src/DicProducer.cc
        src/testDicProducer.cc
        src/CppJiebaSplit.cc
        src/Configuration.cc
    )

add_executable(pageLibPreprocessor_test
        src/Configuration.cc
        src/tinyxml2.cpp
        src/CppJiebaSplit.cc
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
        src/CppJiebaSplit.cc
        src/WebPage.cc
        src/tinyxml2.cpp
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

add_executable(RSserver
        src/Configuration.cc
        src/MyRedis.cc
        src/tinyxml2.cpp
        src/KeyRecommander.cc
        src/Dictionary.cc
        src/WebPageQuery.cc
        src/WebPage.cc
        src/CppJiebaSplit.cc
        src/ProtocolParser.cc
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
        src/Reactor/TestRSserver.cc
        src/Reactor/Thread.cc
        src/Reactor/ThreadPool.cc
        src/Reactor/Acceptor.cc
        src/Reactor/Condition.cc
    )

# <------------ add hiredis dependency --------------->
find_path(HIREDIS_HEADER hiredis)
target_include_directories(RSserver PUBLIC ${HIREDIS_HEADER})

find_library(HIREDIS_LIB hiredis)
target_link_libraries(RSserver ${HIREDIS_LIB})

# <------------ add redis-plus-plus dependency -------------->
# NOTE: this should be *sw* NOT *redis++*
find_path(REDIS_PLUS_PLUS_HEADER sw)
target_include_directories(RSserver PUBLIC ${REDIS_PLUS_PLUS_HEADER})

find_library(REDIS_PLUS_PLUS_LIB redis++)
target_link_libraries(RSserver ${REDIS_PLUS_PLUS_LIB})

