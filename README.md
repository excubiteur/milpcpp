# milpcpp
## An AMPL-like C++ interface to glpk and lp_solve

Both glpk and lp_solve currently lack a C++ interface. MILPCPP aims to fill this gap. 

The approach used here is very different from the one used in the Gurobi or CPLEX C++ interfaces. The emphasis here is modeling rather than providing a C++ wrapper to the solver interface. From a language point of view, another difference is the use of template metaprogramming and lambdas.

The design goal is to allow a model written in C++ to resemble as much as possible the equivalent AMPL model.

For example, the AMPL model (from the book mentioned below)

```

set PROD;     # products
param T > 0;  # number of weeks

param rate {PROD} > 0;          # tons per hour produced
param inv0 {PROD} >= 0;         # initial inventory
param avail {1..T} >= 0;        # hours available in week
param market {PROD,1..T} >= 0;  # limit on tons sold in week

param prodcost {PROD} >= 0;     # cost per ton produced
param invcost {PROD} >= 0;      # carrying cost/ton of inventory
param revenue {PROD,1..T} >= 0; # revenue per ton sold

var Make {PROD,1..T} >= 0;      # tons produced
var Inv {PROD,0..T} >= 0;       # tons inventoried
var Sell {p in PROD, t in 1..T} >= 0, <= market[p,t]; # tons sold

maximize Total_Profit:
sum {p in PROD, t in 1..T} (revenue[p,t]*Sell[p,t] -
prodcost[p]*Make[p,t] - invcost[p]*Inv[p,t]);

# Total revenue less costs in all weeks

subject to Time {t in 1..T}:
sum {p in PROD} (1/rate[p]) * Make[p,t] <= avail[t];

# Total of hours used by all products
# may not exceed hours available, in each week

subject to Init_Inv {p in PROD}:  Inv[p,0] = inv0[p];

# Initial inventory must equal given value

subject to Balance {p in PROD, t in 1..T}:
Make[p,t] + Inv[p,t-1] = Sell[p,t] + Inv[p,t];

# Tons produced and taken from inventory
# must equal tons sold and put into inventory

```
would look like this in C++:

```

	using namespace milpcpp;

	model m;

	MILPCPP_SET(PROD);
	MILPCPP_TYPED_PARAM(T);

	param<PROD>              rate(greater_than(0));
	param<PROD>              inv0(greater_equal(0));
	param<range<1, T>>       avail(greater_equal(0));
	param<PROD,range<1, T>>  market (greater_equal(0));

	param<PROD>               prodcost(greater_equal(0));
	param<PROD>               invcost(greater_equal(0));
	param<PROD, range<1, T>>  revenue(greater_equal(0));

　
	var<PROD, range<1, T>>    Make(greater_equal(0));
	var<PROD, range<0, T>>    Inv(greater_equal(0));
	var<PROD, range<1, T>>    Sell(greater_equal(0), 
              less_equal([&](PROD p, range<1, T> t) { return market(p, t); }));

  // code to load data here
  
	maximize("Total_Profit",

		sum([&](PROD p, range<1, T> t) { return

			revenue(p, t)*Sell(p, t) - prodcost(p)*Make(p, t) - invcost(p)*Inv(p, t);

	}));


　

	subject_to("Time", [&](range<1,T> t) {

		return sum([&](PROD p) { return (1 / rate(p)) * Make(p,t); }) <= avail(t);

	});


	subject_to("Init_Inv", [&](PROD p) {

		return Inv(p, 0) == inv0(p);

	});


	subject_to("Balance", [&](PROD p, range<1, T> t) {

		return Make(p, t) + Inv(p, t - 1) == Sell(p, t) + Inv(p, t);

	});
  

```
In the examples folder of the repository you will find the C++ equivalents of the models from chapters 1 to 4 of "AMPL: A Modeling Language for Mathematical Programming" (http://ampl.com/resources/the-ampl-book/). The goal is to eventually translate all the examples in the book (http://ampl.com/resources/the-ampl-book/example-files/), http://users.iems.northwestern.edu/~4er/amplweb/EXAMPLES/PAPER1/index.html and http://users.iems.northwestern.edu/~4er/amplweb/NEW/LOOP2/index.html into C++

The repository does not include the glpk and lp_solve headers and libraries needed to compile milpcpp and the examples but the cmake files will tell you where to put them.

This is a work in progress. Some non-breaking re-design is planned (use of variadic templates, function arguments need rvalue reference versions, etc..)

Don't hesitate give comments, suggestions or ask for help compiling and running.


Similar in spirit but pre-C++11: http://projects.coin-or.org/FlopC%2B%2B/browser/releases/1.1.2/FlopCpp/test/unitTest.cpp

This one uses more or less the same approach as the Gurobi and CPLEX C++ interfaces: http://projects.coin-or.org/Rehearse
