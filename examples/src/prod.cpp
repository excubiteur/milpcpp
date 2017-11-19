#include <milpcpp/milpcpp.h>
#include <milpcpp/glpk.h>
#include <milpcpp/lp_solve.h>

#include <milpcpp/enumerate.h>

#include<cassert>
#include<iostream>

// AMPL model to translate
// From the book: "AMPL: A Modeling Language for Mathematical Programming" 
// http://ampl.com/resources/the-ampl-book/
// prod.mod (Chapter 1)
/*
set P;

param a {j in P};
param b;
param c {j in P};
param u {j in P};

var X {j in P};

maximize Total_Profit: sum {j in P} c[j] * X[j];

subject to Time: sum {j in P} (1/a[j]) * X[j] <= b;

subject to Limit {j in P}: 0 <= X[j] <= u[j];
*/



void prod(
	const std::vector<std::string>& P_data, 
	const std::vector<double>& a_data,
	double b_data,
	const std::vector<double>& c_data,
	const std::vector<double>& u_data
	)
{
	using namespace milpcpp;

	model m;

	MILPCPP_SET(P);

	param<P> a;
	param<> b;
	param<P> c;
	param<P> u;

	var<P> X;



	//////////////////////////////////////////////////////////
	// Start data
	for(const auto& p: P_data)
		P::add(p);

	for (const auto&[data_index, p] : utils::enumerate(P_data))
	{
		a.add(p, a_data[data_index]);
		c.add(p, c_data[data_index]);
		u.add(p, u_data[data_index]);
	}

	b = b_data;

	m.seal_data();
	// End data
	//////////////////////////////////////////////////////////




	maximize("Total_Profit", 
		sum([&](P j) { return c(j)*X(j); }
	));

	subject_to("Time", sum([&](P j) { return (1 / a(j))*X(j); }) <= b);

	subject_to("Limit", [&](P j) { return 0 <= X(j) <= u(j); });


	// Solve

	{	// Solve using glpk
		std::cout << "glpk" << std::endl;

		glpk solver(&m);
		solver.solve();

		solver.get_values(X, [](auto value, auto i) {
			std::cout << i.name() << " = " << value << std::endl;
		});

		std::cout << "objective = " << solver.get_objective_value() << std::endl;

		assert(long(solver.get_objective_value() + 0.5) == 192000);

	}

	{	// Solve using lp_solve
		std::cout << "lp_solve" << std::endl;

		lp_solve solver(&m);
		solver.solve();

		solver.get_values(X, [](auto value, auto i) {
			std::cout << i.name() << " = " << value << std::endl;
		});

		std::cout << "objective = " << solver.get_objective_value() << std::endl;

		assert(long(solver.get_objective_value() + 0.5) == 192000);
	}

}

void prod()
{
	std::vector<std::string> P_data{ "bands" , "coils" };
	std::vector<double> a_data{ 200, 140 };
	double b_data = 40;
	std::vector<double> c_data{ 25,30 };
	std::vector<double> u_data{ 6000, 4000 };

	prod(P_data, a_data, b_data, c_data, u_data);
}
