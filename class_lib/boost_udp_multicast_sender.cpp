/*********************************
**  参考：https://blog.csdn.net/yangzhenzhen/article/details/52886760
**      https://blog.csdn.net/gubenpeiyuan/article/details/8562923
**      https://blog.csdn.net/zwl1584671413/article/details/79363317
**********************************/

#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <boost/asio.hpp>

class sender {
public:
    sender(boost::asio::io_service &context, const boost::asio::ip::address &addr) 
        : endpoint_(addr, multicast_port_),
        socket_(context, endpoint_.protocol()),
        timer_(context) {
        do_send();
    }

    void set_message_limit(int l) {
        message_limit_ = l;
    }

    void set_multicast_port(int port) {
        multicast_port_ = port;
    }

private:
    void do_send() {
        std::ostringstream os;
        os << "message" << message_count_++;
        message_ = os.str();
        socket_.async_send_to(boost::asio::buffer(message_), endpoint_, 
            [this](boost::system::error_code ec, std::size_t) {
            if (!ec && this->message_count_ < message_limit_) {
                do_timeout();
            }

        });
    }

    void do_timeout() {
        timer_.expires_from_now(boost::posix_time::seconds(1));
        timer_.async_wait([this](boost::system::error_code ec) {
            if (!ec) {
                this->do_send();
            }

        });
    }

private:
    int message_count_ = 0;
    int message_limit_ = 100;
    int multicast_port_ = 30001;
    std::string message_;
    boost::asio::ip::udp::endpoint endpoint_;
    boost::asio::ip::udp::socket socket_;
    boost::asio::deadline_timer timer_;
};

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "usage:./sender 239.0.0.2" << std::endl;
        return -1;
    }

    boost::asio::io_service context;
    try {
        sender mysender(context, boost::asio::ip::address::from_string(argv[1]));
        context.run();
    }
    catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}
