#include <milpcpp/milpcpp.h>
#include <milpcpp/glpk.h>
#include <milpcpp/lp_solve.h>

#include <milpcpp/enumerate.h>

#include<cassert>
#include<iostream>

// AMPL model to translate
// From the book: "AMPL: A Modeling Language for Mathematical Programming" 
// http://ampl.com/resources/the-ampl-book/
// transp.mod (Chapter 3)
/*
set ORIG;   # origins
set DEST;   # destinations

param supply {ORIG} >= 0;   # amounts available at origins
param demand {DEST} >= 0;   # amounts required at destinations

check: sum {i in ORIG} supply[i] = sum {j in DEST} demand[j];

param cost {ORIG,DEST} >= 0;   # shipment costs per unit
var Trans {ORIG,DEST} >= 0;    # units to be shipped

minimize Total_Cost:
sum {i in ORIG, j in DEST} cost[i,j] * Trans[i,j];

subject to Supply {i in ORIG}:
sum {j in DEST} Trans[i,j] = supply[i];

subject to Demand {j in DEST}:
sum {i in ORIG} Trans[i,j] = demand[j];


*/



void transp(
	const std::vector<std::string>& ORIG_data, 
	const std::vector<std::string>& DEST_data,
	const std::vector<double>& supply_data,
	const std::vector<double>& demand_data,
	const std::vector<std::vector<double> >& cost_data
	)
{
	using namespace milpcpp;

	model m;

	MILPCPP_SET(ORIG);
	MILPCPP_SET(DEST);

	param<ORIG> supply(greater_than(0));
	param<DEST> demand(greater_equal(0));

	param<ORIG,DEST> cost(greater_equal(0));
	
	var<ORIG, DEST> Trans(greater_equal(0));


	//////////////////////////////////////////////////////////
	// Start data
	for(const auto& n: ORIG_data)
		ORIG::add(n);

	for (const auto& f :DEST_data)
		DEST::add(f);

	for (const auto&[data_index, o] : utils::enumerate(ORIG_data))
	{
		supply.add(o, supply_data[data_index]);
	}

	for (const auto&[data_index, d] : utils::enumerate(DEST_data))
	{
		demand.add(d, demand_data[data_index]);
	}

	for (const auto&[data_index, o] : utils::enumerate(ORIG_data))
	{
		for (const auto&[data_index2, d] : utils::enumerate(DEST_data))
		{
			cost.add(o, d, cost_data[data_index][data_index2]);
		}
	}

	m.seal_data();

	// End data
	//////////////////////////////////////////////////////////




	minimize("Total_Cost", 
		sum([&](ORIG i, DEST j) { return cost(i,j)*Trans(i, j); }
	));

	subject_to("Supply", [&](ORIG i) {
		return sum([&](DEST j) { return Trans(i,j); }) == supply(i);
	});

	subject_to("Demand", [&](DEST j) {
		return sum([&](ORIG i) { return Trans(i, j); }) == demand(j);
	});

	// Solve

	{	// Solve using glpk
		std::cout << "glpk" << std::endl;

		glpk solver(&m);
		solver.solve();

		solver.get_values(Trans, [](auto value, ORIG i, DEST j) {
			std::cout << i.name() << "," << j.name() << " = " << value << std::endl;
		});

		std::cout << "objective = " << solver.get_objective_value() << std::endl;

		assert(long(solver.get_objective_value() + 0.5) == 196200);
	}

	{	// Solve using lp_solve
		std::cout << "lp_solve" << std::endl;

		lp_solve solver(&m);
		solver.solve();

		solver.get_values(Trans, [](auto value, ORIG i, DEST j) {
			std::cout << i.name() << "," << j.name() << " = " << value << std::endl;
		});

		std::cout << "objective = " << solver.get_objective_value() << std::endl;

		assert(long(solver.get_objective_value() + 0.5) == 196200);
	}

}

void transp()
{
	std::vector<std::string> ORIG_data{"GARY", "CLEV", "PITT"};
	std::vector<std::string> DEST_data{"FRA","DET","LAN","WIN","STL","FRE","LAF" };
	std::vector<double> supply_data{ 1400, 2600, 2900};
	std::vector<double> demand_data{ 900, 1200, 600, 400, 1700, 1100, 1000 };
	std::vector<std::vector<double> > cost_data{
		{   39,   14,  11,   14,   16,   82,    8},
		{   27,    9,   12,    9,   26,   95,   17},
		{   24,   14,   17,   13,   28,   99,   20}
	};

	transp(ORIG_data, DEST_data, supply_data, demand_data, cost_data);
}
