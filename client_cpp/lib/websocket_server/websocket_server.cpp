#include "websocket_server.h"

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>
namespace websocket = boost::beast::websocket;  // from <boost/beast/websocket.hpp>


//------------------------------------------------------------------------------

// Report a failure
void
fail(boost::system::error_code ec, char const* what)
{
	std::cerr << what << ": " << ec.message() << "\n";
}

// Echoes back all received WebSocket messages
template<typename Caller>
class session : public std::enable_shared_from_this<session<Caller>>
{
	websocket::stream<tcp::socket> ws_;
	boost::asio::strand<
		boost::asio::io_context::executor_type> strand_;
	boost::beast::multi_buffer buffer_;
	std::string const  receiver_id_;
	Caller* caller_{ nullptr };

	void
		on_fail()
	{
		caller_->unregister(UI_SERVER_ID);
	}
public:
	// Take ownership of the socket
	explicit
		session(tcp::socket socket, std::string const receiver, Caller* caller)
		: ws_(std::move(socket))
		, strand_(ws_.get_executor())
		, receiver_id_{ receiver }
		, caller_{ caller }
	{
	}

	// Start the asynchronous operation
	void
		run()
	{
		// Accept the websocket handshake
		ws_.async_accept(
			boost::asio::bind_executor(
				strand_,
				std::bind(
					&session::on_accept,
					shared_from_this(),
					std::placeholders::_1)));
	}

	void
		on_accept(boost::system::error_code ec)
	{
		if (ec)
			return fail(ec, "accept");
		//todo: inform connection
		//inform caller for accept
		//2. register its sender with function register method
		// Read a message
		caller_->register_id(UI_SERVER_ID, std::bind(&session::send,
			shared_from_this(), std::placeholders::_1));
		do_read();
	}

	void
		do_read()
	{
		// Read a message into our buffer
		ws_.async_read(
			buffer_,
			boost::asio::bind_executor(
				strand_,
				std::bind(
					&session::on_read,
					shared_from_this(),
					std::placeholders::_1,
					std::placeholders::_2)));
	}

	void
		on_read(
			boost::system::error_code ec,
			std::size_t bytes_transferred)
	{
		boost::ignore_unused(bytes_transferred);

		// This indicates that the session was closed
		if (ec == websocket::error::closed)
			return on_fail();

		if (ec) {
			on_fail();
			return fail(ec, "read");
		}
			
		std::string received = boost::beast::buffers_to_string(buffer_.data());
		assert(received.size() == buffer_.size());
		buffer_.consume(buffer_.size());
		do_read();
		caller_->dispatch(receiver_id_, received);

	}

	void send(std::string m) {
		ws_.text(true);
		auto r = ws_.write(boost::asio::buffer(m));
		assert(r == m.size());
	}

};

//------------------------------------------------------------------------------

// Accepts incoming connections and launches the sessions
template< typename Caller>
class listener : public std::enable_shared_from_this<listener<Caller>>
{
	tcp::acceptor acceptor_;
	tcp::socket socket_;
	Caller* caller_{ nullptr };
	std::string receiver_id_;
public:
	listener(
		boost::asio::io_context& ioc,
		tcp::endpoint endpoint)
		: acceptor_(ioc)
		, socket_(ioc)
	{
		boost::system::error_code ec;

		// Open the acceptor
		acceptor_.open(endpoint.protocol(), ec);
		if (ec)
		{
			fail(ec, "open");
			return;
		}

		// Allow address reuse
		acceptor_.set_option(boost::asio::socket_base::reuse_address(true), ec);
		if (ec)
		{
			fail(ec, "set_option");
			return;
		}

		// Bind to the server address
		acceptor_.bind(endpoint, ec);
		if (ec)
		{
			fail(ec, "bind");
			return;
		}

		// Start listening for connections
		acceptor_.listen(
			boost::asio::socket_base::max_listen_connections, ec);
		if (ec)
		{
			fail(ec, "listen");
			return;
		}
	}

	// Start accepting incoming connections
	void
		run(Caller* caller, std::string receiver_id)
	{
		if (!acceptor_.is_open())
			return;
		caller_ = caller;
		receiver_id_ = receiver_id;
		do_accept();
	}

	void
		do_accept()
	{
		acceptor_.async_accept(
			socket_,
			std::bind(
				&listener::on_accept,
				shared_from_this(),
				std::placeholders::_1));
	}

	void
		on_accept(boost::system::error_code ec)
	{
		if (ec)
		{
			fail(ec, "accept");
		}
		else
		{
			// Create the session and run it
			std::make_shared<session<Caller>>(std::move(socket_), receiver_id_, caller_)->run();
		}

		// Accept another connection
		do_accept();
	}
};

namespace grt {
	void start_server_block(unsigned short port, int threads, util::func_thread_handler* caller, std::string const receiver_id) {
		auto const address = boost::asio::ip::make_address("0.0.0.0");

		// The io_context is required for all I/O
		boost::asio::io_context ioc{ threads };

		// Create and launch a listening port
		std::make_shared<listener< util::func_thread_handler>>(
			ioc, tcp::endpoint{ address, port })->run(caller, receiver_id);

		// Run the I/O service on the requested number of threads
		std::vector<std::thread> v;
		v.reserve(threads - 1);
		for (auto i = threads - 1; i > 0; --i)
			v.emplace_back(
				[&ioc]
		{
			ioc.run();
		});
		ioc.run();

		return;
	}
}//namespace grt