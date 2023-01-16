set(SOURCE_FILES
    Timestamp.h
    InetAddress.h InetAddress.cc
    Logger.h Logger.c
)

add_library(bearTCP STATIC ${SOURCE_FILES})
target_link_libraries(bearTCP pthread)
install(TARGETS bearTCP DESTINATION lib)

set(HEADERS
    Timestamp.h
    InetAddress.h
    Logger.h
)
install(FILES ${HEADERS} DESTINATION include)
