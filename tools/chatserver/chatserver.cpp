/////////////////////////////////////////////////////////////////////////////
//                         Single Threaded Networking
//
// This file is distributed under the MIT License. See the LICENSE file
// for details.
/////////////////////////////////////////////////////////////////////////////


#include "Server.h"

#include <sstream>

#include <unistd.h>


using namespace networking;


std::vector<Connection> clients;


void
onConnect(Connection c) {
  printf("New connection found: %lu\n", c.id);
  clients.push_back(c);
}


void
onDisconnect(Connection c) {
  printf("Connection lost: %lu\n", c.id);
  auto eraseBegin = std::remove(std::begin(clients), std::end(clients), c);
  clients.erase(eraseBegin, std::end(clients));
}


std::string
processMessages(Server &server,
                const std::deque<Message> &incoming,
                bool &quit) {
  std::ostringstream result;
  for (auto& message : incoming) {
    if (message.text == "quit") {
      server.disconnect(message.connection);
    } else if (message.text == "shutdown") {
      printf("Shutting down.\n");
      quit = true;
    } else {
      result << message.connection.id << "> " << message.text << "\n";
    }
  }
  return result.str();
}


std::deque<Message>
buildOutgoing(const std::string& log) {
  std::deque<Message> outgoing;
  for (auto client : clients) {
    outgoing.push_back({client, log});
  }
  return outgoing;
}


int
main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("Usage:\n%s <port>\ne.g. %s 4002\n", argv[0], argv[0]);
    return 1;
  }

  bool done = false;
  unsigned short port = std::stoi(argv[1]);
  Server server{port, onConnect, onDisconnect};

  while (!done) {
    try {
      server.update();
    } catch (std::exception& e) {
      printf("Exception from Server update:\n%s\n\n", e.what());
      done = true;
    }

    auto incoming = server.receive();
    auto log      = processMessages(server, incoming, done);
    auto outgoing = buildOutgoing(log);
    server.send(outgoing);
    sleep(1);
  }

  return 0;
}

