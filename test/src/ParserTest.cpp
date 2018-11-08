////////////////////////////////////////////////////////////////////////////////
/// Copyright of this program is the property of AMADEUS, without
/// whose written permission reproduction in whole or in part is prohibited.
////////////////////////////////////////////////////////////////////////////////


#include <mdw/formula/Parser.hpp>
#include <mdw/formula/Container.hpp>
#include <mdw/formula/IContext.hpp>
#include <mdw/formula/Grammar.hpp>
#include <mdw/formula/StandardTypes.hpp>
#include <mdw/formula/Repeated.hpp>
#include <mdw/formula/Casts.hpp>
#include <mdw/formula/cache/Factorizer.hpp>
#include <mdw/formula/Facts.hpp>
#include <mdw/Tracer.hpp>
#include <boost/mem_fn.hpp>
#include <iostream>

#define ASSERT_TRUE(x) if (!(x)) {std::cerr << "Failed to check: " #x << std::endl; return 1;}
#define ASSERT_FALSE(x) if (x) {std::cerr << "Failed to fail: " #x << std::endl; return 1;}
#define ASSERT_EQ(x,y) ASSERT_TRUE(x == y)

namespace mdw { namespace formula {

  int ConstantBool()
  {
    Grammar aGrammar;
    std::string aTest("true");
    Container aContainer(aTest, aGrammar);

    IContext aContext;
    ASSERT_TRUE(aContainer.getExpression().getBool().evaluate(aContext) == true);
    FORMULA_DEBUG("Expression is: " << aContainer.getExpression().getBool().toString());
    ASSERT_TRUE(aContainer.getExpression().getBool().toString() == "true");
    return 0;
  }


  int ConstantInt()
  {
    ArenaAllocator aAlloc;
    Grammar aGrammar;
    std::string aTest("64");
    aGrammar.registerStandardOperators(aAlloc);
    Container aContainer(aTest, aGrammar);

    IContext aContext;
    ASSERT_TRUE(aContainer.getExpression().getInt().evaluate(aContext) == 64);
    return 0;
  }


  int ConstantDouble()
  {
    Grammar aGrammar;
    std::string aTest(".684");
    Container aContainer(aTest, aGrammar);

    IContext aContext;
    ASSERT_TRUE(aContainer.getExpression().getDouble().evaluate(aContext) == .684);
    return 0;
  }


  int ConstantString()
  {
    Grammar aGrammar;
    std::string aTest("('Pouet' )");
    Container aContainer(aTest, aGrammar);

    IContext aContext;
    ASSERT_TRUE(aContainer.getExpression().getType() == mdw::formula::kExprString);
    FORMULA_DEBUG("Expression is: " << aContainer.getExpression().getString().toString());
    ASSERT_TRUE(aContainer.getExpression().getString().evaluate(aContext) == "Pouet");
    return 0;
  }

  int NotBool()
  {
    ArenaAllocator aAlloc;
    Grammar aGrammar;
    aGrammar.registerStandardOperators(aAlloc);
    std::string aTest("!true");
    Container aContainer(aTest, aGrammar);

    IContext aContext;
    ASSERT_TRUE(aContainer.getExpression().getBool().evaluate(aContext) == false);
    return 0;
  }


  int MinusInt()
  {
    ArenaAllocator aAlloc;
    Grammar aGrammar;
    aGrammar.registerStandardOperators(aAlloc);
    std::string aTest("-64");
    Container aContainer(aTest, aGrammar);

    IContext aContext;
    ASSERT_TRUE(aContainer.getExpression().getInt().evaluate(aContext) == -64);
    return 0;
  }


  int MinusDouble()
  {
    ArenaAllocator aAlloc;
    Grammar aGrammar;
    aGrammar.registerStandardOperators(aAlloc);
    std::string aTest("-.684");
    Container aContainer(aTest, aGrammar);

    IContext aContext;
    ASSERT_TRUE(aContainer.getExpression().getDouble().evaluate(aContext) == -0.684);
    return 0;
  }

  int IntTest()
  {
    ArenaAllocator aAlloc;
    Grammar aGrammar;
    aGrammar.registerStandardOperators(aAlloc);
    IContext aContext;
    std::string aTest("4");
    Container aContainer(aTest, aGrammar);
    ASSERT_TRUE(aContainer.getExpression().getInt().evaluate(aContext) == 4);
    return 0;
  }

  int SimpleFalseGreaterOrEqualTest()
  {
    ArenaAllocator aAlloc;
    Grammar aGrammar;
    aGrammar.registerStandardOperators(aAlloc);
    IContext aContext;
    std::string aTest("3 >= 5");
    Container aContainer(aTest, aGrammar);
    ASSERT_TRUE(aContainer.getExpression().getBool().evaluate(aContext) == false);
    return 0;
  }


  int SimpleTrueGreaterOrEqualTest()
  {
    ArenaAllocator aAlloc;
    Grammar aGrammar;
    aGrammar.registerStandardOperators(aAlloc);
    IContext aContext;
    std::string aTest("6 >= -5");
    Container aContainer(aTest, aGrammar);
    ASSERT_TRUE(aContainer.getExpression().getBool().evaluate(aContext) == true);
    return 0;
  }

  int SimpleTrueANDCombinationTest()
  {
    ArenaAllocator aAlloc;
    Grammar aGrammar;
    aGrammar.registerStandardOperators(aAlloc);
    IContext aContext;
    std::string aTest("(6 >= 5) AND (4 > 3)");
    FORMULA_DEBUG(aTest);
    Container aContainer(aTest, aGrammar);
    ASSERT_TRUE(aContainer.getExpression().getBool().evaluate(aContext) == true);

    Container aContainer2(aContainer.getExpression().toString(), aGrammar);
    ASSERT_EQ(aContainer.getExpression().getBool().evaluate(aContext),
              aContainer2.getExpression().getBool().evaluate(aContext));
    return 0;
  }

  int SimpleFalseANDCombinationTest()
  {
    ArenaAllocator aAlloc;
    Grammar aGrammar;
    aGrammar.registerStandardOperators(aAlloc);
    IContext aContext;
    std::string aTest("(6 >= 5) AND (3 >= 4)");
    FORMULA_DEBUG(aTest);
    Container aContainer(aTest, aGrammar);
    ASSERT_TRUE(aContainer.getExpression().getBool().evaluate(aContext) == false);
    return 0;
  }

  int FalseORCombinationTest()
  {
    ArenaAllocator aAlloc;
    Grammar aGrammar;
    aGrammar.registerStandardOperators(aAlloc);
    IContext aContext;
    std::string aTest("6 > 5 AND 3 >= 4 OR 3 >= 1");
    FORMULA_DEBUG(aTest);
    Container aContainer(aTest, aGrammar);
    ASSERT_TRUE(aContainer.getExpression().getBool().evaluate(aContext) == true);
    return 0;
  }

  int TrueORCombinationTest()
  {
    ArenaAllocator aAlloc;
    Grammar aGrammar;
    aGrammar.registerStandardOperators(aAlloc);
    IContext aContext;
    std::string aTest("6 < 5 AND 3 <= 4 OR 4 <= 3");
    FORMULA_DEBUG(aTest);
    Container aContainer(aTest, aGrammar);
    ASSERT_TRUE(aContainer.getExpression().getBool().evaluate(aContext) == false);

    Container aContainer2(aContainer.getExpression().toString(), aGrammar);
    ASSERT_EQ(aContainer.getExpression().getBool().evaluate(aContext),
              aContainer2.getExpression().getBool().evaluate(aContext));
    return 0;
  }

  int FalseORANDCombinationTest()
  {
    ArenaAllocator aAlloc;
    Grammar aGrammar;
    aGrammar.registerStandardOperators(aAlloc);
    IContext aContext;
    std::string aTest("(6 >= 5 AND 3 >= 4) OR (5 >= 0)");
    FORMULA_DEBUG(aTest);
    Container aContainer(aTest, aGrammar);
    ASSERT_TRUE(aContainer.getExpression().getBool().evaluate(aContext) == true);

    Container aContainer2(aContainer.getExpression().toString(), aGrammar);
    ASSERT_EQ(aContainer.getExpression().getBool().evaluate(aContext),
              aContainer2.getExpression().getBool().evaluate(aContext));
    return 0;
  }

  int TrueAndORCombinationtest()
  {
    ArenaAllocator aAlloc;
    Grammar aGrammar;
    aGrammar.registerStandardOperators(aAlloc);
    IContext aContext;
    std::string aTest("(((6 >= 5) AND (3 >= 4)) OR (5 >= 4)) AND (5 >= 0)");
    FORMULA_DEBUG(aTest);
    Container aContainer(aTest, aGrammar);
    ASSERT_TRUE(aContainer.getExpression().getBool().evaluate(aContext) == true);

    Container aContainer2(aContainer.getExpression().toString(), aGrammar);
    ASSERT_EQ(aContainer.getExpression().getBool().evaluate(aContext),
              aContainer2.getExpression().getBool().evaluate(aContext));
    return 0;
  }

  int AndNoParenthesisTest()
  {
    ArenaAllocator aAlloc;
    Grammar aGrammar;
    aGrammar.registerStandardOperators(aAlloc);
    IContext aContext;
    std::string aTest("6 < 6 AND 9 >= 6");
    FORMULA_DEBUG(aTest);
    Parser aParser(aAlloc, aGrammar);
    aParser.parse(aTest);
    ASSERT_FALSE(aParser.getTopExpression().getBool().evaluate(aContext));

    Expression& anExpr = aParser.getTopExpression();
    aParser.parse(anExpr.toString());
    ASSERT_EQ(anExpr.getBool().evaluate(aContext),
              aParser.getTopExpression().getBool().evaluate(aContext));
    return 0;
  }

  int OrNoParenthesisTest()
  {
    ArenaAllocator aAlloc;
    Grammar aGrammar;
    aGrammar.registerStandardOperators(aAlloc);
    IContext aContext;
    std::string aTest("3 >= 5 OR 3 >= 3");
    FORMULA_DEBUG(aTest);
    Container aContainer(aTest, aGrammar);
    ASSERT_TRUE(aContainer.getExpression().getBool().evaluate(aContext) == true);
    return 0;
  }


  int OrNotOperatorTest()
  {
    ArenaAllocator aAlloc;
    Grammar aGrammar;
    aGrammar.registerStandardOperators(aAlloc);
    IContext aContext;
    std::string aTest("(3 >= 5 AND 3 == 1) OR (3 != 2)");
    FORMULA_DEBUG(aTest);
    Parser aParser(aAlloc, aGrammar, aTest);
    ASSERT_TRUE(aParser.getTopExpression().getBool().evaluate(aContext) == true);

    Container aContainer2(aParser.getTopExpression().toString(), aGrammar);
    ASSERT_EQ(aParser.getTopExpression().getBool().evaluate(aContext),
              aContainer2.getExpression().getBool().evaluate(aContext));
    return 0;
  }

  int NotOperatorTest()
  {
    ArenaAllocator aAlloc;
    Grammar aGrammar;
    aGrammar.registerStandardOperators(aAlloc);
    IContext aContext;
    std::string aTest("!(1>=1)");
    FORMULA_DEBUG(aTest);
    Container aContainer(aTest, aGrammar);
    ASSERT_TRUE(aContainer.getExpression().getBool().evaluate(aContext) == false);
    return 0;
  }

  int Choice1()
  {
    ArenaAllocator aAlloc;
    Grammar aGrammar;
    aGrammar.registerStandardOperators(aAlloc);
    IContext aContext;
    std::string aTest("-6 > 5 ? 'Wrong' : 'Right' ");
    FORMULA_DEBUG(aTest);
    Container aContainer(aTest, aGrammar);
    ASSERT_TRUE(aContainer.getExpression().getString().evaluate(aContext) == "Right");
    return 0;
  }

  int Choice2()
  {
    ArenaAllocator aAlloc;
    Grammar aGrammar;
    aGrammar.registerStandardOperators(aAlloc);
    IContext aContext;
    std::string aTest("!(6 >= 6) ? 2 > 1 : 2 < 1");
    FORMULA_DEBUG(aTest);
    Container aContainer(aTest, aGrammar);
    ASSERT_TRUE(aContainer.getExpression().getBool().evaluate(aContext) == false);

    Container aContainer2(aContainer.getExpression().toString(), aGrammar);
    ASSERT_EQ(aContainer.getExpression().getBool().evaluate(aContext),
              aContainer2.getExpression().getBool().evaluate(aContext));
    return 0;
  }

  int StringAt()
  {
    ArenaAllocator aAlloc;
    Grammar aGrammar;
    aGrammar.registerStandardOperators(aAlloc);
    IContext aContext;
    std::string aTest("'Pouet'[2] == 'u'[0] && 'Pouet'[1] != 'u'[0] ");
    FORMULA_DEBUG(aTest);
    Container aContainer(aTest, aGrammar);
    ASSERT_TRUE(aContainer.getExpression().getBool().evaluate(aContext));
    return 0;
  }

  int CastInt()
  {
    ArenaAllocator aAlloc;
    Grammar aGrammar;
    aGrammar.registerStandardOperators(aAlloc);
    IContext aContext;
    std::string aTest("(int)65.89");
    FORMULA_DEBUG(aTest);
    Container aContainer(aTest, aGrammar);
    ASSERT_TRUE(aContainer.getExpression().getInt().evaluate(aContext) == 66);

    Container aContainer2(aContainer.getExpression().toString(), aGrammar);
    ASSERT_EQ(aContainer.getExpression().getInt().evaluate(aContext),
              aContainer2.getExpression().getInt().evaluate(aContext));
    return 0;
  }

  int CastStringInt()
  {
    ArenaAllocator aAlloc;
    Grammar aGrammar;
    aGrammar.registerStandardOperators(aAlloc);
    IContext aContext;
    std::string aTest("(int)'-23'");
    FORMULA_DEBUG(aTest);
    Container aContainer(aTest, aGrammar);
    ASSERT_TRUE(aContainer.getExpression().getInt().evaluate(aContext) == -23);

    Container aContainer2(aContainer.getExpression().toString(), aGrammar);
    ASSERT_EQ(aContainer.getExpression().getInt().evaluate(aContext),
              aContainer2.getExpression().getInt().evaluate(aContext));
    return 0;
  }

  int CastStringDouble()
  {
    ArenaAllocator aAlloc;
    Grammar aGrammar;
    aGrammar.registerStandardOperators(aAlloc);
    IContext aContext;
    std::string aTest("(double)'-23.'");
    FORMULA_DEBUG(aTest);
    Container aContainer(aTest, aGrammar);
    ASSERT_TRUE(aContainer.getExpression().getDouble().evaluate(aContext) == -23.);

    Container aContainer2(aContainer.getExpression().toString(), aGrammar);
    ASSERT_EQ(aContainer.getExpression().getDouble().evaluate(aContext),
              aContainer2.getExpression().getDouble().evaluate(aContext));
    return 0;
  }

  int CastString()
  {
    ArenaAllocator aAlloc;
    Grammar aGrammar;
    aGrammar.registerStandardOperators(aAlloc);
    IContext aContext;
    std::string aTest("(string)87");
    FORMULA_DEBUG(aTest);
    Container aContainer(aTest, aGrammar);
    ASSERT_TRUE(aContainer.getExpression().getString().evaluate(aContext) == "87");
    return 0;
  }

  int CastDouble()
  {
    ArenaAllocator aAlloc;
    Grammar aGrammar;
    aGrammar.registerStandardOperators(aAlloc);
    IContext aContext;
    std::string aTest("(double)87");
    FORMULA_DEBUG(aTest);
    Container aContainer(aTest, aGrammar);
    ASSERT_TRUE(aContainer.getExpression().getDouble().evaluate(aContext) == (double)87);
    return 0;
  }

  template <class ContextT>
    class Self: public TypedExpression<ContextT>
  {
  public:
    Self(const Grammar& iGrammar):
      TypedExpression<ContextT>(iGrammar)
    {}

    std::string toString() const
    {
      return "$Context";
    }

    const ContextT& evaluate(IContext& ioContext) const
    {
      return ioContext.get<ContextT>();
    }
  };

  class MyContext: public mdw::formula::IContext
  {
    double _test;
    std::string _testString;
    bool _testBool;
  public:
    MyContext(double iDouble, const std::string& iString, bool iBool):
      _test(iDouble), _testString(iString), _testBool(iBool)
    {}

    double getDouble() const
    {
      return _test;
    }

    const std::string& getString() const
    {
      return _testString;
    }

    bool getBool() const
    {
      return _testBool;
    }
  };

  class MyContextFacts: public formula::FactInstantiator
  {
    const Grammar& _grammar;
    const Self<MyContext> _contextAsExpression;
  public:
    MyContextFacts(Grammar& iGrammar):
      _grammar(iGrammar), _contextAsExpression(iGrammar)
    {}

    Expression& instantiate(ArenaAllocator& ioAllocator,
                            const Grammar& iGrammar,
                            const std::string& iName) const
    {
      if (iName == "Test")
      {
        return make_resolver(ioAllocator, _grammar, _contextAsExpression,
                             boost::mem_fn(&MyContext::getDouble), iName);
      } else if (iName == "TestString") {
        return make_resolver(ioAllocator, _grammar, _contextAsExpression,
                             boost::mem_fn(&MyContext::getString), iName);
      } else if (iName == "TestBool") {
        return make_resolver(ioAllocator, _grammar, _contextAsExpression,
                             boost::mem_fn(&MyContext::getBool), iName);
      } else {
        throw mdw::UnknownException("Unknown variable name: " + iName);
      }
    }

  };

  int MultipleMetricTest()
  {
    Grammar aGrammar;
    Factorizer aFactorizer;
    ArenaAllocator& aAlloc(aFactorizer.getAllocator());

    aGrammar.addObserver(aFactorizer);
    aGrammar.registerType<MyContext>();
    aGrammar.registerStandardOperators(aAlloc);
    MyContext aContext(5, "ThisIsATest", false);
    MyContext aContext2(20., "Haha", true);
    MyContextFacts aFacts(aGrammar);
    aGrammar.registerFactResolver("Test", kExprDouble, aFacts);
    aGrammar.registerFactResolver("TestString", kExprString, aFacts);
    aGrammar.registerFactResolver("TestBool", kExprBool, aFacts);

    std::string aTest("$TestString == \"ThisIsATest\"");
    FORMULA_DEBUG(aTest);
    Container aFormula(aTest, aGrammar);
    ASSERT_TRUE(aFormula.getExpression().getBool().evaluate(aContext));
    ASSERT_FALSE(aFormula.getExpression().getBool().evaluate(aContext2));

    std::string aTest2("$TestString == \"Haha\"");
    FORMULA_DEBUG(aTest2);
    Container aFormula2(aTest2, aGrammar);
    ASSERT_FALSE(aFormula2.getExpression().getBool().evaluate(aContext));
    ASSERT_TRUE(aFormula2.getExpression().getBool().evaluate(aContext2));

    std::string aTest3("(($TestString == \"Haha\") AND ($Test > (double)2)) OR $Test == 5.");
    FORMULA_DEBUG(aTest3);
    Container aFormula3(aTest3, aGrammar);
    ASSERT_TRUE(aFormula3.getExpression().getBool().evaluate(aContext));

    std::string aTest4("(($TestString == \"Haha\") AND ((int)$Test > 2))");
    FORMULA_DEBUG(aTest4);
    Container aFormula4(aTest4, aGrammar);
    ASSERT_FALSE(aFormula4.getExpression().getBool().evaluate(aContext));

    std::string aTest6("(($TestString == \"Haha\") OR ((int)$Test > 2)) == true");
    FORMULA_DEBUG(aTest6);
    Container aFormula6(aTest6, aGrammar);
    ASSERT_TRUE(aFormula6.getExpression().getBool().evaluate(aContext));

    std::string aTest5("(($TestString == \'Haha\') AND ($Test > 2.)) == $TestBool");
    FORMULA_DEBUG(aTest5);
    Container aFormula5(aTest5, aGrammar);
    ASSERT_TRUE(aFormula5.getExpression().getBool().evaluate(aContext));

    std::string aTest7("$Test * $Test");
    FORMULA_DEBUG(aTest7);
    Container aFormula7(aTest7, aGrammar);
    ASSERT_TRUE(aFormula7.getExpression().getDouble().evaluate(aContext) == 5.*5.);
    ASSERT_TRUE(aFormula7.getExpression().getDouble().evaluate(aContext2) == 20.*20.);

    std::string aTest8("((($Test * $Test) + 2. - (double)2) * 2.) / $Test");
    FORMULA_DEBUG(aTest8);
    Container aFormula8(aTest8, aGrammar);
    ASSERT_TRUE(aFormula8.getExpression().getDouble().evaluate(aContext) == 10);

    std::string aTest9("(((($Test * $Test) + 2. - 2.) * 2.) / $Test) == $Test * 2.");
    FORMULA_DEBUG(aTest9);
    Container aFormula9(aTest9, aGrammar);
    ASSERT_TRUE(aFormula9.getExpression().getBool().evaluate(aContext));

    std::string aTest10("\"2015-02-10\" >= \"2014-02-10\" ");
    FORMULA_DEBUG(aTest10);
    Container aFormula10(aTest10, aGrammar);
    ASSERT_TRUE(aFormula10.getExpression().getBool().evaluate(aContext));

    /*UP_Date aDate(11,2,2015);
    std::string aTest11("\"2015-02-10\" + 1");
    FORMULA_DEBUG(aTest11);
    Container aFormula11(aTest11);
    FORMULA_DEBUG("Resulting date" << aFormula11.evaluate(aMetrics).getDateValue().asString());
    ASSERT_TRUE(aFormula11.evaluate(aMetrics).getDateValue() == aDate);*/

    return 0;
  }

  class Flight
  {
    std::string _departureCountry;
    double _expectedLoadFactor;
    std::string _cabin;
  public:
    Flight(const std::string& iCountry,
           double iElf,
           const std::string& iCabin):
      _departureCountry(iCountry), _expectedLoadFactor(iElf), _cabin(iCabin)
    {}

    const std::string& getDepartureCountry() const
    {
      return _departureCountry;
    }

    double getExpectedLoadFactor() const
    {
      return _expectedLoadFactor;
    }

    bool hasExpectedLoadFactor() const
    {
      return _expectedLoadFactor != -1;
    }

    const std::string& getCabin() const
    {
      return _cabin;
    }
  };

  class FlightContext: public IContext
  {
  public:
    FlightContext(const Flight& iCurrent, const Flight& iNext):
      _currentFlight(iCurrent), _nextFlight(iNext)
    {}

    Flight _currentFlight;
    Flight _nextFlight;
  };

  class FlightValue: public TypedExpression<Flight>
  {
    std::string _name;
  public:
    FlightValue(const Grammar& ioGrammar, const std::string iName):
      TypedExpression<Flight>(ioGrammar),
      _name(iName)
    {}

    std::string toString() const
    {
      return "$" + _name;
    }

    const Flight& evaluate(IContext& ioContext) const
    {
      FlightContext& aContext = ioContext.get<FlightContext>();
      return aContext._currentFlight;
    }
  };

  class NextFlightValue: public TypedExpression<Flight>
  {
    std::string _name;
  public:
    NextFlightValue(const Grammar& ioGrammar, const std::string iName):
      TypedExpression<Flight>(ioGrammar),
      _name(iName)
    {}

    std::string toString() const
    {
      return "$" + _name;
    }

    const Flight& evaluate(IContext& ioContext) const
    {
      FlightContext& aContext = ioContext.get<FlightContext>();
      return aContext._nextFlight;
    }
  };

  class FlightFacts: public formula::FactInstantiator
  {
    Grammar& _grammar;
  public:
    FlightFacts(Grammar& iGrammar):
      _grammar(iGrammar)
    {
      iGrammar.registerType<Flight>();
    }

    Expression& instantiate(ArenaAllocator& ioAllocator, 
                            const Grammar& iGrammar,
                            const std::string& iName) const
    {
      if (iName == "CurrentFlight")
      {
        return ioAllocator.create<FlightValue>(_grammar, iName);
      } else if (iName == "NextFlight") {
        return ioAllocator.create<NextFlightValue>(_grammar, iName);
      } else {
        throw mdw::UnknownException("Unknown variable name: " + iName);
      }
    }

  };

  class Service
  {
    std::string _code;
    std::string _rfic;

  public:
    Service(const std::string& iCode):
      _code(iCode)
    {}

    Service(const std::string& iCode, const std::string& iRFIC):
      _code(iCode), _rfic(iRFIC)
    {}

    const std::string& getCode() const
    {
      return _code;
    }

    const std::string& getRFIC() const
    {
      return _rfic;
    }

    bool operator==(const Service& iOther) const
    {
      return _code == iOther._code;
    }
  };

  class Customer
  {
    int _value;
    char _gender;
    std::string _name;
    std::vector<Service> _services;
    Service _preferredSvc;
  public:
    Customer(const std::string& iName, int iValue, char iGender):
      _value(iValue), _gender(iGender), _name(iName), _preferredSvc("VGML")
    {}

    void addService(const std::string& iCode,
                    const std::string& iRFIC)
    {
      _services.push_back(Service(iCode, iRFIC));
    }

    int getValue() const {return _value;}
    char getGender() const {return _gender;}
    const std::string& getName() const {return _name;}
    const std::vector<Service>& getServices() const {return _services;}
    const Service& getPreferredService() const {return _preferredSvc;}
  };

  class FactContext: public IContext
  {
  public:
    FactContext(const Flight& iFlight, const Customer& iCustomer):
      _flight(iFlight), _customer(iCustomer)
    {}

    Flight _flight;
    Customer _customer;
    const Flight& getFlight() const {return _flight;}
    const Customer& getCustomer() const {return _customer;}
  };

  int FactTest(){
    ArenaAllocator aAlloc;
    Grammar aGrammar;
    aGrammar.registerStandardOperators(aAlloc);

    Flight aFlight("FR", 0.46, "Y");
    Customer aCustomer("Bob", 350, 'M');
    aCustomer.addService("VGML", "M");
    aCustomer.addService("WIFI", "T");
    aCustomer.addService("LNGE", "T");
    aCustomer.addService("PETC", "S");
    
    Iterable<Service, std::vector<Service> >::RegisterMe(aAlloc, aGrammar);
    RandomAccess<Service, std::vector<Service> >::RegisterMe(aAlloc, aGrammar);

    FactContext aContext(aFlight, aCustomer);

    OwnContext<FactContext>::RegisterFact(aAlloc, aGrammar, "Flight",
                                          boost::mem_fn(&FactContext::getFlight));
    OwnContext<FactContext>::RegisterFact(aAlloc, aGrammar, "Customer",
                                          boost::mem_fn(&FactContext::getCustomer));

    RegisterAttribute(aAlloc, aGrammar, boost::mem_fn(&Flight::getDepartureCountry),
                                       "DepartureCountry");
    RegisterOptionalAttribute(aAlloc, aGrammar,
                              boost::mem_fn(&Flight::getExpectedLoadFactor),
                              boost::mem_fn(&Flight::hasExpectedLoadFactor),
                              "ExpectedLoadFactor");
    RegisterAttribute(aAlloc, aGrammar, boost::mem_fn(&Flight::getCabin), "Cabin");

    RegisterAttribute(aAlloc, aGrammar, boost::mem_fn(&Customer::getName), "Name");
    RegisterAttribute(aAlloc, aGrammar, boost::mem_fn(&Customer::getGender), "Gender");
    RegisterAttribute(aAlloc, aGrammar, boost::mem_fn(&Customer::getValue), "Value");
    RegisterAttribute(aAlloc, aGrammar, boost::mem_fn(&Customer::getServices), "Services");
    RegisterAttribute(aAlloc, aGrammar, boost::mem_fn(&Customer::getPreferredService),
                                       "Preference");

    DefaultCast<std::string, Service>::RegisterMe(aAlloc, aGrammar, "(Service)");

    RegisterAttribute(aAlloc, aGrammar, boost::mem_fn(&Service::getCode), "code");

    std::string aTest6("$Customer.Preference.code");
    FORMULA_DEBUG(aTest6);
    Container aFormula6(aTest6, aGrammar);

    ASSERT_EQ(aFormula6.getExpression().getString().evaluate(aContext), "VGML");

    std::string aTest3("$Customer.Services[2].code == 'LNGE'");
    FORMULA_DEBUG(aTest3);
    Container aFormula3(aTest3, aGrammar);

    ASSERT_TRUE(aFormula3.getExpression().getBool().evaluate(aContext));

    Container aContainer3(aFormula3.getExpression().toString(), aGrammar);
    ASSERT_EQ(aFormula3.getExpression().getBool().evaluate(aContext),
              aContainer3.getExpression().getBool().evaluate(aContext));

    std::string aTest4("$Customer.Services[2] in $Customer.Services && "
                       "(Service)'PETC' in $Customer.Services && "
                       "!((Service)'PETB' in $Customer.Services)");
    FORMULA_DEBUG(aTest4);
    Container aFormula4(aTest4, aGrammar);

    ASSERT_TRUE(aFormula4.getExpression().getBool().evaluate(aContext));

    Container aContainer4(aFormula4.getExpression().toString(), aGrammar);
    ASSERT_EQ(aFormula4.getExpression().getBool().evaluate(aContext),
              aContainer4.getExpression().getBool().evaluate(aContext));
    return 0;
  }


  int BaseFactTest(){
    ArenaAllocator aAlloc;
    Grammar aGrammar;
    aGrammar.registerStandardOperators(aAlloc);

    Flight aFlight("FR", 0.46, "Y");
    Customer aCustomer("Bob", 350, 'M');
    aCustomer.addService("VGML", "M");
    aCustomer.addService("WIFI", "T");
    aCustomer.addService("LNGE", "T");
    aCustomer.addService("PETC", "S");
    
    Iterable<Service, std::vector<Service> >::RegisterMe(aAlloc, aGrammar);
    RandomAccess<Service, std::vector<Service> >::RegisterMe(aAlloc, aGrammar);

    Iterable<char, std::string>::RegisterMe(aAlloc, aGrammar);
    RandomAccess<char, std::string>::RegisterMe(aAlloc, aGrammar);

    RegisterAttribute(aAlloc, aGrammar, boost::mem_fn(&Flight::getDepartureCountry),
                                       "DepartureCountry");
    RegisterOptionalAttribute(aAlloc, aGrammar,
                              boost::mem_fn(&Flight::getExpectedLoadFactor),
                              boost::mem_fn(&Flight::hasExpectedLoadFactor),
                              "ExpectedLoadFactor");
    RegisterAttribute(aAlloc, aGrammar, boost::mem_fn(&Flight::getCabin), "Cabin");

    RegisterAttribute(aAlloc, aGrammar, boost::mem_fn(&Customer::getName), "Name");
    RegisterAttribute(aAlloc, aGrammar, boost::mem_fn(&Customer::getGender), "Gender");
    RegisterAttribute(aAlloc, aGrammar, boost::mem_fn(&Customer::getValue), "Value");
    RegisterAttribute(aAlloc, aGrammar, boost::mem_fn(&Customer::getServices), "Services");
    RegisterAttribute(aAlloc, aGrammar, boost::mem_fn(&Customer::getPreferredService),
                                       "Preference");

    DefaultCast<std::string, Service>::RegisterMe(aAlloc, aGrammar, "(Service)");

    RegisterAttribute(aAlloc, aGrammar, boost::mem_fn(&Service::getCode), "code");
    RegisterAttribute(aAlloc, aGrammar, boost::mem_fn(&Service::getRFIC), "rfic");

    Fact<Flight>::RegisterMe(aAlloc, aGrammar, "Flight");
    Fact<Customer>::RegisterMe(aAlloc, aGrammar, "Customer");

    {
      IContext aContext;
      aContext.setFact(aFlight, "Flight");
      aContext.setFact(aCustomer, "Customer");

      std::string aTest6("$Customer.Preference.code");
      FORMULA_DEBUG(aTest6);
      Container aFormula6(aTest6, aGrammar);

      ASSERT_EQ(aFormula6.getExpression().getString().evaluate(aContext), "VGML");

      std::string aTest3("$Customer.Services[2].code == 'LNGE'");
      FORMULA_DEBUG(aTest3);
      Container aFormula3(aTest3, aGrammar);

      ASSERT_TRUE(aFormula3.getExpression().getBool().evaluate(aContext));

      Container aContainer3(aFormula3.getExpression().toString(), aGrammar);
      ASSERT_EQ(aFormula3.getExpression().getBool().evaluate(aContext),
                aContainer3.getExpression().getBool().evaluate(aContext));

      std::string aTest4("$Customer.Services[2] in $Customer.Services && "
                         "(Service)'PETC' in $Customer.Services && "
                         "!((Service)'PETB' in $Customer.Services)");
      FORMULA_DEBUG(aTest4);
      Container aFormula4(aTest4, aGrammar);

      ASSERT_TRUE(aFormula4.getExpression().getBool().evaluate(aContext));

      Container aContainer4(aFormula4.getExpression().toString(), aGrammar);
      ASSERT_EQ(aFormula4.getExpression().getBool().evaluate(aContext),
                aContainer4.getExpression().getBool().evaluate(aContext));

      std::string aTest5("($Customer.Services -> Svc ? $Svc.code == 'WIFI').count");
      FORMULA_DEBUG(aTest5);
      Container aFormula5(aTest5, aGrammar);

      ASSERT_EQ(aFormula5.getExpression().getInt().evaluate(aContext), 1);

      Container aContainer5(aFormula5.getExpression().toString(), aGrammar);
      ASSERT_EQ(aFormula5.getExpression().getInt().evaluate(aContext),
                aContainer5.getExpression().getInt().evaluate(aContext));

      std::string aTest7("!($Customer.Services -> Svc ? $Svc.code == 'WIFI').empty &&"
                         "($Customer.Services -> Svc ? $Svc.code == 'SCHTROUMPF').empty");
      FORMULA_DEBUG(aTest7);
      Container aFormula7(aTest7, aGrammar);

      ASSERT_TRUE(aFormula7.getExpression().getBool().evaluate(aContext));

      Container aContainer7(aFormula7.getExpression().toString(), aGrammar);
      ASSERT_EQ(aFormula7.getExpression().getBool().evaluate(aContext),
                aContainer7.getExpression().getBool().evaluate(aContext));

      std::string aTest8("($Customer.Services -> Svc ? $Svc.code == 'WIFI').count == 1 &&"
                         "($Customer.Services -> Svc ? $Svc.code == 'SCHTROUMPF').count == 0");
      FORMULA_DEBUG(aTest8);
      Container aFormula8(aTest8, aGrammar);

      ASSERT_TRUE(aFormula8.getExpression().getBool().evaluate(aContext));

      Container aContainer8(aFormula8.getExpression().toString(), aGrammar);
      ASSERT_EQ(aFormula8.getExpression().getBool().evaluate(aContext),
                aContainer8.getExpression().getBool().evaluate(aContext));

      std::string aTest9("(($Customer.Services -> Svc ? $Svc.code == 'VGML') -> v2 ? "
                         "!$v2.rfic.empty && ($v2.rfic[0] == $Customer.Gender)).count == 1");
      FORMULA_DEBUG(aTest9);
      Container aFormula9(aTest9, aGrammar);

      ASSERT_TRUE(aFormula9.getExpression().getBool().evaluate(aContext));

      Container aContainer9(aFormula9.getExpression().toString(), aGrammar);
      ASSERT_EQ(aFormula9.getExpression().getBool().evaluate(aContext),
                aContainer9.getExpression().getBool().evaluate(aContext));

    }
    return 0;
  }
  
  // PTR#12761056 : Make sure that the OR logical operator ignores the fact that
  // one of its operands is NaN
  int LogicalOrBoolOperatorTest(){
    ArenaAllocator aAlloc;
    Grammar aGrammar;
    aGrammar.registerStandardOperators(aAlloc);

    Flight aFlight("FR", -1, "Y");
    
    RegisterOptionalAttribute(aAlloc, aGrammar,
                              boost::mem_fn(&Flight::getExpectedLoadFactor),
                              boost::mem_fn(&Flight::hasExpectedLoadFactor),
                              "ExpectedLoadFactor");
    RegisterAttribute(aAlloc, aGrammar, boost::mem_fn(&Flight::getCabin), "Cabin");


    Fact<Flight>::RegisterMe(aAlloc, aGrammar, "Flight");

    {
      IContext aContext;
      aContext.setFact(aFlight, "Flight");

      std::string aTest0("$Flight.ExpectedLoadFactor <= 0.5 || $Flight.Cabin == \"Y\"");
      FORMULA_DEBUG(aTest0);
      Container aFormula0(aTest0, aGrammar);
      
      ASSERT_TRUE(aFormula0.getExpression().getBool().evaluate(aContext));
      ASSERT_FALSE(aContext.isNaN());

      std::string aTest1("$Flight.Cabin == \"Y\" || $Flight.ExpectedLoadFactor <= 0.5");
      FORMULA_DEBUG(aTest1);
      Container aFormula1(aTest1, aGrammar);
      
      ASSERT_TRUE(aFormula1.getExpression().getBool().evaluate(aContext));
      ASSERT_FALSE(aContext.isNaN());

      std::string aTest2("$Flight.ExpectedLoadFactor <= 0.5 || $Flight.Cabin != \"Y\"");
      FORMULA_DEBUG(aTest2);
      Container aFormula2(aTest2, aGrammar);
      
      ASSERT_FALSE(aFormula2.getExpression().getBool().evaluate(aContext));
      ASSERT_FALSE(aContext.isNaN());

      std::string aTest3("$Flight.ExpectedLoadFactor <= 0.5 || $Flight.ExpectedLoadFactor > 2.0");
      FORMULA_DEBUG(aTest3);
      Container aFormula3(aTest3, aGrammar);
      
      aFormula3.getExpression().getBool().evaluate(aContext);
      ASSERT_TRUE(aContext.isNaN());
    }
    return 0;
  }

  int AllParserTests() {
    int aResult = 0;
    aResult += ConstantBool();
    aResult += ConstantInt();
    aResult += ConstantDouble();
    aResult += ConstantString();
    aResult += NotBool();
    aResult += MinusInt();
    aResult += MinusDouble();
    aResult += IntTest();
    aResult += SimpleFalseGreaterOrEqualTest();
    aResult += SimpleTrueGreaterOrEqualTest();
    aResult += SimpleTrueANDCombinationTest();
    aResult += SimpleFalseANDCombinationTest();
    aResult += FalseORCombinationTest();
    aResult += TrueORCombinationTest();
    aResult += FalseORANDCombinationTest();
    aResult += TrueAndORCombinationtest();
    aResult += AndNoParenthesisTest();
    aResult += OrNoParenthesisTest();
    aResult += OrNotOperatorTest();
    aResult += NotOperatorTest();
    aResult += Choice1();
    aResult += Choice2();
    aResult += StringAt();
    aResult += CastInt();
    aResult += CastStringInt();
    aResult += CastStringDouble();
    aResult += CastString();
    aResult += CastDouble();
    aResult += MultipleMetricTest();
    aResult += FactTest();
    aResult += BaseFactTest();
    aResult += LogicalOrBoolOperatorTest();
    return aResult;
  }

}}

