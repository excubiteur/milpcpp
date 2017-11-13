#ifndef __MILPCPP_MILPCPP_H__
#define __MILPCPP_MILPCPP_H__

#include<algorithm>
#include<functional>
#include<string>

#include<milpcpp/bounds.h>
#include<milpcpp/indexing.h>
#include<milpcpp/model.h>
#include<milpcpp/param.h>
#include<milpcpp/utils.h>
#include<milpcpp/var.h>

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

	inline void maximize(const char * name, const expression&e)
	{
		model::set_objective(e);
		model::set_maximixe();
	}

	inline void minimize(const char * name, const expression&e)
	{
		model::set_objective(e);
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
			result.insert(result.end(), constraints.begin(), constraints.end());
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
		std::vector<constraint> constraints = get_constraints(func);
		model::add_constraints(constraints);
	}

	template<>
	inline void subject_to<constraint>(const char * name, const constraint&c)
	{
		model::add_constraints(std::vector<constraint>{c});
	}
}

#define MILPCPP_SET(X) \
struct X:public milpcpp::indexing::index<X>   \
{   \
	explicit X(size_t i):milpcpp::indexing::index<X>(i){} \
};   \
milpcpp::indexing::index_set __##X##internal##__;   \
X::_index_set =  &__##X##internal##__

#define MILPCPP_TYPED_PARAM(X) struct X:public milpcpp::indexing::range_bound<X> { };

#endif
