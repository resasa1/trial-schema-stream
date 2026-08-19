cmake_minimum_required(VERSION 3.10)
project(StreamEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Cari Boost
find_package(Boost REQUIRED COMPONENTS system)
if(Boost_FOUND)
    message(STATUS "✅ Boost found: ${Boost_INCLUDE_DIRS}")
endif()

# Jangan pake FetchContent, kita clone manual aja
# Tapi kita inject variabel untuk pake source lokal
set(WEBSOCKETPP_DIR ${CMAKE_SOURCE_DIR}/websocketpp)

if(NOT EXISTS ${WEBSOCKETPP_DIR})
    message(FATAL_ERROR "websocketpp not found! Please run: git clone https://github.com/kenmcmil/websocketpp.git")
endif()

add_executable(stream_engine main.cpp)
target_include_directories(stream_engine PRIVATE ${WEBSOCKETPP_DIR} ${Boost_INCLUDE_DIRS})
target_link_libraries(stream_engine ${Boost_LIBRARIES} pthread)
