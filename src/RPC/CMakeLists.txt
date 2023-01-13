add_executable(read 
    RPCTest.cc 
    utils.h RPCException.h
    Serialization.h Serialization.cc
    bearTcp.h bearTcp.cc 
)

target_link_libraries(read jackson tinyev)