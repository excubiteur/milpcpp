#include <milpcpp/milpcpp.h>
#include <milpcpp/glpk.h>
#include <milpcpp/lp_solve.h>

#include <milpcpp/EnumerateIterator.h>

#include<cassert>
#include<iostream>

// AMPL model to translate
// From the book: "AMPL: A Modeling Language for Mathematical Programming" 
// http://ampl.com/resources/the-ampl-book/
// steelT.mod (Chapter 4)
/*
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


*/



void steelT(
	const std::vector<std::string>& PROD_data,
	const std::vector<double>& avail_data,
	const std::vector<double>& rate_data,
	const std::vector<double>& inv0_data,
	const std::vector<double>& prodcost_data,
	const std::vector<double>& invcost_data,
	const std::vector<std::vector<double>>& revenue_data,
	const std::vector<std::vector<double>>& market_data,
	long T_data)
{
	using namespace milpcpp;

	model m;

	MILPCPP_SET(PROD);
	MILPCPP_TYPED_PARAM(T);

	param<PROD>              rate(greater_than(0));
	param<PROD>              inv0(greater_equal(0));
	param<range<1, T>>       avail(greater_than(0));
	param<PROD,range<1, T>>  market (greater_than(0));

	param<PROD>               prodcost(greater_than(0));
	param<PROD>               invcost(greater_than(0));
	param<PROD, range<1, T>>  revenue(greater_than(0));


	var<PROD, range<1, T>>    Make(greater_equal(0));
	var<PROD, range<0, T>>    Inv(greater_equal(0));
	var<PROD, range<1, T>>    Sell(greater_equal(0), less_equal([&](PROD p, range<1, T> t) { return market(p, t); }));


	//////////////////////////////////////////////////////////
	// Start data

	for (const auto& p : PROD_data)
		PROD::add(p);

	T::set_value(T_data);

	for (const auto&[data_index, p] : utils::Enumerate(PROD_data))
	{
		rate.add(p, rate_data[data_index]);
		inv0.add(p, inv0_data[data_index]);
		prodcost.add(p, prodcost_data[data_index]);
		invcost.add(p, invcost_data[data_index]);
		for (const auto&i : range<1, T>())
		{
			market.add(p, i.name(), market_data[data_index][i.raw_index()]);
			revenue.add(p, i.name(), revenue_data[data_index][i.raw_index()]);
		}
	}

	for (const auto&i : range<1, T>())
	{
		avail.add(i.name(), avail_data[i.raw_index()]);
	}


	m.seal_data();
	// End data
	//////////////////////////////////////////////////////////




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

void steelT()
{
	// steelT.dat
	/*
	data;

	param T := 4;
	set PROD := bands coils;

	param avail :=  1 40  2 40  3 32  4 40 ;

	param rate :=  bands 200   coils 140 ;
	param inv0 :=  bands  10   coils   0 ;

	param prodcost :=  bands 10    coils  11 ;
	param invcost  :=  bands  2.5  coils   3 ;

	param revenue:   1     2     3     4 :=
	bands    25    26    27    27
	coils    30    35    37    39 ;

	param market:    1     2     3     4 :=
	bands  6000  6000  4000  6500
	coils  4000  2500  3500  4200 ;

	*/
	long T_data = 4;

	std::vector<std::string> PROD_data{ "bands", "coils" };

	std::vector<double> avail_data{40,40,32,40 };

	std::vector<double> rate_data{ 200, 140 };
	std::vector<double> inv0_data{ 10, 0 };

	std::vector<double> prodcost_data{ 10, 11 };
	std::vector<double> invcost_data{ 2.5, 3 };

	std::vector<std::vector<double>> revenue_data{
		{ 25,    26,    27,    27 },
		{ 30,    35,    37,    39 }

	};

	std::vector<std::vector<double>> market_data{
		{ 6000,  6000,  4000,  6500 },
		{ 4000,  2500,  3500,  4200 }
	};

	steelT(PROD_data, avail_data, rate_data, inv0_data, prodcost_data, invcost_data, revenue_data, market_data, T_data);

}
