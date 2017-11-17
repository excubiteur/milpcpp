#include <milpcpp/milpcpp.h>
#include <milpcpp/glpk.h>
#include <milpcpp/lp_solve.h>

#include<cassert>
#include<iostream>

// AMPL model to translate
// From the book: "AMPL: A Modeling Language for Mathematical Programming" 
// http://ampl.com/resources/the-ampl-book/
// steelT2.mod (Chapter 5)
/*
set PROD;           # products
set WEEKS ordered;  # number of weeks

param rate {PROD} > 0;           # tons per hour produced
param inv0 {PROD} >= 0;          # initial inventory
param avail {WEEKS} >= 0;        # hours available in week
param market {PROD,WEEKS} >= 0;  # limit on tons sold in week

param prodcost {PROD} >= 0;      # cost per ton produced
param invcost {PROD} >= 0;       # carrying cost/ton of inventory
param revenue {PROD,WEEKS} >= 0; # revenue/ton sold

var Make {PROD,WEEKS} >= 0;      # tons produced
var Inv {PROD,WEEKS} >= 0;       # tons inventoried
var Sell {p in PROD, t in WEEKS} >= 0, <= market[p,t]; # tons sold

maximize Total_Profit:
sum {p in PROD, t in WEEKS} (revenue[p,t]*Sell[p,t] -
prodcost[p]*Make[p,t] - invcost[p]*Inv[p,t]);

# Objective: total revenue less costs in all weeks

subject to Time {t in WEEKS}:
sum {p in PROD} (1/rate[p]) * Make[p,t] <= avail[t];

# Total of hours used by all products
# may not exceed hours available, in each week

subject to Balance0 {p in PROD}:
Make[p,first(WEEKS)] + inv0[p]
= Sell[p,first(WEEKS)] + Inv[p,first(WEEKS)];

subject to Balance {p in PROD, t in WEEKS: ord(t) > 1}:
Make[p,t] + Inv[p,prev(t)] = Sell[p,t] + Inv[p,t];

# Tons produced and taken from inventory
# must equal tons sold and put into inventory

*/



void steelT2initlist()
{
	using namespace milpcpp;

	model m;

	MILPCPP_SET_INIT(PROD, "bands", "coils" );
	MILPCPP_SET_INIT(WEEKS, "27sep", "04oct", "11oct", "18oct" );

	param<PROD>              rate{ 200, 140 };
	param<PROD>              inv0{ 10, 0 };
	param<WEEKS>             avail{ 40,40,32,40 };


	param<PROD,WEEKS>        market{
		{ 6000,  6000,  4000,  6500 },
		{ 4000,  2500,  3500,  4200 }
	};

	param<PROD>               prodcost{ 10, 11 };
	param<PROD>               invcost{ 2.5, 3 };


	param<PROD, WEEKS>        revenue{
		{ 25,    26,    27,    27 },
		{ 30,    35,    37,    39 }

	};

	var<PROD, WEEKS>    Make(greater_equal(0));
	var<PROD, WEEKS>    Inv(greater_equal(0));
	var<PROD, WEEKS>    Sell(greater_equal(0), less_equal([&](PROD p, WEEKS t) { return market(p, t); }));

	m.seal_data();

	maximize("Total_Profit",
		sum([&](PROD p, WEEKS t) { return
			revenue(p, t)*Sell(p, t) - prodcost(p)*Make(p, t) - invcost(p)*Inv(p, t);
	}));


	subject_to("Time", [&](WEEKS t) {
		return sum([&](PROD p) { return (1 / rate(p)) * Make(p,t); }) <= avail(t);
	});

	subject_to("Balance0", [&](PROD p) {
		return Make(p, first<WEEKS>()) + inv0(p) == 
			Sell(p, first<WEEKS>()) + Inv(p, first<WEEKS>());
	});

	subject_to("Balance", [&](PROD p, WEEKS t) {
		if (ord(t) > 1)
			return Make(p, t) + Inv(p, prev(t)) == Sell(p, t) + Inv(p, t);
		else
			return null_constraint();
	});

	// Solve

	{	// Solve using glpk
		std::cout << "glpk" << std::endl;

		glpk solver(&m);
		solver.solve();

		solver.get_values(Make, [&](auto value, auto i, auto k) {
			std::cout << "Make: "<< i.name() << "," << k.name() << " = " << value << std::endl;
		});

		solver.get_values(Inv, [&](auto value, auto i, auto j) {
			std::cout << "Inv: " << i.name() << "," << j.name() << " = " << value << std::endl;
		});

		solver.get_values(Sell, [&](auto value, auto i, auto j) {
			std::cout << "Sell: " << i.name() << "," << j.name() << " = " << value << std::endl;
		});

		std::cout << "objective = " << solver.get_objective_value() << std::endl;

		assert(long(solver.get_objective_value() + 0.5) == 515033);
 	}

	{	// Solve using lp_solve
		std::cout << "lp_solve" << std::endl;

		lp_solve solver(&m);
		solver.solve();

		solver.get_values(Make, [&](auto value, auto i, auto k) {
			std::cout << "Make: " << i.name() << "," << k.name() << " = " << value << std::endl;
		});

		solver.get_values(Inv, [&](auto value, auto i, auto j) {
			std::cout << "Inv: " << i.name() << "," << j.name() << " = " << value << std::endl;
		});

		solver.get_values(Sell, [&](auto value, auto i, auto j) {
			std::cout << "Sell: " << i.name() << "," << j.name() << " = " << value << std::endl;
		});


		std::cout << "objective = " << solver.get_objective_value() << std::endl;

		assert(long(solver.get_objective_value() + 0.5) == 515033);
	}

}

