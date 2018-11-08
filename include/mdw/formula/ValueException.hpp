#pragma once

#include <exception>
#include <boost/noncopyable.hpp>

namespace mdw { namespace formula {

  class ValueException: public std::exception
  {
    const char *_message;
  public:
    // Make sure iMessage lives longer than the exception!
    explicit ValueException(const char *iMessage):
      _message(iMessage)
    {}

    ValueException():
      _message(NULL)
    {}

    ~ValueException() throw() {}

    const char *what()
    {
      if (_message)
      {
        return _message;
      } else {
        return "Invalid or missing data to compute the formula";
      }
    }
  };

}}

