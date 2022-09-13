cmake_minimum_required(VERSION 3.23)
project(homework8_1)

set(CMAKE_CXX_STANDARD 17)

add_executable(homework8_1 main.cpp torrent_downloader.cpp)
target_link_libraries(task1 PRIVATE torrent-rasterbar)
