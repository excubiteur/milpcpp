#ifndef __MILPCPP_TUPLES_H__
#define __MILPCPP_TUPLES_H__

namespace milpcpp
{

	template<typename T1, typename ... Ts>
	struct compound_index
	{
		static size_t size() { return T1::size()*compound_index<Ts...>::size(); }

		static std::string name(size_t index) 
		{ 
			size_t actual_index = index / compound_index<Ts...>::size();
			size_t next_component_index = index - (actual_index * compound_index<Ts...>::size());
			return T1::name(actual_index) + "," + compound_index<Ts...>::name(next_component_index);
		}
	};

	template<typename T>
	struct compound_index<T>
	{
		static size_t size() { return T::size(); }
		static std::string name(size_t index) { return T::name(index); }
	};

	template<typename T1, typename ... Ts>
	size_t get_offset(T1 t1, Ts...args)
	{
		return t1.raw_index()*compound_index<Ts...>::size() + get_offset(args...);
	}

	template<typename T>
	size_t get_offset(T t)
	{
		return t.raw_index();
	}

	template<typename T1, typename ... Ts>
	double invoke(size_t index, const std::function<double(T1, Ts...)>&f)
	{
		size_t actual_index = index / compound_index<Ts...>::size();
		size_t next_component_index = index - (actual_index * compound_index<Ts...>::size());
		auto f2 = [=](Ts...args) { return f(T1(actual_index), args...);  };
		return invoke<Ts...>(next_component_index, f2);
	}

	template<typename T>
	double invoke(size_t index, const std::function<double(T)>&f)
	{
		return f(T(index));
	}

	template<typename T1, typename ... Ts>
	void invoke(size_t index, double value, const std::function<void(double, T1, Ts...)>&f)
	{
		size_t actual_index = index / compound_index<Ts...>::size();
		size_t next_component_index = index - (actual_index * compound_index<Ts...>::size());
		auto f2 = [=](double v, Ts...args) { return f(v, T1(actual_index), args...);  };
		invoke<Ts...>(next_component_index, value, f2);
	}

	template<typename T>
	void invoke(size_t index, double value, const std::function<void(double,T)>&f)
	{
		f(value,T(index));
	}

}

#endif
