#pragma once

namespace mdw { namespace formula {

  // Cannot use boost::any because it does copies
  // We don't need copies because we're fast-allocated anyway.
  class AnyFact {
  public:
    virtual ~AnyFact() {}
  };

  template <class T> struct TypedFact: public AnyFact
  {
  private:
    T *_object;

  public:
    TypedFact(T& iObject):
      _object(&iObject)
    {}

    void set(T& iObject)
    {
      _object = &iObject;
    }

    const T& get() const
    {
      return *_object;
    }

    T& get()
    {
      return *_object;
    }
  };

}}

