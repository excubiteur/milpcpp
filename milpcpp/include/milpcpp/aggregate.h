#ifndef __MILPCPP_AGGREGATE_H__
#define __MILPCPP_AGGREGATE_H__

#include<functional>

#include<milpcpp/expressions.h>
#include<milpcpp/model.h>

namespace milpcpp
{
	template<typename T1, typename...Ts>
	expression get_sum(const std::function<expression(T1, Ts...)>& f)
	{
		expressions::sum result;
		size_t size = T1::size();
		for (size_t i = 0; i < size; ++i)
		{
			auto f2 = [=](Ts...args)
			{
				return f(T1(i), args...);
			};

			auto sum_expr = get_sum<Ts...>(f2);
			add(result, sum_expr);
		}
		return result;
	}

	template<typename T>
	expression get_sum(const std::function<expression(T)>& f)
	{
		expressions::sum result;
		size_t size = T::size();
		for (size_t i = 0; i < size; ++i)
		{
			add(result, f(T(i)));
		}
		return result;
	}

	template<typename T>
	expression sum(T f)
	{
		typedef utils::function_traits<T> traits;
		typedef traits::function_type F;
		F func(f);
		return get_sum(func);
	}

	inline void maximize(const char * name, expression&&e)
	{
		model::set_objective(std::move(e));
		model::set_maximixe();
	}

	inline void minimize(const char * name, expression&&e)
	{
		model::set_objective(std::move(e));
		model::set_minimize();
	}

	template<typename T1, typename...Ts>
	std::vector<constraint> get_constraints(const std::function<constraint(T1,Ts...)>& f)
	{
		std::vector<constraint> result;
		size_t size = T1::size();
		for (size_t i = 0; i < size; ++i)
		{
			auto f2 = [=](Ts...args) 
			{ 
				return f(T1(i), args...);
			};

			auto constraints = get_constraints<Ts...>(f2);
			std::move(constraints.begin(), constraints.end(), std::back_inserter(result));
		}
		return result;
	}

	template<typename T>
	std::vector<constraint> get_constraints(const std::function<constraint(T)>& f)
	{
		std::vector<constraint> result;
		size_t size = T::size();
		for (size_t i = 0; i < size; ++i)
		{
			result.push_back(f(T(i)));
		}
		return result;
	}

	template<typename T>
	inline void subject_to(const char * name, const T&f)
	{
		typedef utils::function_traits<T> traits;
		typedef traits::function_type F;
		F func(f);
		model::add_constraints(get_constraints(func));
	}

	template<>
	inline void subject_to<constraint>(const char * name, const constraint&c)
	{
		model::add_constraints(std::vector<constraint>{c});
	}
}

#endif
