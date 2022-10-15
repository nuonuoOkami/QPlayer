

cmake_minimum_required(VERSION 3.18.1)


project("qplayer")


add_library( # Sets the name of the library.
        qplayer


        SHARED


        native-lib.cpp)


find_library(
        log-lib


        log)


target_link_libraries(
        qplayer

        ${log-lib})