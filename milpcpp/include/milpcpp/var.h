#ifndef __MILPCPP_VAR_H__
#define __MILPCPP_VAR_H__

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

	template<typename T1 = void, typename T2 = void, typename T3 = void>
	class var : public variable_set
	{
		std::string name(size_t absolute_index) const override
		{
			return compound_index<T1, T2, T3>::name(absolute_index - _start_index);
		}

		bool has_lower_bound() const override { return (bool)_lower_bound; }
		bool has_upper_bound() const override { return (bool)_upper_bound; }

		double get_lower_bound(size_t absolute_index) const override { return invoke(absolute_index - _start_index, _lower_bound); }
		double get_upper_bound(size_t absolute_index) const override { return invoke(absolute_index - _start_index, _upper_bound); }

		size_t size() const override { return compound_index<T1,T2,T3>::size(); }

		lower_bound<false, T1,T2,T3> _lower_bound;
		upper_bound<false, T1,T2,T3> _upper_bound;
	public :
		typedef T1 index_type_1;
		typedef T2 index_type_2;
		typedef T3 index_type_3;
		typedef double value_type;

		expression operator()(T1 i, T2 j, T3 k)
		{
			return expressions::variable{ _start_index, get_offset(i,j,k) };

		}

		var(const lower_bound<false, T1,T2,T3>&lower, const upper_bound<false, T1,T2,T3>&upper) :
			_lower_bound(lower), _upper_bound(upper) {
			init();
		}

		var(const lower_bound<false, T1,T2,T3>&lower) :
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

}

#endif
