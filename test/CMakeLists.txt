cmake_minimum_required(VERSION 3.30)
project(real_human_bean_test VERSION 0.0.1)

find_package(glm CONFIG REQUIRED)

add_executable(real_human_bean_test main.cpp
        ../lib/engine.cpp
        ../lib/quad.cpp
        ../lib/quad.hpp
        ../lib/mouse.hpp
        ../lib/event.hpp
        ../lib/config.hpp
)

target_link_libraries(real_human_bean_test PRIVATE glm::glm)