# 指定cmake版本
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

#PROJECT命令表明产生的工作空间的名称
PROJECT(PROJECT_MAKE)

MESSAGE(STATUS "Project Name: ${PROJECT_NAME}")               # 打印相关消息消息 
MESSAGE(STATUS "Project Directory: ${PROJECT_SOURCE_DIR}")

# 设置编译器
SET(CMAKE_CXX_COMPILER "g++")

#设置构建类型，及相应的编译选项
SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb -std=c++11 -DBOOST_LOG_DYN_LINK")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall -std=c++11 -DBOOST_LOG_DYN_LINK")

ENABLE_LANGUAGE(ASM)
#源文件
AUX_SOURCE_DIRECTORY(./                             SOURCE_LIST)
AUX_SOURCE_DIRECTORY(./fastcgi_lib/                 SOURCE_LIST)
AUX_SOURCE_DIRECTORY(../comm/                       SOURCE_LIST)

MESSAGE(STATUS "${SOURCE_LIST}")
MESSAGE(STATUS "${ASM_LIST}")

#头文件搜索目录
INCLUDE_DIRECTORIES(./)
INCLUDE_DIRECTORIES(./fastcgi_lib/)
INCLUDE_DIRECTORIES(../comm)
INCLUDE_DIRECTORIES(/usr/include/jsoncpp)
INCLUDE_DIRECTORIES(/usr/include/hiredis)
INCLUDE_DIRECTORIES(/usr/include)


#链接库搜索目录
LINK_DIRECTORIES(/usr/lib)
LINK_DIRECTORIES(/usr/lib64)
LINK_DIRECTORIES(/usr/local/lib)
LINK_DIRECTORIES(/usr/local/lib64)

#example最后生成的可执行文件名 赋值给BIN_NAME变量
SET(BIN_NAME fastcgi_cgi)

#需要链接的动态库
SET(SO_LIST dl pthread boost_system boost_filesystem boost_log_setup boost_log boost_thread ssl crypto dl z)

MESSAGE(STATUS "${SO_LIST}")


#生成可执行文件
ADD_EXECUTABLE(${BIN_NAME} ${SOURCE_LIST} ${ASM_LIST})

#添加可执行文件所需要的库
TARGET_LINK_LIBRARIES(${BIN_NAME} ${SO_LIST})

#需要链接的静态库
