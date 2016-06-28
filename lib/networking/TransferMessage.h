/////////////////////////////////////////////////////////////////////////////
//                         Single Threaded Networking
//
// This file is distributed under the MIT License. See the LICENSE file
// for details.
/////////////////////////////////////////////////////////////////////////////


#ifndef NETWORKING_TRANSFER_MESSAGE
#define NETWORKING_TRANSFER_MESSAGE

#include <istream>
#include <streambuf>
#include <string>


static constexpr char MESSAGE_DELIMITER = '\r';


static inline void
ensureMessageTerminator(std::string &message) {
  if (message.back() != MESSAGE_DELIMITER) {
    message.push_back(MESSAGE_DELIMITER);
  }
}


static inline std::string
extractMessage(std::streambuf &buffer) {
  std::string message;
  std::istream is(&buffer);
  std::getline(is, message, MESSAGE_DELIMITER);
  return message;
}


#endif

