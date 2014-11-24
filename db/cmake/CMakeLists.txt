cmake_minimum_required (VERSION 2.6)

if( WIN32)
    link_directories($ENV{BOOST}/stage/lib)
    link_directories(/distr/mysql-win32/lib)
endif()

project (db)

FILE(GLOB SRC_ALGO_FILES
../../algo/*.cpp
../../algo/*.h
../../algo/*.hpp
)

FILE(GLOB SRC_DB_FILES
../*.cpp
../*.h
../*.hpp
)

FILE(GLOB SRC_EXTERN_FILES
../../extern/*.cpp
../../extern/*.h
../../extern/*.hpp
)


SOURCE_GROUP(algo FILES ${SRC_ALGO_FILES} )
SOURCE_GROUP(db FILES ${SRC_DB_FILES} )
SOURCE_GROUP(extern FILES ${SRC_EXTERN_FILES} )


add_executable(db ${SRC_ALGO_FILES} ${SRC_DB_FILES} ${SRC_EXTERN_FILES})

add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)

if(NOT WIN32)
    add_definitions(-D_FILE_OFFSET_BITS=64)
    TARGET_LINK_LIBRARIES(db boost_filesystem boost_signals boost_system mysqlclient)
	include_directories(/usr/include/mysql)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

if( MSVC)
    TARGET_LINK_LIBRARIES(db libmysql)
    include_directories($ENV{BOOST})
	include_directories(/distr/mysql-win32/include)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()
