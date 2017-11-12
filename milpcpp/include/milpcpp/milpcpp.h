#ifndef __MILPCPP_MILPCPP_H__
#define __MILPCPP_MILPCPP_H__

#include<algorithm>
#include<functional>
#include<string>

#include<milpcpp/bounds.h>
#include<milpcpp/indexing.h>
#include<milpcpp/utils.h>

namespace milpcpp
{
	struct variable_set
	{
		size_t _start_index = -100;
		void set_start_index(size_t index) { _start_index = index; }
		size_t start_index() const { return  _start_index; }
		void init();

		virtual size_t size() const = 0;
		virtual std::string name(size_t absolute_index) const = 0;
		virtual bool has_lower_bound() const = 0;
		virtual bool has_upper_bound() const = 0;
		virtual double get_lower_bound(size_t absolute_index) const = 0;
		virtual double get_upper_bound(size_t absolute_index) const = 0;
	};

	class model
	{
		friend class glpk;
		friend class lp_solve;

		std::vector<variable_set*> _variable_sets;
		std::vector<size_t> _cumulative_sizes;

		expression _objective;
		bool _minimize;

		std::vector<constraint> _constraints;

		void index_variable_sets()
		{
			size_t previous_size = 0;
			for (auto&set : _variable_sets)
			{
				size_t current_size = previous_size + set->size();
				_cumulative_sizes.push_back(current_size);
				set->set_start_index(previous_size);
				previous_size = current_size;
			}
		}

		size_t variable_set_from_absolute_index(size_t absolute_index) const
		{
			auto it = std::upper_bound(_cumulative_sizes.begin(), _cumulative_sizes.end(), absolute_index);
			return it - _cumulative_sizes.begin();
		}

		static model * _context;
	public:
		model() { _context = this;  }
		void seal_data() { index_variable_sets(); }
		static void add_variable_set(variable_set * var_set)
		{
			_context->_variable_sets.push_back(var_set);
		}

		static void set_maximixe() { _context->_minimize = false; }
		static void set_minimize() { _context->_minimize = true; }

		static void set_objective(const expression& e) { _context->_objective = e; }
		static void add_constraints(const std::vector<constraint>&c) 
		{ 
			_context->_constraints.insert(_context->_constraints.end(), c.begin(), c.end());
		}

		std::string variable_name(size_t absolute_index) const
		{
			size_t var_set_index = variable_set_from_absolute_index(absolute_index);
			return _variable_sets[var_set_index]->name(absolute_index);
		}

		bool has_lower_bound(size_t absolute_index) const
		{
			size_t var_set_index = variable_set_from_absolute_index(absolute_index);
			return _variable_sets[var_set_index]->has_lower_bound();
		}

		bool has_upper_bound(size_t absolute_index) const
		{
			size_t var_set_index = variable_set_from_absolute_index(absolute_index);
			return _variable_sets[var_set_index]->has_upper_bound();
		}

		double get_lower_bound(size_t absolute_index) const
		{
			size_t var_set_index = variable_set_from_absolute_index(absolute_index);
			return _variable_sets[var_set_index]->get_lower_bound(absolute_index);
		}

		double get_upper_bound(size_t absolute_index) const
		{
			size_t var_set_index = variable_set_from_absolute_index(absolute_index);
			return _variable_sets[var_set_index]->get_upper_bound(absolute_index);
		}

		size_t variable_count() const { return _cumulative_sizes.back(); }

	};

	inline void variable_set::init() { model::add_variable_set(this); }

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

	template<typename F, int Arity>
	struct sum_internal 
	{
	};

	template<typename F>
	struct sum_internal<F, 3>
	{
		F _f;

		sum_internal(const F&f) :_f(f) {}

		expression operator()()
		{
			typedef utils::function_traits<F> traits;
			typedef traits::arg<0>::type T1;
			typedef traits::arg<1>::type T2;
			typedef traits::arg<2>::type T3;

			expressions::sum result;
			size_t size = T1::size();
			size_t size2 = T2::size();
			size_t size3 = T3::size();
			for (size_t i = 0; i < size; ++i)
				for (size_t j = 0; j < size2; ++j)
					for (size_t k = 0; k < size3; ++k)
						add(result, _f(T1(i), T2(j), T3(k)));
			return result;
		}

	};

	template<typename F>
	struct sum_internal<F,2>
	{
		F _f;

		sum_internal(const F&f) :_f(f) {}

		expression operator()()
		{
			typedef utils::function_traits<F> traits;
			typedef traits::arg<0>::type T1;
			typedef traits::arg<1>::type T2;

			expressions::sum result;
			size_t size = T1::size();
			size_t size2 = T2::size();
			for (size_t i = 0; i < size; ++i)
			{
				for (size_t j = 0; j < size2; ++j)
					add(result, _f(T1(i), T2(j)));
			}
			return result;
		}

	};


	template<typename F>
	struct sum_internal<F, 1>
	{
		F _f;

		sum_internal(const F&f) :_f(f) {}

		expression operator()()
		{
			typedef utils::function_traits<F> traits;
			typedef traits::arg<0>::type T;
			expressions::sum result;
			size_t size = T::size();
			for (size_t i = 0; i < size; ++i)
			{
				add(result, _f(T(i)));
			}
			return result;
		}		
	};

	template<typename F>
	expression sum(F f)
	{
		typedef utils::function_traits<F> traits;
		sum_internal<F, traits::arity> summer(f);
		return summer();

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

	template<typename F, int Arity>
	class subject_to_internal{};

	template<typename F>
	struct subject_to_internal<F, 2>
	{
		F _f;

		subject_to_internal(const F&f) :_f(f) {}

		std::vector<constraint> operator()()
		{
			typedef utils::function_traits<F> traits;
			typedef traits::arg<0>::type T1;
			typedef traits::arg<1>::type T2;

			std::vector<constraint> result;
			size_t size = T1::size();
			size_t size2 = T2::size();
			for (size_t i = 0; i < size; ++i)
			{
				for (size_t j = 0; j < size2; ++j)
					result.push_back(_f(T1(i),T2(j)));
			}
			return result;
		}
	};

	template<typename F>
	struct subject_to_internal<F, 1>
	{
		F _f;

		subject_to_internal(const F&f) :_f(f) {}

		std::vector<constraint> operator()()
		{
			typedef utils::function_traits<F> traits;
			typedef traits::arg<0>::type T;

			std::vector<constraint> result;
			size_t size = T::size();
			for (size_t i = 0; i < size; ++i)
			{
				result.push_back(_f(T(i)));
			}
			return result;
		}
	};

	template<typename F>
	struct subject_to_internal<F, 0>
	{
		F _f;

		subject_to_internal(const F&f) :_f(f) {}

		std::vector<constraint> operator()()
		{
			std::vector<constraint> result{ _f() };
			return result;
		}
	};

	template<typename F>
	inline void subject_to(const char * name, const F&f)
	{
		typedef utils::function_traits<F> traits;
		subject_to_internal<F, traits::arity> func(f);
		std::vector<constraint> constraints = func();
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
