#ifndef __MILPCPP_BOUNDS_H__
#define __MILPCPP_BOUNDSP_H__

#include<milpcpp/expressions.h>

namespace milpcpp
{
	template<bool _Strict = false, typename T1 = void, typename T2 = void>
	struct bound : std::function<double(T1, T2)>
	{
		bound() = default;

		bound(const std::function<expression(T1, T2)>&f) :
			std::function<double(T1,T2)>([=](T1 t1, T2 t2) {
			return std::get<expressions::constant>(f(t1, t2))._value;
		}) {}

		bound(const std::function<double(T1, T2)>&f) :
			std::function<double(T1, T2)>(f) {}
	};

	template<bool _Strict, typename T>
	struct bound<_Strict, T, void>: std::function<double(T)>
	{
		bound() = default;

		bound(const std::function<double(T)>&f):
			std::function<double(T)>(f) {}

		bound(const std::function<expression(T)>&f) :
			std::function<double(T)>([=](T t) { 
				return std::get<expressions::constant>(f(t))._value;
			}) {}

		bound(double value): 
			std::function<double(T)>([=](T) { return value; }) {}
	};

	template<bool _Strict>
	struct bound<_Strict, void, void> : std::function<double()>
	{
		bound(double value) :
			std::function<double()>([=]() { return value; } ) {}
	};

	template<bool _Strict = false, typename T1 = void, typename T2 = void>
	struct lower_bound : bound<_Strict, T1, T2>
	{
		lower_bound(double value) :
			bound(value) {}
		lower_bound(const lower_bound<_Strict >&f) :
			bound([=](T1, T2) {return f();  }) {	}

	};

	template<bool _Strict, typename T>
	struct lower_bound<_Strict, T> : bound<_Strict, T>
	{
		lower_bound() = default;

		lower_bound(const std::function<double(T)>&f) :
			bound(f) {}

		lower_bound(const std::function<expression(T)>&f) :
			bound(f) {}

		lower_bound(const lower_bound<_Strict, void >&f) :
			bound([=](T) {return f();  }) {}

		lower_bound(double value) :
			bound(value) {}
	};

	template<bool _Strict>
	struct lower_bound<_Strict,void> : bound<_Strict,void>
	{
		lower_bound(double value) :
			bound(value) {}
	};

	template<bool _Strict = false, typename T1 = void, typename T2 = void>
	struct upper_bound : bound<_Strict, T1, T2>
	{
		upper_bound() = default;

		upper_bound(const std::function<expression(T1, T2)>&f) :
			bound(f) {}

	};

	template<bool _Strict, typename T>
	struct upper_bound <_Strict, T>: bound<_Strict, T>
	{
		upper_bound() = default;

		upper_bound(const std::function<double(T)>&f) :
			bound(f) {}

		upper_bound(const std::function<expression(T)>&f) :
			bound(f) {}

		upper_bound(const upper_bound<_Strict >&f) :
			bound([=](T) {return f();  }) {}

		upper_bound(double value) :
			bound(value) {}


	};

	template<bool _Strict>
	struct upper_bound<_Strict, void> : bound<_Strict, void>
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
	auto less_equal_internal(std::integral_constant<int, 2>,  T bound)
	{
		typedef utils::function_traits<T> traits;
		typedef traits::arg<0>::type index_type1;
		typedef traits::arg<1>::type index_type2;
		return upper_bound<false, index_type1, index_type2 >(bound);
	}

	template<typename T>
	auto less_equal_internal(std::integral_constant<int, 1>, T bound)
	{
		typedef utils::function_traits<T> traits;
		typedef traits::arg<0>::type index_type;
		return upper_bound<false, index_type >(bound);
	}

	template<typename T>
	inline auto less_equal(T bound)
	{
		typedef utils::function_traits<T> traits;
		constexpr int arity = traits::arity;
		return less_equal_internal(std::integral_constant<int, arity>(), bound);
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
