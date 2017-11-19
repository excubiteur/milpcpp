#include <milpcpp/milpcpp.h>
#include <milpcpp/glpk.h>
#include <milpcpp/lp_solve.h>

#include <milpcpp/enumerate.h>

#include<cassert>
#include<iostream>

// AMPL model to translate
// From the book: "AMPL: A Modeling Language for Mathematical Programming" 
// http://ampl.com/resources/the-ampl-book/
// diet.mod (Chapter 2)
/* 
set NUTR;
set FOOD;

param cost {FOOD} > 0;
param f_min {FOOD} >= 0;
param f_max {j in FOOD} >= f_min[j];

param n_min {NUTR} >= 0;
param n_max {i in NUTR} >= n_min[i];

param amt {NUTR,FOOD} >= 0;

var Buy {j in FOOD} >= f_min[j], <= f_max[j];

minimize Total_Cost:  sum {j in FOOD} cost[j] * Buy[j];

subject to Diet {i in NUTR}:
n_min[i] <= sum {j in FOOD} amt[i,j] * Buy[j] <= n_max[i];

*/



void diet(
	const std::vector<std::string>& NUTR_data, 
	const std::vector<std::string>& FOOD_data,
	const std::vector<double>& cost_data,
	const std::vector<double>& f_min_data,
	const std::vector<double>& f_max_data,
	const std::vector<double>& n_min_data,
	const std::vector<double>& n_max_data,
	const std::vector<std::vector<double> >& amt_data
	)
{
	using namespace milpcpp;

	model m;

	MILPCPP_SET(NUTR);
	MILPCPP_SET(FOOD);

	param<FOOD> cost(greater_than(0));
	param<FOOD> f_min(greater_equal(0));
	param<FOOD> f_max(greater_equal([&](FOOD j) { return f_min(j); }));
	
	param<NUTR> n_min(greater_equal(0));
	param<NUTR> n_max(greater_equal([&](NUTR j) { return n_min(j); }));

	param<NUTR, FOOD> amt(greater_equal(0));

	var<FOOD> Buy(
		greater_equal([&](FOOD j) { return f_min(j); }),
		less_equal([&](FOOD j) { return f_max(j); })
	);


	//////////////////////////////////////////////////////////
	// Start data


	for(const auto& n: NUTR_data)
		NUTR::add(n);

	for (const auto& f :FOOD_data)
		FOOD::add(f);

	for (const auto& [data_index, n] : utils::enumerate(NUTR_data) )
	{
		n_min.add(n, n_min_data[data_index]);
		n_max.add(n, n_max_data[data_index]);
	}

	for (const auto& [data_index, f] : utils::enumerate(FOOD_data) )
	{
		f_min.add(f, f_min_data[data_index]);
		f_max.add(f, f_max_data[data_index]);
		cost.add(f, cost_data[data_index]);
	}

	for (const auto& [data_index, n] : utils::enumerate(NUTR_data))
	{
		for (const auto& [data_index2, f] : utils::enumerate(FOOD_data))
		{
			amt.add(n, f, amt_data[data_index][data_index2]);
		}
	}

	m.seal_data();
	// End data
	//////////////////////////////////////////////////////////




	minimize("Total_Cost", 
		sum([&](FOOD j) { return cost(j)*Buy(j); }
	));

	subject_to("Diet", [&](NUTR i) {
		return n_min(i) <= sum([&](FOOD j) { return amt(i,j)*Buy(j); }) <= n_max(i);
	});

	// Solve

	{	// Solve using glpk
		std::cout << "glpk" << std::endl;

		glpk solver(&m);
		solver.solve();

		solver.get_values(Buy, [](double value, FOOD i) {
			std::cout << i.name() << " = " << value << std::endl;
		});

		std::cout << "objective = " << solver.get_objective_value() << std::endl;

		long optimum = long(solver.get_objective_value() * 10 + 0.5);
		assert(optimum == 882);
	}

	{	// Solve using lp_solve
		std::cout << "lp_solve" << std::endl;

		lp_solve solver(&m);
		solver.solve();

		solver.get_values(Buy, [](auto value, auto i) {
			std::cout << i.name() << " = " << value << std::endl;
		});

		std::cout << "objective = " << solver.get_objective_value() << std::endl;
		
		long optimum = long(solver.get_objective_value() * 10 + 0.5);
		assert(optimum == 882);	}

}

void diet()
{
	std::vector<std::string> NUTR_data{ "A", "B1", "B2", "C" };
	std::vector<std::string> FOOD_data{ "BEEF", "CHK", "FISH", "HAM", "MCH", "MTL", "SPG", "TUR" };
	std::vector<double> cost_data{ 3.19,2.59,2.29,2.89,1.89,1.99,1.99,2.49 };
	std::vector<double> f_min_data{ 0,0,0,0,0,0,0,0 };
	std::vector<double> f_max_data{ 100,100,100,100,100,100,100,100 };
	std::vector<double> n_min_data{ 700,700,700,700 };
	std::vector<double> n_max_data{ 10000,10000,10000,10000 };

	std::vector<std::vector<double>> amt_data{
		{60,8,8,40,	15,	70,	25,	60},
		{20,0,10,40,35,30,50,20},
		{10,20,15,35,15,15,25,15},
		{15,20,10,10,15,15,15,10}
	};

	diet(NUTR_data, FOOD_data, cost_data, f_min_data, f_max_data, n_min_data, n_max_data, amt_data);
}
