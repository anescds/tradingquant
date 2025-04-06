cmake_minimum_required(VERSION 3.10)
project(PortfolioOptimizer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the executable
add_executable(portfolio_optimizer 
    portfolio_optimizer.cpp
    prism_client.h
)

# Link against Windows libraries
target_link_libraries(portfolio_optimizer PRIVATE wininet) 