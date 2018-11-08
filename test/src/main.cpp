#include <iostream>
#include <mdw/UnknownException.hpp>

namespace mdw { namespace formula {
  int AllParserTests();
  int AllFormulaTests();
}}

int main(int argc, char **argv)
{
  int aResult = 0;
  try {
    aResult += mdw::formula::AllParserTests();
    aResult += mdw::formula::AllFormulaTests();
  } catch (const mdw::UnknownException& iEx) {
    std::cerr << "Received an exception: " << iEx.message() << std::endl;
  }
  if (aResult == 0) {
    std::cout << "ALL TESTS PASSED" << std::endl;
  } else {
    std::cout << "SOME TESTS FAILED" << std::endl;
  }
  return aResult;
}
