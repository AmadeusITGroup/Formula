#include <mdw/formula/IContext.hpp>
#include <mdw/formula/ArenaAllocator.hpp>

namespace mdw { namespace formula {

  int IContext::LatestUniqueId = 0;

  IContext::IContext(ArenaAllocator& ioAllocator):
    _allocator(ioAllocator), _uniqueId(++LatestUniqueId),
    _ownsAllocator(false), _invalidExpression(false)
  {}

  IContext::IContext():
    _allocator(*(new ArenaAllocator())), _uniqueId(++LatestUniqueId),
    _ownsAllocator(true), _invalidExpression(false)
  {}

  IContext::~IContext()
  {
    if (_ownsAllocator)
    {
      delete &_allocator;
    }
  }

  const ArenaAllocator& IContext::getAllocator() const
  {
    return _allocator;
  }

  ArenaAllocator& IContext::getAllocator()
  {
    return _allocator;
  }

  void IContext::clean()
  {
    _knownFacts.clear();
    if (_ownsAllocator)
    {
      _allocator.clean();
    }
    _invalidExpression = false;
    _uniqueId = ++LatestUniqueId;
  }
}}

