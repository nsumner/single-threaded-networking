/////////////////////////////////////////////////////////////////////////////
//                         Single Threaded Networking
//
// This file is distributed under the MIT License. See the LICENSE file
// for details.
/////////////////////////////////////////////////////////////////////////////


#ifndef NETWORKING_CLIENT_H
#define NETWORKING_CLIENT_H

#include <boost/asio.hpp>

#include <deque>
#include <string>
#include <sstream>


namespace networking {


/**
 *  @class Client
 *
 *  @brief A single threaded network client for transferring text.
 *
 *  The Client class transfers text to and from a Server running on a given
 *  IP address and port. The behavior is single threaded, so all transfer
 *  operations are grouped and performed on the next call to Client::update().
 *  Text can be sent to the Server using Client::send() and received from the
 *  Server using Client::receive().
 *
 *  Messages to and from the server may not contain carriage returns ('\r').
 *  These characters are used to delimit individual messages during
 *  transmission.
 */
class Client {
public:
  /**
   *  Construct a Client and acquire a connection to a remote Server at the
   *  given address and port.
   */
  Client(const char* address, const char* port)
    : isClosed{false},
      ioService{},
      socket{ioService} {
    boost::asio::ip::tcp::resolver resolver{ioService};
    connect(resolver.resolve({address, port}));
  }

  /**
   *  Perform all pending sends and receives. This function can throw an
   *  exception if any of the I/O operations encounters an error.
   */
  void update();

  /**
   *  Send a message to the server. The message may not contain carriage
   *  returns.
   */
  void send(std::string message);

  /**
   *  Receive messages from the Server. This returns all messages collected by
   *  previous calls to Client::update() and not yet received. If multiple
   *  messages were received from the Server, they are first concatenated
   *  into a single std::string.
   */
  std::string receive();

  /**
   *  Returns true iff the client disconnected from the server after initally
   *  connecting.
   */
  bool isDisconnected() { return isClosed; }

private:
  void disconnect();

  void connect(boost::asio::ip::tcp::resolver::iterator endpoint);

  void readMessage();

  static constexpr std::size_t BUFFER_SIZE = 256;

  bool isClosed;
  boost::asio::io_service ioService;
  boost::asio::ip::tcp::socket socket;
  boost::asio::streambuf readBuffer;
  std::ostringstream incomingMessage;
  std::deque<std::string> writeBuffer;
};


}


#endif

