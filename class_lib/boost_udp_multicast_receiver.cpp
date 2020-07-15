/*********************************
**  参考：https://blog.csdn.net/yangzhenzhen/article/details/52886760
**      https://blog.csdn.net/gubenpeiyuan/article/details/8562923
**      https://blog.csdn.net/zwl1584671413/article/details/79363317
**********************************/

#include <iostream>
#include <string>
#include <sstream>
#include <array>
#include <chrono>
#include <boost/asio.hpp>

class receiver {
public:
    receiver(boost::asio::io_service &context, const boost::asio::ip::address &listen_addr,
        const boost::asio::ip::address &multicast_addr) : socket_(context) {
        boost::asio::ip::udp::endpoint listen_endpoint(listen_addr, multicast_port_);
        socket_.open(listen_endpoint.protocol());
        socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true));
        socket_.bind(boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), multicast_port_));
        socket_.set_option(boost::asio::ip::multicast::join_group(multicast_addr));
        do_recv();
    }

    void set_multicast_port(int port) {
        multicast_port_ = port;
    }

private:
    void do_recv() {
        socket_.async_receive_from(boost::asio::buffer(buff_), endpoint_, 
            [this](boost::system::error_code ec, std::size_t len) {
            if (!ec) {
                std::cout.write(buff_.data(), len);
                std::cout << std::endl;
                do_recv();
            }
        });
    }

private:
    static constexpr size_t BUFF_SIZE = 1024;

private:
    int multicast_port_ = 30001;
    std::array<char, BUFF_SIZE> buff_;
    boost::asio::ip::udp::endpoint endpoint_;
    boost::asio::ip::udp::socket socket_;
};

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "usage: ./recv 192.168.1.100 239.0.0.2" << std::endl;
        return -1;
    }
    boost::asio::io_service context;
    try {
        receiver myrecv(context,
            boost::asio::ip::address::from_string(argv[1]),
            boost::asio::ip::address::from_string(argv[2])
        );
        context.run();
    }
    catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}
