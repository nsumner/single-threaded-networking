/////////////////////////////////////////////////////////////////////////////
//                         Single Threaded Networking
//
// This file is distributed under the MIT License. See the LICENSE file
// for details.
/////////////////////////////////////////////////////////////////////////////


#ifndef NETWORKING_SERVER_H
#define NETWORKING_SERVER_H

#include <boost/asio.hpp>

#include <deque>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>


namespace networking {


/**
 *  An identifier for a Client connected to a Server. The ID of a Connection is
 *  guaranteed to be unique across all actively connected Client instances.
 */
struct Connection {
  uintptr_t id;

  bool
  operator==(Connection other) const {
    return id == other.id;
  }
};


struct ConnectionHash {
  size_t
  operator()(Connection c) const {
    return std::hash<decltype(c.id)>{}(c.id);
  }
};


/**
 *  A Message containing text that can be sent to or was recieved from a given
 *  Connection.
 */
struct Message {
  Connection connection;
  std::string text;
};


/**
 *  @class Server
 *
 *  @brief A single threaded network server for transferring text.
 *
 *  The Server class transfers text to and from multiple Client instances
 *  connected on a given port. The behavior is single threaded, so all transfer
 *  operations are grouped and performed on the next call to Server::update().
 *  Text can be sent to the Server using Client::send() and received from the
 *  Server using Client::receive().
 *
 *  Messages to and from the server may not contain carriage returns ('\r').
 *  These characters are used to delimit individual messages during
 *  transmission.
 */
class Server {
public:
  /**
   *  Construct a Server that listens for connections on the given port.
   *  The onConnect and onDisconnect arguments are callbacks called when a
   *  Client connects or disconnects from the Server respectively.
   *
   *  The callbacks can be functions, function pointers, lambdas, or any other
   *  callable construct. They should support the signature:
   *      void onConnect(Connection c);
   *      void onDisconnect(Connection c);
   *  The Connection class is an identifier for each connected Client. It
   *  contains an ID that is guaranteed to be unique across all active
   *  connections.
   */
  template <typename C, typename D>
  Server(unsigned short port, C onConnect, D onDisconnect)
    : connectionHandler{std::make_unique<ConnectionHandlerImpl<C,D>>(onConnect, onDisconnect)},
      endpoint{boost::asio::ip::tcp::v4(), port},
      ioService{},
      acceptor{ioService, endpoint} {
    listenForConnections();
  }

  /**
   *  Perform all pending sends and receives. This function can throw an
   *  exception if any of the I/O operations encounters an error.
   */
  void update();

  /**
   *  Send a list of messages to their respective Clients. The messages may not
   *  contain carriage returns.
   */
  void send(const std::deque<Message>& messages);

  /**
   *  Receive Message instances from Client instances. This returns all Message
   *  instances collected by previous calls to Server::update() and not yet
   *  received.
   */
  std::deque<Message> receive();

  /**
   *  Disconnect the Client specified by the given Connection.
   */
  void disconnect(Connection connection);

private:
  // Hiding the template parameters of the Server class behind a pointer to
  // a private interface allows us to refer to an unparameterized Server
  // object while still having the handlers of connect & disconnect be client
  // defined types. This is a form of *type erasure*.
  class ConnectionHandler {
  public:
    virtual ~ConnectionHandler() = default;
    virtual void handleConnect(Connection) = 0;
    virtual void handleDisconnect(Connection) = 0;
  };

  template <typename C, typename D>
  class ConnectionHandlerImpl final : public ConnectionHandler {
  public:
    ConnectionHandlerImpl(C onConnect, D onDisconnect)
      : onConnect{std::move(onConnect)},
        onDisconnect{std::move(onDisconnect)}
        { }
    ~ConnectionHandlerImpl() override = default;
    void handleConnect(Connection c)    override { onConnect(c);    }
    void handleDisconnect(Connection c) override { onDisconnect(c); }
  private:
    C onConnect;
    D onDisconnect;
  };

  class Channel;
  using ChannelMap =
    std::unordered_map<Connection, std::shared_ptr<Channel>, ConnectionHash>;

  void listenForConnections();

  const std::unique_ptr<ConnectionHandler> connectionHandler;
  const boost::asio::ip::tcp::endpoint endpoint;
  boost::asio::io_service ioService;
  boost::asio::ip::tcp::acceptor acceptor;
  ChannelMap channels;
  std::deque<Message> incoming;
};


}


#endif

