cmake_minimum_required(VERSION 3.15)
project(wms-userver CXX)
find_package(userver COMPONENTS core postgresql REQUIRED)
include_directories(src)
add_executable(${PROJECT_NAME} src/main.cpp
    src/handlers/arrivals/get/arrivals.cpp
    src/handlers/arrivals/post/arrivals.cpp
    src/handlers/auth/post/auth.cpp
    src/handlers/inventory/get/inventory.cpp
    src/handlers/inventory/post/inventory.cpp
    src/handlers/products/delete/products.cpp
    src/handlers/products/get/products.cpp
    src/handlers/products/patch/products.cpp
    src/handlers/products/post/products.cpp
    src/handlers/users/get/users.cpp
)
target_link_libraries(${PROJECT_NAME} userver::core userver::postgresql)
