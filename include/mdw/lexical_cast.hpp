////////////////////////////////////////////////////////////////////////////////
/// Copyright of this program is the property of AMADEUS, without
/// whose written permission reproduction in whole or in part is prohibited.
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <boost/lexical_cast.hpp>
#include <string>
#include <mdw/UnknownException.hpp>

namespace mdw {

  template<typename Target, typename Source>
    Target lexical_cast(const Source& iArg) {

    try{
      Target aTarget = boost::lexical_cast<Target>(iArg);
      return aTarget;
    }
      catch (boost::bad_lexical_cast& aException){
        mdw::UnknownException aEx(boost::lexical_cast<std::string>(iArg) + " cannot be cast (" + aException.what() + ")");
        throw aEx;
      }
  }

/*  template<typename Source>
  std::string lexical_cast(const Source& iArg) {
    return boost::lexical_cast<std::string>(iArg);
  }*/

}
