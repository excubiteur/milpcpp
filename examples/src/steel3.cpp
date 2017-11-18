#include <milpcpp/milpcpp.h>
#include <milpcpp/glpk.h>
#include <milpcpp/lp_solve.h>

#include <milpcpp/EnumerateIterator.h>

#include<cassert>
#include<iostream>

// AMPL model to translate
// From the book: "AMPL: A Modeling Language for Mathematical Programming" 
// http://ampl.com/resources/the-ampl-book/
// steel3.mod (Chapter 1)
/*
set PROD;  # products

param rate {PROD} > 0;     # produced tons per hour
param avail >= 0;          # hours available in week
param profit {PROD};       # profit per ton

param commit {PROD} >= 0;  # lower limit on tons sold in week
param market {PROD} >= 0;  # upper limit on tons sold in week

var Make {p in PROD} >= commit[p], <= market[p]; # tons produced

maximize Total_Profit: sum {p in PROD} profit[p] * Make[p];

# Objective: total profits from all products

subject to Time: sum {p in PROD} (1/rate[p]) * Make[p] <= avail;

# Constraint: total of hours used by all
# products may not exceed hours available

*/

void steel3(
	const std::vector<std::string>& PROD_data,
	const std::vector<double>& rate_data,
	double avail_data,
	const std::vector<double>& profit_data,
	const std::vector<double>& market_data,
	const std::vector<double>& commit_data
)
{
	using namespace milpcpp;

	model m;

	MILPCPP_SET(PROD);

	param<PROD> rate(greater_than(0));
	param<>     avail(greater_equal(0));
	param<PROD> profit;

	param<PROD> commit(greater_equal(0));
	param<PROD> market(greater_equal(0));

	var<PROD>   Make(
		greater_equal([&](PROD p) { return commit(p);  }), 
		less_equal([&](PROD p) { return market(p);  }) );


	//////////////////////////////////////////////////////////
	// Start data
	for (const auto& p : PROD_data)
		PROD::add(p);

	for (const auto& [data_index,p] : utils::Enumerate(PROD_data))
	{
		rate.add(p, rate_data[data_index]);
		profit.add(p, profit_data[data_index]);
		market.add(p, market_data[data_index]);
		commit.add(p, commit_data[data_index]);
	}

	avail = avail_data;

	m.seal_data();
	// End data
	//////////////////////////////////////////////////////////


	maximize("Total_Profit", sum([&](PROD p) { return profit(p)*Make(p);  }) );

	subject_to("Time", sum([&](PROD p) { return (1 / rate(p)) * Make(p); }) <= avail );


	// Solve

	{	// Solve using glpk
		std::cout << "glpk" << std::endl;

		glpk solver(&m);
		solver.solve();

		solver.get_values(Make, [](auto value, auto i) {
			std::cout << i.name() << " = " << value << std::endl;
		});

		std::cout << "objective = " << solver.get_objective_value()*10 << std::endl;

		long optimum = long(solver.get_objective_value() * 10000 + 0.5);
		assert(optimum == 1948285714);
	}

	{	// Solve using lp_solve
		std::cout << "lp_solve" << std::endl;

		lp_solve solver(&m);
		solver.solve();

		solver.get_values(Make, [](auto value, auto i) {
			std::cout << i.name() << " = " << value << std::endl;
		});

		std::cout << "objective = " << solver.get_objective_value() << std::endl;

		long optimum = long(solver.get_objective_value() * 10000 + 0.5);
		assert(optimum == 1948285714);
	}
}

void steel3()
{
	std::vector<std::string> PROD_data{ "bands", "coils", "plates" };
	std::vector<double> rate_data{ 200, 140, 160 };
	double avail_data = 40;
	std::vector<double> profit_data{ 25, 30, 29 };
	std::vector<double> market_data{ 6000, 4000, 3500 };
	std::vector<double> commit_data{ 1000, 500, 750 };

	steel3(PROD_data, rate_data, avail_data, profit_data, market_data, commit_data);
}
