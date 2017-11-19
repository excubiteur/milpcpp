#include <milpcpp/milpcpp.h>
#include <milpcpp/glpk.h>
#include <milpcpp/lp_solve.h>

#include <milpcpp/enumerate.h>

#include<cassert>
#include<iostream>

// AMPL model to translate
// From the book: "AMPL: A Modeling Language for Mathematical Programming" 
// http://ampl.com/resources/the-ampl-book/
// steelP.mod (Chapter 4)
/*
set ORIG;   # origins (steel mills)
set DEST;   # destinations (factories)
set PROD;   # products

param rate {ORIG,PROD} > 0;     # tons per hour at origins
param avail {ORIG} >= 0;        # hours available at origins
param demand {DEST,PROD} >= 0;  # tons required at destinations

param make_cost {ORIG,PROD} >= 0;        # manufacturing cost/ton
param trans_cost {ORIG,DEST,PROD} >= 0;  # shipping cost/ton

var Make {ORIG,PROD} >= 0;       # tons produced at origins
var Trans {ORIG,DEST,PROD} >= 0; # tons shipped

minimize Total_Cost:
sum {i in ORIG, p in PROD} make_cost[i,p] * Make[i,p] +
sum {i in ORIG, j in DEST, p in PROD}
trans_cost[i,j,p] * Trans[i,j,p];

subject to Time {i in ORIG}:
sum {p in PROD} (1/rate[i,p]) * Make[i,p] <= avail[i];

subject to Supply {i in ORIG, p in PROD}:
sum {j in DEST} Trans[i,j,p] = Make[i,p];

subject to Demand {j in DEST, p in PROD}:
sum {i in ORIG} Trans[i,j,p] = demand[j,p];

*/



void steelP(
	const std::vector<std::string>& ORIG_data, 
	const std::vector<std::string>& DEST_data,
	const std::vector<std::string>& PROD_data,
	const std::vector<std::vector<double> >& rate_data,
	const std::vector<double>& avail_data,
	const std::vector<std::vector<double> >& demand_data,
	const std::vector<std::vector<double> >& make_cost_data,
	const std::vector<std::vector<std::vector<double> > >& trans_cost_data
)
{
	using namespace milpcpp;

	model m;

	MILPCPP_SET(ORIG);
	MILPCPP_SET(DEST);
	MILPCPP_SET(PROD);

	param<ORIG, PROD>  rate(greater_than(0));
	param<ORIG>        avail(greater_than(0));
	param<DEST, PROD>  demand(greater_equal(0));

	param<ORIG, PROD>        make_cost(greater_than(0));
	param<ORIG, DEST, PROD>  trans_cost (greater_than(0));

	var<ORIG,PROD>          Make(greater_equal(0));	
	var<ORIG, DEST, PROD>   Trans(greater_equal(0));


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
		avail.add(o, avail_data[data_index]);
		for (const auto&[data_index2, p] : utils::enumerate(PROD_data))
		{
			rate.add(o, p, rate_data[data_index2][data_index]);
			make_cost.add(o, p, make_cost_data[data_index2][data_index]);
		}
	}

	for (const auto&[data_index, d] : utils::enumerate(DEST_data))
	{
		for (const auto&[data_index2, p] : utils::enumerate(PROD_data))
		{
			demand.add(d, p, demand_data[data_index2][data_index]);
		}
	}

	for (const auto&[data_index, o] : utils::enumerate(ORIG_data))
	{
		for (const auto&[data_index2, d] : utils::enumerate(DEST_data))
		{
			int data_index3 = 0;
			for (const auto&[data_index3, p] : utils::enumerate(PROD_data))
			{
				double value = trans_cost_data[data_index3][data_index][data_index2];
				trans_cost.add(o, d, p, value);
			}
		}
	}

	m.seal_data();
	// End data
	//////////////////////////////////////////////////////////




	minimize("Total_Cost", 		
		sum([&](ORIG i, PROD p) { return make_cost(i, p)*Make(i, p); })
		+
		sum([&](ORIG i, DEST j, PROD p) { return trans_cost(i, j, p)*Trans(i, j, p); }));

	subject_to("Time", [&](ORIG i) {
		return sum([&](PROD p) { return (1 / rate(i, p)) * Make(i, p); }) <= avail(i);
	});

	subject_to("Supply", [&](ORIG i, PROD p) {
		return sum([&](DEST j) { return Trans(i,j, p); }) == Make(i,p);
	});

	subject_to("Demand", [&](DEST j, PROD p) {
		return sum([&](ORIG i) { return Trans(i, j, p); }) == demand(j, p);
	});

	// Solve

	{	// Solve using glpk
		std::cout << "glpk" << std::endl;

		glpk solver(&m);
		solver.solve();

		solver.get_values(Make, [&](auto value, ORIG i, PROD k) {
			std::cout << i.name() << "," << k.name() << " = " << value << std::endl;
		});

		solver.get_values(Trans, [&](auto value, ORIG i, DEST j, PROD k) {
			std::cout << i.name() << "," << j.name() << "," << k.name() << " = " << value << std::endl;
		});

		std::cout << "objective = " << solver.get_objective_value() << std::endl;

		assert(long(solver.get_objective_value() + 0.5) == 1392175);
 	}

	{	// Solve using lp_solve
		std::cout << "lp_solve" << std::endl;

		lp_solve solver(&m);
		solver.solve();

		solver.get_values(Make, [&](auto value, ORIG i, PROD k) {
			std::cout << i.name() << "," << k.name() << " = " << value << std::endl;
		});

		solver.get_values(Trans, [](auto value, ORIG i, DEST j, PROD k) {
			std::cout << i.name() << "," << j.name() << "," << k.name() << " = " << value << std::endl;
		});

		std::cout << "objective = " << solver.get_objective_value() << std::endl;

		assert(long(solver.get_objective_value() + 0.5) == 1392175);
	}

}

void steelP()
{
	// steelP.dat
	/*
	data;

	set ORIG := GARY CLEV PITT ;
	set DEST := FRA DET LAN WIN STL FRE LAF ;
	set PROD := bands coils plate ;

	param avail :=  GARY 20  CLEV 15  PITT 20 ;

	param demand (tr):
	FRA   DET   LAN   WIN   STL   FRE   LAF :=
	bands    300   300   100    75   650   225   250
	coils    500   750   400   250   950   850   500
	plate    100   100     0    50   200   100   250 ;

	param rate (tr):  GARY   CLEV   PITT :=
	bands    200    190    230
	coils    140    130    160
	plate    160    160    170 ;

	param make_cost (tr):
	GARY   CLEV   PITT :=
	bands    180    190    190
	coils    170    170    180
	plate    180    185    185 ;

	param trans_cost :=

	[*,*,bands]:  FRA  DET  LAN  WIN  STL  FRE  LAF :=
	GARY    30   10    8   10   11   71    6
	CLEV    22    7   10    7   21   82   13
	PITT    19   11   12   10   25   83   15

	[*,*,coils]:  FRA  DET  LAN  WIN  STL  FRE  LAF :=
	GARY    39   14   11   14   16   82    8
	CLEV    27    9   12    9   26   95   17
	PITT    24   14   17   13   28   99   20

	[*,*,plate]:  FRA  DET  LAN  WIN  STL  FRE  LAF :=
	GARY    41   15   12   16   17   86    8
	CLEV    29    9   13    9   28   99   18
	PITT    26   14   17   13   31  104   20 ;
	*/

	std::vector<std::string> ORIG_data{"GARY", "CLEV", "PITT"};
	std::vector<std::string> DEST_data{"FRA","DET","LAN","WIN","STL","FRE","LAF" };
	std::vector<std::string> PROD_data{ "bands", "coils", "plates" };

	std::vector<double> avail_data{ 20, 15, 20 };

	std::vector<std::vector<double>> demand_data{
		{   300,     300 ,  100   , 75 ,  650,   225 ,  250},
		{   500,     750,   400 ,  250  , 950 ,  850 ,  500},
		{   100 ,    100 ,    0 ,   50 ,  200 ,  100 ,  250}

	};

	std::vector<std::vector<double>> rate_data{
		{ 200,    190,    230 },
		{ 140,    130,    160 },
		{ 160,    160,    170 }
	};

	std::vector<std::vector<double>> make_cost_data{
		{180,    190,    190},
		{170,    170,    180},
		{180,    185,    185}
	};


	std::vector< std::vector<std::vector<double> >  >trans_cost_data{
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

	steelP(ORIG_data, DEST_data, PROD_data,  rate_data, avail_data, demand_data, make_cost_data, trans_cost_data);
}
