#include <mdw/formula/Expression.hpp>
#include <mdw/lexical_cast.hpp>

namespace mdw { namespace formula {


  const char TypeTraits<std::string>::kTypeAsString[] = "string";
  const char TypeTraits<bool>::kTypeAsString[] = "bool";
  /*const char TypeTraits<int8_t>::kTypeAsString[] = "int";
  const char TypeTraits<char>::kTypeAsString[] = "int";
  const char TypeTraits<int16_t>::kTypeAsString[] = "int";
  const char TypeTraits<int32_t>::kTypeAsString[] = "int";
  const char TypeTraits<int64_t>::kTypeAsString[] = "int";
  const char TypeTraits<uint8_t>::kTypeAsString[] = "int";
  const char TypeTraits<uint16_t>::kTypeAsString[] = "int";
  const char TypeTraits<uint32_t>::kTypeAsString[] = "int";
  const char TypeTraits<uint64_t>::kTypeAsString[] = "int";*/
  const char TypeTraits<double>::kTypeAsString[] = "double";
  const char TypeTraits<float>::kTypeAsString[] = "double";

  TypedExpression<std::string>& Expression::getString() {
    throw mdw::UnknownException("Expression " + toString() + " is not of type string but " +
                                mdw::lexical_cast<std::string>(getType()));
  }

  TypedExpression<bool>& Expression::getBool() {
    throw mdw::UnknownException("Expression " + toString() + " is not of type bool but " +
                                mdw::lexical_cast<std::string>(getType()));

  }
  TypedExpression<int>& Expression::getInt() {
    throw mdw::UnknownException("Expression " + toString() + " is not of type int but " +
                                mdw::lexical_cast<std::string>(getType()));
  }
  TypedExpression<int64_t>& Expression::getLong() {
    throw mdw::UnknownException("Expression " + toString() + " is not of type long but " +
                                mdw::lexical_cast<std::string>(getType()));

  }
  TypedExpression<double>& Expression::getDouble() {
    throw mdw::UnknownException("Expression " + toString() + " is not of type double but " +
                                mdw::lexical_cast<std::string>(getType()));
  }

  const TypedExpression<std::string>& Expression::getString() const {
    throw mdw::UnknownException("Expression " + toString() + " is not of type string but " +
                                mdw::lexical_cast<std::string>(getType()));
  }

  const TypedExpression<bool>& Expression::getBool() const {
    throw mdw::UnknownException("Expression " + toString() + " is not of type bool but " +
                                mdw::lexical_cast<std::string>(getType()));

  }
  const TypedExpression<int>& Expression::getInt() const {
    throw mdw::UnknownException("Expression " + toString() + " is not of type int but " +
                                mdw::lexical_cast<std::string>(getType()));

  }
  const TypedExpression<int64_t>& Expression::getLong() const {
    throw mdw::UnknownException("Expression " + toString() + " is not of type long but " +
                                mdw::lexical_cast<std::string>(getType()));

  }
  const TypedExpression<double>& Expression::getDouble() const {
    throw mdw::UnknownException("Expression " + toString() + " is not of type double but " +
                                mdw::lexical_cast<std::string>(getType()));
  }

  size_t Expression::complexity() const
  {
    return 1;
  }

  template <> TypedExpression<bool>& Expression::get<bool>()
  {
    return getBool();
  }

  template <> const TypedExpression<bool>& Expression::get<bool>() const
  {
    return getBool();
  }

  template <> TypedExpression<int>& Expression::get<int>()
  {
    return getInt();
  }

  template <> const TypedExpression<int>& Expression::get<int>() const
  {
    return getInt();
  }

  template <> TypedExpression<int64_t>& Expression::get<int64_t>()
  {
    return getLong();
  }

  template <> const TypedExpression<int64_t>& Expression::get<int64_t>() const
  {
    return getLong();
  }

  template <> TypedExpression<std::string>& Expression::get<std::string>()
  {
    return getString();
  }

  template <> const TypedExpression<std::string>& Expression::get<std::string>() const
  {
    return getString();
  }

  template <> TypedExpression<double>& Expression::get<double>()
  {
    return getDouble();
  }

  template <> const TypedExpression<double>& Expression::get<double>() const
  {
    return getDouble();
  }

}}

