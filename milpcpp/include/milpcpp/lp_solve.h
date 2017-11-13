#ifndef __MILPCPP_LP_SOLVE_H__
#define __MILPCPP_LP_SOLVE_H__

#include<functional>
struct _lprec;

namespace milpcpp
{
	class model;

	class lp_solve
	{
		model * _model;
		_lprec * _lp;
		double * _variable_values;
	public:
		lp_solve(model *m);
		~lp_solve();
		void solve();

		template<typename T>
		void get_values(const T& vars, const typename T::value_iterator_t&f)
		{
			size_t size = vars.size();
			size_t start_index = vars.start_index();
			for (int i = 0; i < size; ++i)
			{
				invoke(i, _variable_values[start_index + i], f);
			}
		}

		double get_objective_value();

	};
}

#endif
