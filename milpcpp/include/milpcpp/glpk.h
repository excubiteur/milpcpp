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
		void get_values(const T& vars, const std::function<void(typename T::index_type, typename T::value_type)>&f)
		{
			size_t size = vars.size();
			size_t start_index = vars.start_index();
			for (int i = 0; i < size; ++i)
			{
				f(T::index_type(i), get_variable_value(start_index + i));
			}
		}

		template<typename T>
		void get_values(const T& vars, const std::function<void(typename T::index_type_1, typename T::index_type_2, typename T::value_type)>&f)
		{
			typedef typename T::index_type_1 T1;
			typedef typename T::index_type_2 T2;
			size_t size1 = T1::size();
			size_t size2 = T2::size();
			size_t start_index = vars.start_index();
			for (int i = 0; i < size1; ++i)
			{
				for (int j = 0; j < size2; ++j)
				{
					f(T1(i), T2(j),  get_variable_value(start_index + i*size2 + j));
				}
			}
		}

		double get_objective_value();
	};
}

#endif
