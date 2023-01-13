add_executable(read 
    RPCTest.cc Serialization.h Serialization.cc 
)

target_link_libraries(read jackson)