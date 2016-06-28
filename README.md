
# Single Threaded Networking

This repository contains an example library for single threaded client/server
programs based on boost asio. Multiple clients and the server can transfer
simple strings messages back and forth. Because the API is single threaded, it
integrates easily into a main update loop, e.g. for a game.

*Note:* Both the creation of the communication channel as well as all
communication between the client and the server is insecure. It is trivially
subject to interception and alteration, and it should not be used to transmit
sensitive information of any sort.

In addition, a simple chat server and NCurses based chat client demonstrate
how to use this API.


## Building with CMake

1. Clone the repository.

        git clone https://github.com/nsumner/single-threaded-networking.git

2. Create a new directory for building.

        mkdir networkbuild

3. Change into the new directory.

        cd networkbuild

4. Run CMake with the path to the source.

        cmake ../single-threaded-networking/

5. Run make inside the build directory:

        make

This produces `chatserver` and `chatclient` tools called `bin/chatserver` and
`bin/chatclient` respectively. The library for single threaded clients and
servers is built in `lib/`.

Note, building with a tool like ninja can be done by adding `-G Ninja` to
the cmake invocation and running `ninja` instead of `make`.


## Running the Example Chat Client and Chat Server

First run the chat server on an unused port of the server machine.

    bin/chatserver 4000

In separate terminals, run multiple instances of the chat client using:

    bin/chatclient localhost 4000

This will connect to the given port (4000 in this case) of the local machine.
Connecting to a remote machine can be done by explicitly using the remote
machine's IP address instead of `localhost`. Inside the chat client, you can
enter commands or chat with other clients by typing text and hitting the
ENTER key. You can disconnect from the server by typing `quit`. You can shut
down the server and disconnect all clients by typing `shutdown`. Typing
anything else will send a chat message to other clients.

