#ifndef __MILPCPP_PARAM_H__
#define __MILPCPP_PARAM_H__

#include<vector>

#include<milpcpp/bounds.h>

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

	template<typename T1 = void, typename T2 = void, typename T3 = void>
	class param : public indexed_param
	{
	public:
		param() = default;
		param(const upper_bound<true, T1>&upper) {}
		param(const lower_bound<true, T1>&lower) {}
		param(const upper_bound<false, T1>&upper) {}
		param(const lower_bound<false, T1>&lower) {}

		expression operator()(T1 i, T2 j, T3 k)
		{
			if (_values.empty())
				return expressions::constant{ _default };
			return expressions::constant{ _values[i.raw_index()*T2::size()*T3::size() + j.raw_index()*T3::size() + k.raw_index()] };
		}

		void add(const std::string&index1, const std::string&index2,const std::string&index3, double value)
		{
			if (_values.empty())
			{
				_values.resize(T1::size()*T2::size()*T3::size());
			}
			_values[T1::index_of(index1)*T2::size()*T3::size() + T2::index_of(index2)*T3::size() + T3::index_of(index3)] = value;
		}
	};

	template<typename T1, typename T2>
	class param<T1, T2, void>: public indexed_param
	{
	public:
		param() = default;
		param(const upper_bound<true, T1>&upper) {}
		param(const lower_bound<true, T1>&lower) {}
		param(const upper_bound<false, T1>&upper) {}
		param(const lower_bound<false, T1>&lower) {}

		expression operator()(T1 i, T2 j)
		{
			if (_values.empty())
				return expressions::constant{ _default };
			return expressions::constant{ _values[i.raw_index()*T2::size() + j.raw_index()] };
		}

		void add(const std::string&index1, const std::string&index2, double value)
		{
			if (_values.empty())
			{
				_values.resize(T1::size()*T2::size());
			}
			_values[T1::index_of(index1)*T2::size() + T2::index_of(index2)] = value;
		}

		void add(const std::string&index1, long index2, double value)
		{
			if (_values.empty())
			{
				_values.resize(T1::size()*T2::size());
			}
			_values[T1::index_of(index1)*T2::size() + T2::index_of(index2)] = value;
		}

	};

	template<typename T>
	class param<T,void,void> : public indexed_param
	{
	public:
		param() = default;
		param(const upper_bound<true, T>&upper) {}
		param(const lower_bound<true, T>&lower) {}
		param(const upper_bound<false, T>&upper) {}
		param(const lower_bound<false, T>&lower) {}

 		expression operator()(T i) 
		{ 
			if (_values.empty())
				return expressions::constant{ _default };
			return expressions::constant{ _values[i.raw_index()] };
		}

		void add(const std::string&index, double value) 
		{
			if (_values.empty())
			{
				_values.resize(T::size());
			}
			_values[T::index_of(index)] = value;
		}

		void add(long index, double value)
		{
			if (_values.empty())
			{
				_values.resize(T::size());
			}
			_values[T::index_of(index)] = value;
		}
	};

	template<>
	struct param<void, void, void>
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
