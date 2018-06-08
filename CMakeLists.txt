cmake_minimum_required(VERSION 2.8)
project(ordersvr)
find_package(gflags REQUIRED)
 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -std=c++11 -DDSOLUTION -DMULTIPLE_THREAD -DKEEP_ALIVE_TEST -DDEBUG")
 

 
include_directories(${PROJECT_SOURCE_DIR}/include)
 
# tnode include
 
#include_directories(${PROJECT_SOURCE_DIR}/../comm-lib/config/tnode/include)

#include_directories(${PROJECT_SOURCE_DIR}/../comm-lib/market/include)
 
#include_directories(${PROJECT_SOURCE_DIR}/../comm-lib/new_tnode/include)
include_directories(${PROJECT_SOURCE_DIR}/../comm-lib/tnode2.0.1/include)
include_directories(${PROJECT_SOURCE_DIR}/../comm-lib/xlogger/include)
include_directories(${PROJECT_SOURCE_DIR}/../comm-lib/xflagger/include)
include_directories(${PROJECT_SOURCE_DIR}/../comm-lib/db/include/)

set(SOURCE_FILE main.cc 
	src/define.cc
	src/xdatetime.cc
	src/ordersvr_subscribe.cc
	src/tnode_adapter.cc)


 
 

link_directories(${PROJECT_SOURCE_DIR}/../comm-lib/tnode2.0.1/lib)
link_directories(${PROJECT_SOURCE_DIR}/../comm-lib/xlogger/lib)
link_directories(${PROJECT_SOURCE_DIR}/../comm-lib/db/lib)
link_directories(${PROJECT_SOURCE_DIR}/../comm-lib/xflagger/lib)
 
 
add_executable(ordersvr ${SOURCE_FILE})
target_link_libraries(ordersvr tnode2 rabbitmq mysqlop redisop   xlogger solution_config xconfig  gflags glog pthread uuid mysqlclient hiredis)

 
