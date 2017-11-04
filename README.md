# milpcpp
## An AMPL-like C++ interface to glpk and lp_solve

Both glpk and lp_solve currently lack a C++ interface. MILPCPP aims to fill this gap. 

The approach used here is very different from the one used in the Gurobi or CPLEX C++ interfaces. The emphasis here is modeling rather than providing a C++ wrapper to the solver interface. From a language point of view, another difference is the use of template metaprogramming and lambdas.

The design goal is to allow a model written in C++ to resemble as much as possible the equivalent AMPL model.

In the examples folder of the repository you will find the C++ equivalents of the models from chapters 1 to 3 of "AMPL: A Modeling Language for Mathematical Programming" (http://ampl.com/resources/the-ampl-book/). The goal is to eventually translate all the examples in the book into C++.

The repository does not include the glpk and lp_solve headers and libraries needed to compile milpcpp and the examples but the cmake files will tell you where to put them.

This is a work in progress. Some non-breaking re-design is planned (use of variadic templates, function arguments need rvalue reference versions, etc..)

Don't hesitate give comments, suggestions or ask for help compiling and running.


Similar in spirit but pre-C++11: http://projects.coin-or.org/FlopC%2B%2B/browser/releases/1.1.2/FlopCpp/test/unitTest.cpp

This one uses more or less the same approach as the Gurobi and CPLEX C++ interfaces: http://projects.coin-or.org/Rehearse
