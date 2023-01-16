#include <bearTCP/InetAddress.h>
#include <iostream>

using namespace bear;
int main()
{
    InetAddress addr(8888);
    std::cout<<"addr ip:"<<addr.toIp()<<std::endl;
    std::cout<<"addr port:"<<addr.toPort() <<std::endl;
    std::cout<<"addr ip&port:"<<addr.toIpPort()<<std::endl;
}