add_library(minixml STATIC xml.c xml.h)
target_include_directories(minixml PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
