/////////////////////////////////////////////////////////////////////////////
//                         Single Threaded Networking
//
// This file is distributed under the MIT License. See the LICENSE file
// for details.
/////////////////////////////////////////////////////////////////////////////


#include "Client.h"
#include "TransferMessage.h"

using namespace networking;


/////////////////////////////////////////////////////////////////////////////
// Core Client
/////////////////////////////////////////////////////////////////////////////


void
Client::update() {
  ioService.poll();  
}


std::string
Client::receive() {
  auto result = incomingMessage.str();
  incomingMessage.str(std::string{});
  incomingMessage.clear();
  return result;
}


void
Client::send(std::string message) {
  if (message.empty()) {
    return;
  }
  ensureMessageTerminator(message);

  writeBuffer.emplace_back(std::move(message));
  boost::asio::async_write(socket, boost::asio::buffer(writeBuffer.back()),
    [this] (auto errorCode, std::size_t size) {
      if (!errorCode) {
        writeBuffer.pop_front();
      } else {
        this->disconnect();
      }
    });
}


void
Client::disconnect() {
  isClosed = true;
  socket.close();
}


void
Client::connect(boost::asio::ip::tcp::resolver::iterator endpoint) {
  boost::asio::async_connect(socket, endpoint,
    [this] (auto errorCode, auto) {
      if (!errorCode) {
        this->readMessage();
      }
    });
}


void
Client::readMessage() {
  boost::asio::async_read_until(socket, readBuffer, MESSAGE_DELIMITER,
    [this] (auto errorCode, std::size_t size) {
      if (!errorCode) {
        if (size > 0) {
          auto message = extractMessage(readBuffer);
          incomingMessage.write(message.c_str(), message.size());
          this->readMessage();
        }
      } else {
        this->disconnect();
      }
    });
}

