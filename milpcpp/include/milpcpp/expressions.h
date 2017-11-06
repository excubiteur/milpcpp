#ifndef __MILPCPP_EXPRESSIONS_H__
#define __MILPCPP_EXPRESSIONS_H__

#include<exception>
#include<map>
#include<string>
#include<variant>

namespace milpcpp
{
	namespace expressions
	{
		struct variable
		{
			size_t _start_index;
			size_t _offset_index;
			size_t absolute_index() const { return _start_index + _offset_index;  }
		};

		struct constant
		{
			double _value = 0;
		};

		struct term
		{
			variable _variable;
			constant _coefficient;
		};


		struct sum
		{
			std::map<size_t, term> _terms;
			constant _constant_term;
		};

		struct nonlinear_expression : std::logic_error
		{
			nonlinear_expression(const std::string&what) : std::logic_error(what) {}
		};
	};

	typedef std::variant<expressions::constant, expressions::variable, expressions::term, expressions::sum> expression;

	inline expression multiply(const expressions::constant&e1, const expressions::constant&e2)
	{
		return expressions::constant{ e1._value + e2._value };
	}

	inline expression multiply(const expressions::constant&e1, const expressions::variable&e2)
	{
		return expressions::term{ e2, e1};
	}

	inline expression multiply(const expressions::constant&e1, const expressions::term&e2)
	{
		return expressions::term{ e2._variable, e1._value * e2._coefficient._value };
	}

	inline expression multiply(const expressions::constant&e1, const expressions::sum&e2)
	{
		expressions::sum result;
		for (const auto & term : e2._terms)
		{
			result._terms[term.first] = expressions::term{ term.second._variable, e1._value * term.second._coefficient._value };
		}
		return result;
	}

	inline expression multiply(const expressions::constant&c, const expression&other)
	{
		if (std::holds_alternative<expressions::constant>(other))
		{
			return multiply(c, std::get<expressions::constant>(other));
		}
		else if (std::holds_alternative<expressions::variable>(other))
		{
			return multiply(c, std::get<expressions::variable>(other));
		}
		else if (std::holds_alternative<expressions::term>(other))
		{
			return multiply(c, std::get<expressions::term>(other));
		}
		else if (std::holds_alternative<expressions::sum>(other))
		{
			return multiply(c, std::get<expressions::sum>(other));
		}
		else
		{
			throw; //implement later
		}
	}

	inline expression operator*(const expression&e1, const expression&e2)
	{
		if (std::holds_alternative<expressions::constant>(e1) )
		{
			return multiply(std::get<expressions::constant>(e1), e2);
		}
		else if (std::holds_alternative<expressions::constant>(e2))
		{
			return multiply(std::get<expressions::constant>(e2), e1);
		}
		else
		{
			throw expressions::nonlinear_expression("Linear expressions only");
		}
	}

	inline expression operator/(double e1, const expression&e2)
	{
		if (std::holds_alternative<expressions::constant>(e2))
		{
			return expressions::constant{ e1 / std::get<expressions::constant>(e2)._value };
		}
		else
		{
			throw expressions::nonlinear_expression("Linear expressions only");
		}
	}

	inline expression operator/(const expression&e1, const expression&e2)
	{
		if (std::holds_alternative<expressions::constant>(e1) && std::holds_alternative<expressions::constant>(e2))
		{
			return expressions::constant{ std::get<expressions::constant>(e2)._value / std::get<expressions::constant>(e2)._value };
		}
		else
		{
			throw expressions::nonlinear_expression("Linear expressions only");
		}
	}

	inline void add(expressions::sum& sum, const expression&e)
	{
		if (std::holds_alternative<expressions::constant>(e))
		{
			const auto & new_term = std::get<expressions::constant>(e);
			sum._constant_term._value += new_term._value;
		}
		else if (std::holds_alternative<expressions::term>(e))
		{
			const auto & new_term = std::get<expressions::term>(e);
			auto&term = sum._terms.find(new_term._variable.absolute_index());
			if (term == sum._terms.end())
			{
				sum._terms[new_term._variable.absolute_index()] = new_term;
			}
			else
			{
				term->second._coefficient._value += new_term._coefficient._value;
			}
		}
		else if (std::holds_alternative<expressions::variable>(e))
		{
			const auto & var = std::get<expressions::variable>(e);
			expressions::term new_term{ {var}, {1} };
			auto&term = sum._terms.find(new_term._variable.absolute_index());
			if (term == sum._terms.end())
			{
				sum._terms[new_term._variable.absolute_index()] = new_term;
			}
			else
			{
				term->second._coefficient._value += new_term._coefficient._value;
			}
		}
		else if (std::holds_alternative<expressions::sum>(e))
		{
			const auto & other = std::get<expressions::sum>(e);
			sum._constant_term._value += other._constant_term._value;
			for (const auto&term : other._terms)
			{
				add(sum, term.second);
			}
		}
		else
		{
			throw; //implement later
		}

	}

	inline void subtract(expressions::sum& sum, const expression&e)
	{
		if (std::holds_alternative<expressions::constant>(e))
		{
			const auto & new_term = std::get<expressions::constant>(e);
			sum._constant_term._value -= new_term._value;
		}
		else if (std::holds_alternative<expressions::term>(e))
		{
			const auto & new_term = std::get<expressions::term>(e);
			auto&term = sum._terms.find(new_term._variable.absolute_index());
			if (term == sum._terms.end())
			{
				auto negative = new_term;
				negative._coefficient._value *= -1;
				sum._terms[new_term._variable.absolute_index()] = negative;
			}
			else
			{
				term->second._coefficient._value -= new_term._coefficient._value;
			}
		}
		else if (std::holds_alternative<expressions::variable>(e))
		{
			const auto & var = std::get<expressions::variable>(e);
			expressions::term new_term{ { var },{ 1 } };
			auto&term = sum._terms.find(new_term._variable.absolute_index());
			if (term == sum._terms.end())
			{
				auto negative = new_term;
				negative._coefficient._value *= -1;
				sum._terms[new_term._variable.absolute_index()] = negative;
			}
			else
			{
				term->second._coefficient._value -= new_term._coefficient._value;
			}
		}
		else if (std::holds_alternative<expressions::sum>(e))
		{
			const auto & other = std::get<expressions::sum>(e);
			sum._constant_term._value -= other._constant_term._value;
			for (const auto&term : other._terms)
			{
				subtract(sum, term.second);
			}
		}
		else
		{
			throw; //implement later
		}

	}

	inline expression operator+(const expression&e1, const expression&e2)
	{
		if (std::holds_alternative<expressions::sum>(e1))
		{
			expressions::sum result = std::get<expressions::sum>(e1);
			add(result, e2);
			return result;
		}
		else if (std::holds_alternative<expressions::sum>(e2))
		{
			expressions::sum result = std::get<expressions::sum>(e2);
			add(result, e1);
			return result;
		}
		else
		{
			throw; // to do 
		}
	}

	inline expression operator-(const expression&e1, const expression&e2)
	{
		if (std::holds_alternative<expressions::sum>(e1))
		{
			expressions::sum result = std::get<expressions::sum>(e1);
			subtract(result, e2);
			return result;
		}
		else if (std::holds_alternative<expressions::sum>(e2))
		{
			expressions::sum result = std::get<expressions::sum>(e2);
			subtract(result, e1);
			return result;
		}
		else
		{
			throw; // to do 
		}
	}

	struct constraint
	{
		expression _expression;
		double _lower_bound = 0;
		double _upper_bound = 0;
		bool _lower_bounded = false;
		bool _upper_bounded = false;
	};

	namespace constraints
	{
		inline constraint upper_bound(const expression&e, double value)
		{
			constraint result;
			result._expression = e;
			result._upper_bounded = true;
			result._upper_bound = value;
			return result;
		}

		inline constraint equal(const expression&e, double value)
		{
			constraint result;
			result._expression = e;
			result._upper_bounded = true;
			result._upper_bound = value;
			result._lower_bounded = true;
			result._lower_bound = value;
			return result;
		}

		inline constraint equal(const expressions::sum&sum, const expression&e)
		{
			constraint result;
			result._expression = sum - e;
			result._upper_bounded = true;
			result._upper_bound = 0;
			result._lower_bounded = true;
			result._lower_bound = 0;
			return result;
		}

		inline constraint upper_bound(const constraint&c, double value)
		{
			constraint result = c;
			if (result._upper_bounded)
			{
				if (result._upper_bound > value)
				{
					result._upper_bound = value;
				}
			}
			else
			{
				result._upper_bounded = true;
				result._upper_bound = value;
			}
			return result;
		}

		inline constraint lower_bound(const expression&e, double value)
		{
			constraint result;
			result._expression = e;
			result._lower_bounded = true;
			result._lower_bound = value;
			return result;
		}

	}

	inline constraint operator<=(const expression&e1, const expression&e2)
	{
		if (std::holds_alternative<expressions::constant>(e2))
		{
			return constraints::upper_bound(e1, std::get<expressions::constant>(e2)._value);
		}
		else if (std::holds_alternative<expressions::constant>(e1))
		{
			return constraints::lower_bound(e2, std::get<expressions::constant>(e1)._value);
		}
		else
		{
			throw; // Implement other cases later
		}
	}

	inline constraint operator<=(double value, const expression&e)
	{
		return constraints::lower_bound(e, value);
	}

	inline constraint operator<=(const constraint&e1, const expression&e2)
	{
		if (std::holds_alternative<expressions::constant>(e2))
		{
			return constraints::upper_bound(e1, std::get<expressions::constant>(e2)._value);
		}
		else
		{
			throw; // Implement other cases later
		}
	}

	inline constraint operator==(const expression&e1, const expression&e2)
	{
		if (std::holds_alternative<expressions::constant>(e2))
		{
			return constraints::equal(e1, std::get<expressions::constant>(e2)._value);
		}
		else if (std::holds_alternative<expressions::constant>(e1))
		{
			return constraints::equal(e2, std::get<expressions::constant>(e1)._value);
		}
		else if (std::holds_alternative<expressions::sum>(e1))
		{
			return constraints::equal(std::get<expressions::sum>(e1),e2);
		}
		else if (std::holds_alternative<expressions::sum>(e2))
		{
			return constraints::equal(std::get<expressions::sum>(e2), e1);
		}
		else
		{
			throw; // Implement other cases later
		}
	}
}

#endif
