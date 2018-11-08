////////////////////////////////////////////////////////////////////////////////
/// Copyright of this program is the property of AMADEUS, without
/// whose written permission reproduction in whole or in part is prohibited.
////////////////////////////////////////////////////////////////////////////////

#include <mdw/formula/Expression.hpp>
#include <mdw/formula/Operator.hpp>
#include <mdw/formula/IContext.hpp>
#include <mdw/formula/Grammar.hpp>
#include <mdw/formula/Constant.hpp>
#include <mdw/formula/StandardTypes.hpp>
#include <mdw/formula/Facts.hpp>

#include <mdw/Tracer.hpp>
#include <boost/mem_fn.hpp>
#include <iostream>

namespace mdw { namespace formula {
  class Pouet;
}}

#define ASSERT_TRUE(x) if (!(x)) {std::cerr << "Failed to check: " #x << std::endl;}

namespace mdw { namespace formula {

  class Pouet {
    int64_t _foo;
  public:

    friend std::ostream& operator<<(std::ostream& oStream, const mdw::formula::Pouet& iFact)
    {
      oStream << "Pouet with foo: " << iFact.getFoo();
      return oStream;
    }

    Pouet(): _foo(456)
    {}

    int64_t getFoo() const
    {
      return _foo;
    }
  } gPouet;


  std::string kStringValue("PouetValue");

  ConstExpression<std::string> gString(kStringValue);
  ConstExpression<int> gInt(524);
  ConstExpression<double> gDouble(35.);
  ConstExpression<bool> gBool(true);


  int TestPlus()
  {
    ConstExpression<int> aInt(87);
    Grammar aGrammar;
    static const std::string kPlus("+");
    SymmetricTypedOperator<int, int, std::plus<int64_t> > aPlusInt(gInt, aInt, aGrammar, kPlus);
    IContext aContext;
    FORMULA_DEBUG("Formula is: " << aPlusInt.toString() << " with value " << aPlusInt.evaluate(aContext));
    ASSERT_TRUE(aPlusInt.evaluate(aContext) == (524+87));

    return 0;
  }

  int TestChoice()
  {
    Grammar aGrammar;
    ConstExpression<int> aInt(93);
    ChoiceOperator<int> aChoice(gBool, aGrammar, aInt, gInt);
    IContext aContext;
    FORMULA_DEBUG("Formula is: " << aChoice.toString() << " with value " << aChoice.evaluate(aContext));
    ASSERT_TRUE(aChoice.evaluate(aContext) == 93);

    ChoiceOperator<int> aChoice2(gBool, aGrammar, gInt, aInt);

    ASSERT_TRUE(aChoice2.evaluate(aContext) == 524);
    return 0;
  }

  int TestString()
  {
    std::string *aStr = new std::string("PouetValue");
    Grammar aGrammar;
    ConstExpression<std::string> aString(*aStr);
    SymmetricTypedOperator<std::string, bool, std::equal_to<std::string> > aEqualString(gString, aString, aGrammar, "==");
    IContext aContext;
    ASSERT_TRUE(gString.toString() == "'PouetValue'");

    ASSERT_TRUE(gString.evaluate(aContext) == "PouetValue");
    ASSERT_TRUE(aEqualString.evaluate(aContext) == true);
    delete aStr;
    return 0;
  }

  int AttributeTest()
  {
    IContext aContext;
    Grammar aGrammar;
    ArenaAllocator aAllocator;
    aGrammar.registerType<Pouet>();
    ConstExpression<Pouet> aFactExpr(gPouet, aGrammar);
    TypedExpression<int>& aResolver =
      formula::make_resolver(aAllocator, aGrammar, aFactExpr, boost::mem_fn(&Pouet::getFoo), "foo");

    ASSERT_TRUE(aResolver.evaluate(aContext) == 456);
    return 0;
  }


  int AllFormulaTests() {
    int aResult = 0;
    aResult += TestPlus();
    aResult += TestChoice();
    aResult += TestString();
    aResult += AttributeTest();
    return aResult;
  }

}}
