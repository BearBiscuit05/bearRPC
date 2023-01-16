# add_executable(read 
#     RPCTest.cc 
#     utils.h RPCException.h
#     Serialization.h Serialization.cc
#     bearTcpServer.h bearTcpServer.cc
#     bearTcpClinet.h bearTcpClinet.cc
# )

# add_executable(Echoserver test/server.cc)
# add_executable(Echoclinet test/clinet.cc)
# target_link_libraries(Echoserver tinyev)
# target_link_libraries(Echoclinet tinyev)

add_subdirectory(tcp)
