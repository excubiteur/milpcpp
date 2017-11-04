#ifndef __MILPCPP_BOUNDS_H__
#define __MILPCPP_BOUNDSP_H__

#include<milpcpp/expressions.h>

namespace milpcpp
{

	template<bool _Strict = false, typename T = nullptr_t>
	struct bound: std::function<double(T)>
	{
		bound() = default;

		bound(const std::function<double(T)>&f):
			std::function<double(T)>(f) {}

		bound(const std::function<expression(T)>&f) :
			std::function<double(T)>([=](T t) { 
				return std::get<expressions::constant>(f(t))._value;
			}) {}

		bound(double value): 
			std::function<double(T)>([](T) { return value; }) {}
	};

	template<bool _Strict>
	struct bound<_Strict, nullptr_t> : std::function<double()>
	{
		bound(double value) :
			std::function<double()>([=]() { return value; } ) {}
	};

	template<bool _Strict = false, typename T = nullptr_t>
	struct lower_bound : bound<_Strict, T>
	{
		lower_bound() = default;

		lower_bound(const std::function<double(T)>&f) :
			bound(f) {}

		lower_bound(const std::function<expression(T)>&f) :
			bound(f) {}

		lower_bound(const lower_bound<_Strict, nullptr_t >&f) :
			bound([=](T) {return f();  }) {}

		lower_bound(double value) :
			bound(value) {}
	};

	template<bool _Strict>
	struct lower_bound<_Strict,nullptr_t> : bound<_Strict,nullptr_t>
	{
		lower_bound(double value) :
			bound(value) {}
	};

	template<bool _Strict = false, typename T = nullptr_t>
	struct upper_bound : bound<_Strict, T>
	{
		upper_bound() = default;

		upper_bound(const std::function<double(T)>&f) :
			bound(f) {}

		upper_bound(const std::function<expression(T)>&f) :
			bound(f) {}

		upper_bound(const upper_bound<_Strict, nullptr_t >&f) :
			bound([=](T) {return f();  }) {}

		upper_bound(double value) :
			bound(value) {}


	};

	template<bool _Strict>
	struct upper_bound<_Strict, nullptr_t> : bound<_Strict, nullptr_t>
	{
		upper_bound(double value) :
			bound(value) {}
	};

	inline lower_bound<true> greater_than(double bound)
	{
		return lower_bound<true>(bound);
	}

	inline upper_bound<true> less_than(double bound)
	{
		return upper_bound<true>(bound);
	}

	template<typename T>
	inline auto less_equal(T bound)
	{
		typedef utils::function_traits<T> traits;
		typedef traits::arg<0>::type index_type;
		return upper_bound<false, index_type >(bound);
	}

	template<>
	inline auto less_equal<double>(double bound)
	{
		return upper_bound<>(bound);
	}

	template<>
	inline auto less_equal<long>(long bound)
	{
		return upper_bound<>(bound);
	}

	template<typename T>
	inline auto greater_equal(T bound)
	{
		typedef utils::function_traits<T> traits;
		typedef traits::arg<0>::type index_type;
		return lower_bound<false, index_type >(bound);
	}

	template<>
	inline auto greater_equal<double>(double bound)
	{
		return lower_bound<>(bound);
	}

	template<>
	inline auto greater_equal<int>(int bound)
	{
		return lower_bound<>(bound);
	}

}

#endif
