#include <milpcpp/milpcpp.h>
#include <milpcpp/glpk.h>
#include <milpcpp/lp_solve.h>

#include <milpcpp/EnumerateIterator.h>

#include<cassert>
#include<iostream>

// AMPL model to translate
// From the book: "AMPL: A Modeling Language for Mathematical Programming" 
// http://ampl.com/resources/the-ampl-book/
// steel4.mod (Chapter 1)
/*
set PROD;   # products
set STAGE;  # stages

param rate {PROD,STAGE} > 0; # tons per hour in each stage
param avail {STAGE} >= 0;    # hours available/week in each stage
param profit {PROD};         # profit per ton

param commit {PROD} >= 0;    # lower limit on tons sold in week
param market {PROD} >= 0;    # upper limit on tons sold in week

var Make {p in PROD} >= commit[p], <= market[p]; # tons produced

maximize Total_Profit: sum {p in PROD} profit[p] * Make[p];

# Objective: total profits from all products

subject to Time {s in STAGE}:
sum {p in PROD} (1/rate[p,s]) * Make[p] <= avail[s];

# In each stage: total of hours used by all
# products may not exceed hours available


*/

void steel4(
	const std::vector<std::string>& PROD_data,
	const std::vector<std::string>&STAGE_data,
	const std::vector<std::vector<double>>& rate_data,
	const std::vector<double>& avail_data,
	const std::vector<double>& profit_data,
	const std::vector<double>& market_data,
	const std::vector<double>& commit_data
)
{
	using namespace milpcpp;

	model m;

	MILPCPP_SET(PROD);
	MILPCPP_SET(STAGE);

	param<PROD,STAGE> rate(greater_than(0));
	param<STAGE>      avail(greater_equal(0));
	param<PROD>       profit;

	param<PROD> commit(greater_equal(0));
	param<PROD> market(greater_equal(0));

	var<PROD>   Make(
		greater_equal([&](PROD p) { return commit(p);  }), 
		less_equal([&](PROD p) { return market(p);  }) );


	//////////////////////////////////////////////////////////
	// Start data
	for (const auto& p : PROD_data)
		PROD::add(p);

	for (const auto& p : STAGE_data)
		STAGE::add(p);

	for (const auto&[data_index, p] : utils::Enumerate(PROD_data))
	{
		for (const auto&[data_index2, s] : utils::Enumerate(STAGE_data))
		{
			rate.add(p, s, rate_data[data_index][data_index2]);
		}
		profit.add(p, profit_data[data_index]);
		market.add(p, market_data[data_index]);
		commit.add(p, commit_data[data_index]);
	}

	for (const auto&[data_index, s] : utils::Enumerate(STAGE_data))
		avail.add(s, avail_data[data_index]);


	m.seal_data();
	// End data
	//////////////////////////////////////////////////////////


	maximize("Total_Profit", sum([&](PROD p) { return profit(p)*Make(p);  }) );

	subject_to("Time", [&](STAGE s) {
		return sum([&](PROD p) { return (1 / rate(p, s)) * Make(p); }) <= avail(s);
	});


	// Solve

	{	// Solve using glpk
		std::cout << "glpk" << std::endl;

		glpk solver(&m);
		solver.solve();

		solver.get_values(Make, [](auto value, auto i) {
			std::cout << i.name() << " = " << value << std::endl;
		});

		std::cout << "objective = " << solver.get_objective_value() << std::endl;

		long optimum = long(solver.get_objective_value() * 10000 + 0.5);
		assert(optimum == 1900714286);
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
		assert(optimum == 1900714286);
	}
}

void steel4()
{
	std::vector<std::string> PROD_data{ "bands", "coils", "plates" };
	std::vector<std::string> STAGE_data{ "reheat", "roll" };

	std::vector<std::vector<double>>  rate_data{
		{200,200},
		{200, 140},
		{200, 160}
	};


	std::vector<double> avail_data{ 35, 40 };

	std::vector<double> profit_data{ 25, 30, 29 };
	std::vector<double> market_data{ 6000, 4000, 3500 };
	std::vector<double> commit_data{ 1000, 500, 750 };

	steel4(PROD_data, STAGE_data, rate_data, avail_data, profit_data, market_data, commit_data);
}
