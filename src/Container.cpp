#include <mdw/formula/Container.hpp>
#include <mdw/formula/Expression.hpp>
#include <mdw/formula/Grammar.hpp>
#include <mdw/formula/Parser.hpp>
#include <string>

namespace mdw { namespace formula {

  Container::Container(const std::string& iFormula, const Grammar& iGrammar):
    _allocator(*new ArenaAllocator()),
    _knownTypes(iGrammar),
    _topExpression(Parser(_allocator, iGrammar, iFormula).getTopExpression())
  {
  }
  
  Container::~Container()
  {
    delete (&_allocator);
  }

  const Expression& Container::getExpression() const
  {
    return _topExpression;
  }

  const Grammar& Container::getKnownTypes() const
  {
    return _knownTypes;
  }

}}
