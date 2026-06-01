add_executable(EventLoopThreadtest EventLoopThreadtest.cpp)
target_link_libraries(EventLoopThreadtest network base)

add_executable(InetAddressTest InetAddressTest.cpp)
target_link_libraries(InetAddressTest network base)

add_executable(TcpClentTest TcpClentTest.cpp)
target_link_libraries(TcpClentTest network base)

add_executable(TcpServerTest TcpServerTest.cpp)
target_link_libraries(TcpServerTest network base)

add_executable(UdpClentTest UdpClentTest.cpp)
target_link_libraries(UdpClentTest network base)

add_executable(UdpServerTest UdpServerTest.cpp)
target_link_libraries(UdpServerTest network base)