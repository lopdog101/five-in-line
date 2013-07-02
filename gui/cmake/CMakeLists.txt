cmake_minimum_required (VERSION 2.6)
set(CMAKE_MFC_FLAG 1)

link_directories($ENV{BOOST}/stage/lib)

project (five_in_line)

FILE(GLOB SRC_ALGO_FILES
../../algo/*.cpp
../../algo/*.h
../../algo/*.hpp
)

FILE(GLOB SRC_GUI_FILES
../*.cpp
../*.h
../*.hpp
../*.rc
)

FILE(GLOB SRC_EXTERN_FILES
../../extern/*.cpp
../../extern/*.h
../../extern/*.hpp
)

SOURCE_GROUP(algo FILES ${SRC_ALGO_FILES} )
SOURCE_GROUP(gui FILES ${SRC_GUI_FILES} )
SOURCE_GROUP(extern FILES ${SRC_EXTERN_FILES} )


add_executable(five_in_line WIN32 ${SRC_ALGO_FILES} ${SRC_GUI_FILES} ${SRC_EXTERN_FILES})

add_definitions(-D_AFXDLL -D_CRT_SECURE_NO_WARNINGS)
include_directories($ENV{BOOST})

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()
