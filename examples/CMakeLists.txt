project(sim8051)

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SIM8051_SOURCES
	sim8051.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSDL_MAIN_HANDLED")

add_executable(sim8051 ${SIM8051_SOURCES})
include_directories(sim8051 ${BEE8051_INCLUDE_DIR})
target_link_libraries(sim8051 libbee8051)

find_package(SDL2 REQUIRED)

if (TARGET SDL2::SDL2)
    target_link_libraries(sim8051 SDL2::SDL2)
else()
    target_link_libraries(sim8051 ${SDL2_LIBRARIES})
endif()