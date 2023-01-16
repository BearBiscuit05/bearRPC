set(SOURCE_FILES
    Timestamp.h test.cc

)

add_library(bearTCP STATIC ${SOURCE_FILES})
target_link_libraries(bearTCP pthread)
install(TARGETS bearTCP DESTINATION lib)

set(HEADERS
    Timestamp.h
)
install(FILES ${HEADERS} DESTINATION include)
