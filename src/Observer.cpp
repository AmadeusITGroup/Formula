#include <mdw/formula/Observer.hpp>
#include <mdw/formula/ArenaAllocator.hpp>

namespace mdw { namespace formula {

  Observer::Observer(ArenaAllocator& ioAllocator):
    _allocator(ioAllocator), _subObserver(NULL)
  {}

  void Observer::setObserver(Observer *ioSubObserver)
  {
    _subObserver = ioSubObserver;
  }

}}

