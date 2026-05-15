add_library(TapeLib
    FileTape.cpp
    TapeSorter.cpp
)

target_include_directories(TapeLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})