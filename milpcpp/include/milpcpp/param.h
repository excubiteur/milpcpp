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

	template<typename ... Ts>
	class param : public indexed_param
	{
	public:
		param() = default;
		param(const upper_bound<true, Ts...>&upper) {}
		param(const lower_bound<true, Ts...>&lower) {}
		param(const upper_bound<false, Ts...>&upper) {}
		param(const lower_bound<false, Ts...>&lower) {}

		expression operator()(Ts...args)
		{
			if (_values.empty())
				return expressions::constant{ _default };
			return expressions::constant{ _values[get_offset(args...)] };
		}

		void add(const typename Ts::lookup_type&...args , double value)
		{
			if (_values.empty())
			{
				_values.resize(compound_index<Ts...>::size());
			}
			_values[get_offset_by_lookup<Ts...>(args...)] = value;
		}
	};


	template<>
	struct param<>
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
