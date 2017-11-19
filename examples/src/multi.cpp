#include <milpcpp/milpcpp.h>
#include <milpcpp/glpk.h>
#include <milpcpp/lp_solve.h>

#include <milpcpp/enumerate.h>

#include<cassert>
#include<iostream>

// AMPL model to translate
// From the book: "AMPL: A Modeling Language for Mathematical Programming" 
// http://ampl.com/resources/the-ampl-book/
// multi.mod (Chapter 4)
/*
set ORIG;   # origins
set DEST;   # destinations
set PROD;   # products

param supply {ORIG,PROD} >= 0;  # amounts available at origins
param demand {DEST,PROD} >= 0;  # amounts required at destinations

check {p in PROD}:
sum {i in ORIG} supply[i,p] = sum {j in DEST} demand[j,p];

param limit {ORIG,DEST} >= 0;

param cost {ORIG,DEST,PROD} >= 0;  # shipment costs per unit
var Trans {ORIG,DEST,PROD} >= 0;   # units to be shipped

minimize Total_Cost:
sum {i in ORIG, j in DEST, p in PROD}
cost[i,j,p] * Trans[i,j,p];

subject to Supply {i in ORIG, p in PROD}:
sum {j in DEST} Trans[i,j,p] = supply[i,p];

subject to Demand {j in DEST, p in PROD}:
sum {i in ORIG} Trans[i,j,p] = demand[j,p];

subject to Multi {i in ORIG, j in DEST}:
sum {p in PROD} Trans[i,j,p] <= limit[i,j];

*/



void multi(
	const std::vector<std::string>& ORIG_data, 
	const std::vector<std::string>& DEST_data,
	const std::vector<std::string>& PROD_data,
	const std::vector<std::vector<double> >& supply_data,
	const std::vector<std::vector<double> >& demand_data,
	double limit_data,
	const std::vector<std::vector<std::vector<double> > >& cost_data
)
{
	using namespace milpcpp;

	model m;

	MILPCPP_SET(ORIG);
	MILPCPP_SET(DEST);
	MILPCPP_SET(PROD);

	param<ORIG, PROD>  supply(greater_than(0));
	param<DEST, PROD>  demand(greater_equal(0));

	param<ORIG, DEST>  limit (greater_than(0));

	param<ORIG,DEST,PROD> cost(greater_equal(0));
	
	var<ORIG, DEST, PROD> Trans(greater_equal(0));


	//////////////////////////////////////////////////////////
	// Start data
	for(const auto& o: ORIG_data)
		ORIG::add(o);

	for (const auto& d :DEST_data)
		DEST::add(d);

	for (const auto& p : PROD_data)
		PROD::add(p);

	for (const auto&[data_index, o] : utils::enumerate(ORIG_data))
	{
		for (const auto&[data_index2, p] : utils::enumerate(PROD_data) )
		{
			supply.add(o, p, supply_data[data_index2][data_index]);
		}
	}

	for (const auto&[data_index,  d] : utils::enumerate(DEST_data) )
	{
		for (const auto&[data_index2, p] : utils::enumerate(PROD_data) )
		{
			demand.add(d, p, demand_data[data_index2][data_index]);
		}
	}

	for (const auto& [data_index,o] : utils::enumerate(ORIG_data))
	{
		for (const auto&[data_index2, d] : utils::enumerate(DEST_data))
		{
			for (const auto&[data_index3, p] : utils::enumerate(PROD_data))
			{
				double value = cost_data[data_index3][data_index][data_index2];
				cost.add(o, d, p, value);
			}
		}
	}

	limit.set_default(limit_data);

	m.seal_data();
	// End data
	//////////////////////////////////////////////////////////




	minimize("Total_Cost", 
		sum([&](ORIG i, DEST j, PROD p) { return cost(i,j, p)*Trans(i, j, p); }
	));

	subject_to("Supply", [&](ORIG i, PROD p) {
		return sum([&](DEST j) { return Trans(i,j, p); }) == supply(i,p);
	});

	subject_to("Demand", [&](DEST j, PROD p) {
		return sum([&](ORIG i) { return Trans(i, j, p); }) == demand(j, p);
	});

	subject_to("Multi", [&](ORIG i, DEST j) {
		return sum([&](PROD p) { return Trans(i, j, p); }) <= limit(i, j);
	});

	// Solve

	{	// Solve using glpk
		std::cout << "glpk" << std::endl;

		glpk solver(&m);
		solver.solve();

		solver.get_values(Trans, [&](double value, ORIG i, DEST j, PROD k) {
			std::cout << i.name() << "," << j.name() << "," << k.name() << " = " << value << std::endl;
		});

		std::cout << "objective = " << solver.get_objective_value() << std::endl;

		assert(long(solver.get_objective_value() + 0.5) == 199500);
 	}

	{	// Solve using lp_solve
		std::cout << "lp_solve" << std::endl;

		lp_solve solver(&m);
		solver.solve();

		solver.get_values(Trans, [](auto value, ORIG i, DEST j, PROD k) {
			std::cout << i.name() << "," << j.name() << "," << k.name() << " = " << value << std::endl;
		});

		std::cout << "objective = " << solver.get_objective_value() << std::endl;

		assert(long(solver.get_objective_value() + 0.5) == 199500);
	}

}

void multi()
{
	// multi.dat
	/*
	data;

	set ORIG := GARY CLEV PITT ;
	set DEST := FRA DET LAN WIN STL FRE LAF ;
	set PROD := bands coils plate ;

	param supply (tr):  GARY   CLEV   PITT :=
	bands    400    700    800
	coils    800   1600   1800
	plate    200    300    300 ;

	param demand (tr):
	        FRA     DET   LAN   WIN   STL   FRE   LAF :=
	bands   300     300   100    75   650   225   250
	coils   500     750   400   250   950   850   500
	plate   100     100     0    50   200   100   250 ;

	param limit default 625 ;

	param cost :=

	[*,*,bands]:  FRA  DET  LAN  WIN  STL  FRE  LAF :=
	GARY          30   10    8   10   11   71    6
	CLEV          22    7   10    7   21   82   13
	PITT          19   11   12   10   25   83   15

	[*,*,coils]:  FRA  DET  LAN  WIN  STL  FRE  LAF :=
	GARY          39   14   11   14   16   82    8
	CLEV          27    9   12    9   26   95   17
	PITT          24   14   17   13   28   99   20

	[*,*,plate]:  FRA  DET  LAN  WIN  STL  FRE  LAF :=
	GARY          41   15   12   16   17   86    8
	CLEV          29    9   13    9   28   99   18
	PITT          26   14   17   13   31  104   20 ;
	*/

	std::vector<std::string> ORIG_data{"GARY", "CLEV", "PITT"};
	std::vector<std::string> DEST_data{"FRA","DET","LAN","WIN","STL","FRE","LAF" };
	std::vector<std::string> PROD_data{ "bands", "coils", "plates" };

	std::vector<std::vector<double>> supply_data{ 
		{ 400, 700,    800 },
		{ 800, 1600,   1800 },
		{ 200, 300,  300}
	};

	std::vector<std::vector<double>> demand_data{
		{   300,     300 ,  100   , 75 ,  650,   225 ,  250},
		{   500,     750,   400 ,  250  , 950 ,  850 ,  500},
		{   100 ,    100 ,    0 ,   50 ,  200 ,  100 ,  250}

	};


	std::vector< std::vector<std::vector<double> >  >cost_data{
		{
			{30 ,  10 ,   8 ,  10 ,  11 ,  71 ,   6},
			{22 ,   7 ,  10 ,   7 ,  21  , 82 ,  13},
			{ 19  , 11  , 12 ,  10 ,  25 ,  83 ,  15}
		},

		{
			{39 ,  14  , 11 ,  14 ,  16 ,  82 ,   8},
			{  27 ,   9  , 12 ,   9  , 26,   95,   17},
			{    24 ,  14  , 17 ,  13 ,  28  , 99 ,  20}
		},
		
		{
			{  41 ,  15 ,  12 ,  16,   17 ,  86 ,   8},
			{  29 ,   9  , 13  ,  9 ,  28 ,  99 ,  18},
			{    26 ,  14  , 17 ,  13 ,  31 , 104 ,  20}
		}

	};

	multi(ORIG_data, DEST_data, PROD_data, supply_data, demand_data, 625, cost_data);
}
