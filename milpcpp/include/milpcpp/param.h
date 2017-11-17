#ifndef __MILPCPP_PARAM_H__
#define __MILPCPP_PARAM_H__

#include<vector>

#include<milpcpp/bounds.h>
#include<milpcpp/tuples.h>

namespace milpcpp
{
	class indexed_param
	{
	protected:
		double _default{};
		std::vector<double> _values;
	public:
		void set_default(double d) { _default = d;  }
	};

	template<typename T1 = void, typename ... Ts>
	class param : public indexed_param
	{
	public:
		typedef typename param<Ts...>::list_element_t list_element_t2;
		typedef std::initializer_list<list_element_t2> list_element_t;

		static void initialize_values(std::vector<double>&values, std::initializer_list<list_element_t> list)
		{
			for (const auto&l : list)
			{
				param<Ts...>::initialize_values(values, l);
			}
		}

		param(std::initializer_list<list_element_t> list)
		{
			initialize_values(_values, list);
		}

		param() = default;
		param(const upper_bound<true,T1,  Ts...>&upper) {}
		param(const lower_bound<true, T1, Ts...>&lower) {}
		param(const upper_bound<false, T1, Ts...>&upper) {}
		param(const lower_bound<false, T1, Ts...>&lower) {}

		expression operator()(T1 arg1, Ts...args)
		{
			if (_values.empty())
				return expressions::constant{ _default };
			return expressions::constant{ _values[get_offset(arg1, args...)] };
		}

		void add(const typename T1::lookup_type&arg1, const typename Ts::lookup_type&...args , double value)
		{
			if (_values.empty())
			{
				_values.resize(compound_index<T1, Ts...>::size());
			}
			_values[get_offset_by_lookup<T1, Ts...>(arg1, args...)] = value;
		}
	};

	template<typename T>
	class param<T> : public indexed_param
	{
	public:
		typedef double list_element_t;
		
		static void initialize_values(std::vector<double>&values, std::initializer_list<list_element_t> list)
		{
			values.insert(values.end(), list.begin(), list.end());
		}

		param(std::initializer_list<list_element_t> list)
		{
			initialize_values(_values, list);
		}

		param() = default;
		param(const upper_bound<true, T>&upper) {}
		param(const lower_bound<true, T>&lower) {}
		param(const upper_bound<false, T>&upper) {}
		param(const lower_bound<false, T>&lower) {}

		expression operator()(T arg)
		{
			if (_values.empty())
				return expressions::constant{ _default };
			return expressions::constant{ _values[get_offset(arg)] };
		}

		void add(const typename T::lookup_type&arg, double value)
		{
			if (_values.empty())
			{
				_values.resize(compound_index<T>::size());
			}
			_values[get_offset_by_lookup<T>(arg)] = value;
		}
	};

	template<>
	struct param<void>
	{		
		double _value;
	public:
		param() = default;
		param(const upper_bound<true>&upper) {}
		param(const lower_bound<true>&lower) {}
		param(const upper_bound<false>&upper) {}
		param(const lower_bound<false>&lower) {}

		operator expression() const 
		{ 
			return expressions::constant{ _value };
		}
		double operator=(double value) { return _value = value;  }
	};


}


#endif
