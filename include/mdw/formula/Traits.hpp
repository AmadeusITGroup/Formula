#pragma once
#include <string>
#include <typeinfo>
#include <boost/type_traits/is_integral.hpp>
#include <boost/utility/enable_if.hpp>
#include <mdw/formula/ArenaAllocator.hpp>
#include <stdint.h>

namespace mdw { namespace formula {

  typedef size_t ExpressionType;

  static const ExpressionType kExprVoid = 0;
  static const ExpressionType kExprString = 1;
  static const ExpressionType kExprInt = 2;
  static const ExpressionType kExprDouble = 3;
  /*static const ExpressionType kExprDate = 4;
  static const ExpressionType kExprTime = 5;*/
  static const ExpressionType kExprBool = 6;
  static const ExpressionType kExprFact = 7;
  static const ExpressionType kExprMaxType = 20;

  // Traits for object types
  template <class T, class ConditionT = void> struct TypeTraits
  {
    typedef const T& ReturnType;
    static const char *kTypeAsString;
  };

  template <> struct TypeTraits <std::string, void>
  {
    typedef const std::string& ReturnType;
    static const char kTypeAsString[];
  };

  template <> struct TypeTraits <bool, void>
  {
    typedef bool ReturnType;
    static const char kTypeAsString[];
  };

  template <class T> struct TypeTraits <T, typename boost::enable_if<boost::is_integral<T> >::type>
  {
    typedef int64_t ReturnType;
    static const char *kTypeAsString;
  };

  /*template <> struct TypeTraits <int>
  {
    typedef int64_t ReturnType;
    static const char kTypeAsString[];
  };*/

  /*template <> struct TypeTraits <char>
  {
    typedef int64_t ReturnType;
    static const char kTypeAsString[];
  };

  template <> struct TypeTraits <int64_t>
  {
    typedef int64_t ReturnType;
    static const char kTypeAsString[];
  };

  template <> struct TypeTraits <int16_t>
  {
    typedef int64_t ReturnType;
    static const char kTypeAsString[];
  };

  template <> struct TypeTraits <uint16_t>
  {
    typedef int64_t ReturnType;
    static const char kTypeAsString[];
  };

  template <> struct TypeTraits <int8_t>
  {
    typedef int64_t ReturnType;
    static const char kTypeAsString[];
  };

  template <> struct TypeTraits <uint8_t>
  {
    typedef int64_t ReturnType;
    static const char kTypeAsString[];
  };

  template <> struct TypeTraits <int32_t>
  {
    typedef int64_t ReturnType;
    static const char kTypeAsString[];
  };

  template <> struct TypeTraits <uint32_t>
  {
    typedef int64_t ReturnType;
    static const char kTypeAsString[];
  };

  template <> struct TypeTraits <uint64_t>
  {
    typedef int64_t ReturnType;
    static const char kTypeAsString[];
  };*/

  template <> struct TypeTraits <double, void>
  {
    typedef double ReturnType;
    static const char kTypeAsString[];
  };

  template <> struct TypeTraits <float, void>
  {
    typedef double ReturnType;
    static const char kTypeAsString[];
  };

  template <class T, class ConditionT = void> class __TypeTraits
  {
  public:
    typedef T actual_type;
    static const bool _IsBase = true;
    typedef T cached_type;
    typedef T key_type;
    static cached_type ToCached(T iValue) {
      return iValue;
    }
    static T FromCached(cached_type iValue) {
      return iValue;
    }
  };

  template <class T> class __TypeTraits <T, typename boost::enable_if<boost::is_integral<T> >::type>
  {
  public:
    typedef int64_t actual_type;
    static const bool _IsBase = true;
    static const void *ToVoid(T iValue) {
      return (void*)iValue;
    }
    typedef T cached_type;
    typedef T key_type;
    static cached_type ToCached(T iValue) {
      return iValue;
    }
    static T FromCached(cached_type iValue) {
      return iValue;
    }
  };

  template <> class __TypeTraits <bool, void>
  {
  public:
    typedef bool actual_type;
    static const bool _IsBase = true;
    static const void *ToVoid(bool iValue) {
      return (void*)iValue;
    }
    typedef bool cached_type;
    typedef bool key_type;
    static cached_type ToCached(bool iValue) {
      return iValue;
    }
    static bool FromCached(cached_type iValue) {
      return iValue;
    }
  };

  template <> class __TypeTraits <float, void>
  {
  public:
    typedef float actual_type;
    static const bool _IsBase = true;
    static const void *ToVoid(float iValue) {
      return (void*)(int64_t)iValue;
    }
    typedef float cached_type;
    typedef float key_type;
    static cached_type ToCached(float iValue) {
      return iValue;
    }
    static float FromCached(cached_type iValue) {
      return iValue;
    }
  };

  template <> class __TypeTraits <double, void>
  {
  public:
    typedef double actual_type;
    static const bool _IsBase = true;
    static const void *ToVoid(double iValue) {
      return (void*)(int64_t)iValue;
    }
    typedef double cached_type;
    typedef double key_type;
    static cached_type ToCached(double iValue) {
      return iValue;
    }
    static double FromCached(cached_type iValue) {
      return iValue;
    }
  };

  template <class T, class U> class __TypeTraits<T*, U>
  {
  public:
    typedef T actual_type;
    static const bool _IsBase = false;
    typedef T* cached_type;
    static cached_type ToCached(T* iValue) {
      return iValue;
    }
    static T* FromCached(cached_type iValue) {
      return iValue;
    }
  };

  template <class T, class U> class __TypeTraits<T&, U>
  {
  public:
    typedef T actual_type;
    static const bool _IsBase = false;
    static const void *ToVoid(const T& iValue) {
      return &iValue;
    }
    typedef T* cached_type;
    typedef T key_type;
    static cached_type ToCached(T& iValue) {
      return &iValue;
    }
    static T& FromCached(cached_type iValue) {
      return *iValue;
    }
  };

  template <class T, class U> class __TypeTraits<const T*, U>
  {
  public:
    typedef T actual_type;
    static const bool _IsBase = false;
    typedef const T* cached_type;
    static cached_type ToCached(const T* iValue) {
      return iValue;
    }
    static const T *FromCached(cached_type iValue) {
      return iValue;
    }
  };

  template <class T, class U> class __TypeTraits<const T&, U>
  {
  public:
    typedef T actual_type;
    static const bool _IsBase = false;
    static const void *ToVoid(const T& iValue) {
      return &iValue;
    }
    typedef const T* cached_type;
    typedef T key_type;
    static cached_type ToCached(const T& iValue) {
      return &iValue;
    }
    static const T& FromCached(cached_type iValue) {
      return *iValue;
    }
  };

  template <class T> const char *TypeTraits<T, typename boost::enable_if<boost::is_integral<T> >::type>::kTypeAsString = "int";

  template <class T, class U> const char *TypeTraits<T, U>::kTypeAsString = typeid(T).name();

  template <class T> class CopyIfNeeded
  {
    static const T& Copy(const T& iValue, ArenaAllocator& ioAllocator)
    {
      return ioAllocator.create<T>(iValue);
    }
  };

  template <class T> class CopyIfNeeded<T&>
  {
    static const T& Copy(const T& iValue, ArenaAllocator& ioAllocator)
    {
      return iValue;
    }
  };

  template <class T> class CopyIfNeeded<T*>
  {
    static const T& Copy(const T& iValue, ArenaAllocator& ioAllocator)
    {
      return iValue;
    }
  };

}}

