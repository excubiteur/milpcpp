#ifndef __MILPCPP_GLPK_H__
#define __MILPCPP_GLPK_H__

#include <functional>

struct glp_prob;

namespace milpcpp
{
	class model;

	class glpk
	{
		model * _model;
		glp_prob * _lp;

		double get_variable_value(size_t absolute_index);
	public:
		glpk(model *m);
		~glpk();
		void solve();

		template<typename T>
		void get_values(const T& vars, const typename T::value_iterator_t&f)
		{
			size_t size = vars.size();
			size_t start_index = vars.start_index();
			for (int i = 0; i < size; ++i)
			{
				invoke(i, get_variable_value(start_index + i), f);
			}
		}

		double get_objective_value();
	};
}

#endif
