cmake_minimum_required(VERSION 3.20)
project(elasticsearch_manager)

set(CMAKE_CXX_STANDARD 20)

add_executable(elasticsearch_manager main.cpp include/rest.h include/url_parser/url_parser.cpp include/url_parser/url_parser.h include/useful_funcs/hashing.cpp include/useful_funcs/hashing.h include/useful_funcs/find.cpp include/useful_funcs/find.h include/custom.cpp include/custom.h)

add_subdirectory(include/cpp-httplib)
target_link_libraries(elasticsearch_manager httplib)

add_subdirectory(include/fmt)
target_link_libraries(elasticsearch_manager fmt)

add_subdirectory(include/json)
target_link_libraries(elasticsearch_manager nlohmann_json::nlohmann_json)