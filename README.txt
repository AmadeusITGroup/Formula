This software was initially written by Amadeus as a solution to provide an
efficient and generic C++ rules engine for various projects. It can also be
used as a generic calculator.
Genericity in this software stems from the fact that it can work on any
type(s). The C++ types, their attributes and methods can be used directly in
the dynamic expressions, without any costly conversion into a library-defined
representation.

The main driver for this component is to:
* define a domain language that (typically) lasts for the duration of the process, 
* parse rules/expressions that change on a rare basis (such as business rules...),
* compute those rules on new objects a lot of times (coming from transactional
  messages, rows in a database...)


_________________
Main classes
_________________

The **Expression** class is the generic representation of any
formula/rule/condition... and is the central key to dynamic evaluations.
Expressions are typed with their return type (in the same fashion as C/C++
expressions), and there is no automatic conversion done between expression
types. However, the **ExpressionCast** provides conversions between types of
expressions.

The core of this library is the **Grammar** class, which represents the domain
language that will be expressed in rules or formulæ. It is in particular
possible to register types (basic types, classes, enums...), their attributes (in a
recursive fashion), and the operators that can operate on them. Note that
there is no notion of registered method, but it is possible to use unary, binary, ternary
operators instead.
Note that the Grammar does not need to be a Singleton/global variable, even
though it might be convenient if a single domain needs to be supported in a
process. It is perfectly possible to handle several different grammars in the
same process.
If you want to register your own operators, you will have to provide
**Instantiators** to the Grammar, in order to let the engine instantiate the
correct type of Expression able to perform this operator.

The **StandardTypes** class can register basic types (ints, doubles, strings)
automatically, as well as some operators. It is a conveniency class though and does
NOT have to be used.

The **Facts** class is used to define and list the domain objects that can be
used in the expressions. They can have any type and need to be registered as
facts in the Grammar.

The **Parser** class can interpret strings as expressions, using the facts,
operators and types registered in the Grammar.

The **IContext** class is used to pass the actual objects as facts to the
engine. It also contains a "session" information, so that many calls to
evaluate expressions can share the results cache. This is especially usefule
when some facts are shared between several evaluations of expressions (such as
when going through a tree and evaluating expressions on all branches).
A new IContext needs to be created to reset all cached results.

Finally, the **Factorizer** is able to detect similar parts in one or many
expressions. It is especially useful when similar
blocks/conditions/sub-expressions are used in several expressions.
Its output is to insert CachedExpression's in the expressions to compute, and
share the instances everywhere they're used. This results in performance
gains, since the common parts are evaluated only once per (cachable) fact they depend on.

Please refer to the unit tests in the test folder to have more complete
information on the way to use the different classes.


_________________
Compilation
_________________

Nothing very fancy here, the library is composed of an include folder and src
for sources. The grammar files (flex + bison) are in the grammar folder.

The main Makefile will generate the corresponding sources, and generate the
library.

The Makefile in the test folder will compile the unit tests.

A Doxyfile is provided to generate the documentation and navigate the
hierarchy of classes.


_________________
Limitations
_________________

As mentioned above, methods are not currently supported by the grammar and
parser. Unary and ternary operators can be used instead to combine several
objects (or attributes, for methods without any parameter).

Integral types are all represented using int64_t. There is no automatic
conversion between expressions of integral types (but they all inherit from
BaseExpression<int>).

In the same fashion, floats and doubles are represented as doubles in
expressions.

There is currently no optimization of expressions depending on several facts (in the fashion of
beta-nodes in the Rete algorithms).

The CachedExpression uses the *address* of a fact (except for integral/float
types, for which the value is used) to determine whether the fact is the same
as in the cache.
This may result in wrong results in case a fact object is modified between several
calls to the engine (or in weird cases where objects are created in the same
memory spot as a previously deleted object).

