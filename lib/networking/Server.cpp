/////////////////////////////////////////////////////////////////////////////
//                         Single Threaded Networking
//
// This file is distributed under the MIT License. See the LICENSE file
// for details.
/////////////////////////////////////////////////////////////////////////////


#include "Server.h"
#include "TransferMessage.h"

using namespace networking;


/////////////////////////////////////////////////////////////////////////////
// Channels (connections private to the implementation)
/////////////////////////////////////////////////////////////////////////////


namespace networking {

class Server::Channel : public std::enable_shared_from_this<Server::Channel> {
public:
  Channel(boost::asio::io_service& io_service,
          Server& server,
          std::deque<Message> &readBuffer)
    : disconnected{false},
      connection{reinterpret_cast<uintptr_t>(this)},
      socket{io_service},
      server{server},
      streamBuf{BUFFER_SIZE},
      readBuffer{readBuffer}
      { }

  void start();
  void send(std::string outgoing);
  void disconnect();

  boost::asio::ip::tcp::socket & getSocket() { return socket; }
  Connection getConnection() const { return connection; }

  static constexpr unsigned BUFFER_SIZE = 256;

private:
  void readLine();

  bool disconnected;
  Connection connection;
  boost::asio::ip::tcp::socket socket;
  Server &server;
  boost::asio::streambuf streamBuf;
  std::deque<Message> &readBuffer;
  std::deque<std::string> writeBuffer;
};

}


void
Server::Channel::start() {
  readLine();
}


void
Server::Channel::disconnect() {
  disconnected = true;
  socket.cancel();
  socket.close();
}


void
Server::Channel::send(std::string outgoing) {
  if (outgoing.empty()) {
    return;
  }
  ensureMessageTerminator(outgoing);
  writeBuffer.push_back(outgoing);

  auto self = shared_from_this();
  boost::asio::async_write(socket, boost::asio::buffer(writeBuffer.back()),
    [this, self] (auto errorCode, std::size_t size) {
      if (!errorCode) {
        writeBuffer.pop_front();
      } else if (!disconnected) {
        server.disconnect(connection);
      }
    });
}


void
Server::Channel::readLine() {
  auto self = shared_from_this();
  boost::asio::async_read_until(socket, streamBuf, MESSAGE_DELIMITER,
    [this, self] (auto errorCode, std::size_t size) {
      if (!errorCode) {
        readBuffer.push_back({connection, extractMessage(streamBuf)});
        this->readLine();
      } else if (!disconnected) {
        server.disconnect(connection);
      }
    });
}


/////////////////////////////////////////////////////////////////////////////
// Core Server
/////////////////////////////////////////////////////////////////////////////


void
Server::update() {
  ioService.poll();
}


std::deque<Message>
Server::receive() {
  auto oldIncoming = std::move(incoming);
  incoming = std::deque<Message>{};
  return oldIncoming;
}


void
Server::send(const std::deque<Message>& messages) {
  for (auto& message : messages) {
    auto found = channels.find(message.connection);
    if (channels.end() != found) {
      found->second->send(message.text);
    }
  }
}


void
Server::disconnect(Connection connection) {
  auto found = channels.find(connection);
  if (channels.end() != found) {
    connectionHandler->handleDisconnect(connection);
    found->second->disconnect();
    channels.erase(found);
  }
}


void
Server::listenForConnections() {
  auto newChannel =
    std::make_shared<Channel>(acceptor.get_io_service(), *this, this->incoming);

  acceptor.async_accept(newChannel->getSocket(),
    [this, newChannel] (auto errorCode) {
      if (!errorCode) {
        auto connection = newChannel->getConnection();
        channels[connection] = newChannel;
        connectionHandler->handleConnect(connection);
        newChannel->start();
      }
      this->listenForConnections();
  });
}

