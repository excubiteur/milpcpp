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

namespace milpcpp
{
	template<typename T1 = void, typename T2 = void, typename T3 = void>
	class var : public variable_set
	{
		std::string name(size_t absolute_index) const override
		{
			return 
				T1::name(  (absolute_index - _start_index) / (T2::size()*T3::size())) + 
				"," + 
				T2::name(  ((absolute_index - _start_index) % (T2::size()*T3::size()) ) /T3::size() )
				+ "," + 
				T3::name((absolute_index - _start_index) % T3::size());
		}

		bool has_lower_bound() const override { return (bool)_lower_bound; }
		bool has_upper_bound() const override { return (bool)_upper_bound; }

		double get_lower_bound(size_t absolute_index) const override { return _lower_bound(T1((absolute_index - _start_index) / T2::size())); }
		double get_upper_bound(size_t absolute_index) const override { return _upper_bound(T1((absolute_index - _start_index) / T2::size())); }

		size_t size() const override { return T1::size() * T2::size() * T3::size(); }

		lower_bound<false, T1> _lower_bound;
		upper_bound<false, T1> _upper_bound;
	public :
		const int arity = 3;
		typedef T1 index_type_1;
		typedef T2 index_type_2;
		typedef T3 index_type_3;
		typedef double value_type;

		expression operator()(T1 i, T2 j, T3 k)
		{
			return expressions::variable{ _start_index,  i.raw_index()*T2::size()*T3::size() + j.raw_index()*T3::size() + k.raw_index() };
		}

		var(const lower_bound<false, T1>&lower, const upper_bound<false, T1>&upper) :
			_lower_bound(lower), _upper_bound(upper) {
			init();
		}

		var(const lower_bound<false, T1>&lower) :
			_lower_bound(lower) {
			init();
		}

	};

	template<typename T1, typename T2>
	class var<T1, T2,void> : public variable_set
	{

		std::string name(size_t absolute_index) const override
		{
			return T1::name((absolute_index - _start_index)/T2::size()) + "," + T2::name((absolute_index - _start_index) % T2::size());
		}


		lower_bound<false, T1, T2> _lower_bound;
		upper_bound<false, T1, T2> _upper_bound;

		bool has_lower_bound() const override { return (bool)_lower_bound; }
		bool has_upper_bound() const override { return (bool)_upper_bound; }

		double get_lower_bound(size_t absolute_index) const override { return _lower_bound(T1((absolute_index - _start_index)/T2::size()), T2((absolute_index - _start_index) % T2::size())); }
		double get_upper_bound(size_t absolute_index) const override { return _upper_bound(T1((absolute_index - _start_index)/T2::size()), T2((absolute_index - _start_index) % T2::size())); }

	public:
		const int arity = 2;
		typedef T1 index_type_1;
		typedef T2 index_type_2;
		typedef double value_type;

		size_t size() const override { return T1::size() * T2::size(); }

		var() { init(); }


		/*
		var(const upper_bound<false, T>&upper) { init(); }
		var(const lower_bound<false, T>&lower) { init(); }
		var(const upper_bound<false, T>&upper, const lower_bound<false, T>&lower) { init(); }
		var(const lower_bound<false, T>&lower, const upper_bound<false, T>&upper) { init(); }
		*/

		var(const lower_bound<false, T1, T2>&lower, const upper_bound<false, T1, T2>&upper) :
			_lower_bound(lower), _upper_bound(upper) {
			init();
		}

		var(const lower_bound<false, T1, T2>&lower) :
			_lower_bound(lower) {
			init();
		}

		expression operator()(T1 i, T2 j)
		{
			return expressions::variable{ _start_index,  i.raw_index()*T2::size() + j.raw_index() };
		}
	};

	template<typename T>
	class var<T, void, void>: public variable_set
	{
		std::string name(size_t absolute_index) const override
		{
			return T::name(absolute_index - _start_index);
		}

		lower_bound<false, T> _lower_bound;
		upper_bound<false, T> _upper_bound;

		bool has_lower_bound() const override { return (bool)_lower_bound; }
		bool has_upper_bound() const override { return (bool)_upper_bound; }

		double get_lower_bound(size_t absolute_index) const override { return _lower_bound(T(absolute_index - _start_index)); }
		double get_upper_bound(size_t absolute_index) const override { return _upper_bound(T(absolute_index - _start_index)); }

	public:
		const int arity = 1;
		typedef T index_type;
		typedef double value_type;

		size_t size() const override { return T::size(); }
		var() { init(); }


		/*
		var(const upper_bound<false, T>&upper) { init(); }
		var(const lower_bound<false, T>&lower) { init(); }
		var(const upper_bound<false, T>&upper, const lower_bound<false, T>&lower) { init(); }
		var(const lower_bound<false, T>&lower, const upper_bound<false, T>&upper) { init(); }
		*/

		var(const lower_bound<false, T>&lower, const upper_bound<false, T>&upper) :
			_lower_bound(lower), _upper_bound(upper) { init(); }

		expression operator()(T i) 
		{ 
			return expressions::variable{ _start_index,  i.raw_index() };
		}
	};


	template<>
	struct var<void>
	{

	};

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
