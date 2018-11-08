#pragma once

#include <exception>
#include <string>

namespace mdw {

  class UnknownException: public std::exception {
    std::string _message;

  public:
    UnknownException(const std::string& iMsg):
      _message(iMsg)
    {}

    ~UnknownException() throw()
    {}

    const char *what() {
      return _message.c_str();
    }

    const char *message() const {
      return _message.c_str();
    }

  };

}

