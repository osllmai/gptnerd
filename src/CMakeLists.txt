cmake_minimum_required(VERSION 3.10)

set(CXX_STANDARD 11)
set(CXX_STANDARD_REQUIRED True)

project(NerdGPT)

include_directories(include)
add_executable(${PROJECT_NAME} chat.cpp  ../gptnerd_backend/llmodel_c.h)
target_link_libraries(${PROJECT_NAME} PRIVATE  llmodel llama)


