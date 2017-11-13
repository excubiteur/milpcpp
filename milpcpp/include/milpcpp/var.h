#ifndef __MILPCPP_VAR_H__
#define __MILPCPP_VAR_H__

#include<milpcpp/tuples.h>

namespace milpcpp
{

	template<typename ... Ts>
	class var : public variable_set
	{
		std::string name(size_t absolute_index) const override
		{
			return compound_index<Ts...>::name(absolute_index - _start_index);
		}

		bool has_lower_bound() const override { return (bool)_lower_bound; }
		bool has_upper_bound() const override { return (bool)_upper_bound; }

		double get_lower_bound(size_t absolute_index) const override { return invoke(absolute_index - _start_index, _lower_bound); }
		double get_upper_bound(size_t absolute_index) const override { return invoke(absolute_index - _start_index, _upper_bound); }

		lower_bound<false, Ts...> _lower_bound;
		upper_bound<false, Ts...> _upper_bound;
	public :
		var() { init();  }

		size_t size() const override { return compound_index<Ts...>::size(); }

		typedef std::function<void(double, Ts...)> value_iterator_t;

		expression operator()(Ts...args)
		{
			return expressions::variable{ _start_index, get_offset(args...) };

		}

		var(const lower_bound<false, Ts...>&lower, const upper_bound<false, Ts...>&upper) :
			_lower_bound(lower), _upper_bound(upper) {
			init();
		}

		var(const lower_bound<false, Ts...>&lower) :
			_lower_bound(lower) {
			init();
		}

	};

	template<>
	struct var<>
	{

	};

}

#endif
