cmake_minimum_required(VERSION 3.5)

project(cpps_resource_client LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include_directories("C:/Program Files/MySQL/MySQL Server 8.0/include")

link_directories("C:/Program Files/MySQL/MySQL Server 8.0/lib")

link_libraries(libmysql.lib)


link_directories("D:/Qt/Qt5.14.2/Tools/mingw730_64/86_64-w64-mingw32/lib")

link_libraries(libws2_32.a)

link_libraries(libiphlpapi.a)

add_executable(cpps_resource_client WIN32
    main.cpp
    cdb.cpp
    cua.cpp
    open62541.c)
