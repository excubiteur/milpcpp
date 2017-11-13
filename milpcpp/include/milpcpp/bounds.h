#ifndef __MILPCPP_BOUNDS_H__
#define __MILPCPP_BOUNDS_H__

#include<milpcpp/expressions.h>

namespace milpcpp
{
	template<bool _Strict = false, typename ... Ts>
	struct bound : std::function<double(Ts...)>
	{
		bound() = default;

		bound(const std::function<expression(Ts...)>&f) :
			std::function<double(Ts...)>([=](Ts...args) {
			return std::get<expressions::constant>(f(args...))._value;
		}) {}

		bound(const std::function<double(Ts...)>&f) :
			std::function<double(Ts...)>(f) {}

		bound(double value) :
			std::function<double(Ts...)>([=](Ts...) { return value; }) {}
	};


	template<bool _Strict = false, typename ... Ts>
	struct lower_bound : bound<_Strict, Ts...>
	{
		lower_bound() = default;

		lower_bound(double value) :
			bound(value) {}
		lower_bound(const lower_bound<_Strict>&f) :
			bound([=](Ts...) {return f();  }) {	}
		lower_bound(const std::function<expression(Ts...)>&f) :
			bound(f) {}
	};

	template<bool _Strict>
	struct lower_bound<_Strict> : bound<_Strict>
	{
		lower_bound(double value) :
			bound(value) {}
	};


	template<bool _Strict = false, typename ... Ts>
	struct upper_bound : bound<_Strict, Ts...>
	{
		upper_bound() = default;

		upper_bound(const std::function<expression(Ts...)>&f) :
			bound(f) {}

	};

	template<bool _Strict>
	struct upper_bound<_Strict> : bound<_Strict>
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

	template<typename...Ts>
	auto less_equal_internal(const std::function<expression(Ts...)>& f)
	{
		return upper_bound<false, Ts... >(f);
	}

	template<typename T>
	inline auto less_equal(T f)
	{
		typedef utils::function_traits<T> traits;
		typedef traits::function_type F;
		F func(f);
		return less_equal_internal(func);
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
		return lower_bound<false>(bound);
	}

	template<>
	inline auto greater_equal<long>(long bound)
	{
		return lower_bound<false>(bound);
	}

	template<>
	inline auto greater_equal<int>(int bound)
	{
		return lower_bound<false>(bound);
	}
}

#endif
