cmake_minimum_required(VERSION 3.4.1)
add_library( # Sets the name of the library.
        native-lib
        SHARED
        src/main/cpp/native-lib.cpp
        src/main/cpp/dgif_lib.c
        src/main/cpp/gifalloc.c
        )

find_library(
        jnigraphics-lib
        jnigraphics )

find_library( # Sets the name of the path variable.
        log-lib
        log )
target_link_libraries(
        native-lib
        ${log-lib}
        ${jnigraphics-lib}
)