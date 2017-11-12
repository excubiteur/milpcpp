#include<milpcpp/lp_solve.h>
#include<milpcpp/milpcpp.h>

#include <lp_lib.h>

#include<variant>

using namespace milpcpp;

lp_solve::lp_solve(model * m) :_model(m)
{
}

lp_solve::~lp_solve()
{
	delete_lp(_lp);
}

void lp_solve::solve()
{
	int var_count = (int)_model->variable_count();
	_lp = make_lp(0, var_count);

	set_add_rowmode(_lp, TRUE);

	for (int i = 1; i <= var_count; ++i)
	{
		set_col_name(_lp, i, const_cast<char*>(_model->variable_name(i - 1).c_str()));
		if (_model->has_lower_bound(i - 1) && _model->has_upper_bound(i - 1))
		{
			set_bounds(_lp, i, 
				_model->get_lower_bound(i - 1),
				_model->get_upper_bound(i - 1));
		}
		else if (_model->has_lower_bound(i - 1))
		{
			set_bounds(_lp, i, 
				_model->get_lower_bound(i - 1),
				get_infinite(_lp));
		}
		else if (_model->has_upper_bound(i - 1))
		{
			set_bounds(_lp, i, 
				-get_infinite(_lp),
				_model->get_upper_bound(i - 1));
		}
		else
		{
			set_unbounded(_lp, i);
		}
	}

	for (const auto & c : _model->_constraints)
	{
		const auto & e = c._expression;

		if (std::holds_alternative<expressions::sum>(e))
		{
			const auto&sum = std::get<expressions::sum>(e);
			double lower = c._lower_bound - sum._constant_term._value;
			double upper = c._upper_bound - sum._constant_term._value;
			int size = (int)sum._terms.size();
			std::vector<int> indices(size);
			std::vector<double> values(size);
			int current_index = 0;
			for (const auto&i : sum._terms)
			{
				indices[current_index] = ((int)(i.first + 1));
				values[current_index] = (i.second._coefficient._value);
				++current_index;
			}
			if(c._lower_bounded)
				add_constraintex(_lp, size, &values[0], &indices[0], GE, lower);
			if(c._upper_bounded)
				add_constraintex(_lp, size, &values[0], &indices[0], LE, upper);

		}
		else if (std::holds_alternative<expressions::term>(e))
		{
			const auto&term = std::get<expressions::term>(e);
			int index = (int)term._variable.absolute_index() + 1;
			double value = term._coefficient._value;
			if (c._lower_bounded)
				add_constraintex(_lp, 1, &value, &index, GE, c._lower_bound);
			if (c._upper_bounded)
				add_constraintex(_lp, 1, &value, &index, LE, c._upper_bound);
		}
		else if (std::holds_alternative<expressions::variable>(e))
		{
			const auto&var = std::get<expressions::variable>(e);
			int index = (int)var.absolute_index() + 1;
			double value = 1;
			if (c._lower_bounded)
				add_constraintex(_lp, 1, &value, &index, GE, c._lower_bound);
			if (c._upper_bounded)
				add_constraintex(_lp, 1, &value, &index, LE, c._upper_bound);
		}
		else
		{

		}
	}

	set_add_rowmode(_lp, FALSE);

	const auto& objective = _model->_objective;
	if (std::holds_alternative<expressions::sum>(objective))
	{
		const auto&sum = std::get<expressions::sum>(objective);
		int size = (int)sum._terms.size();
		std::vector<int> indices(size);
		std::vector<double> values(size);
		int current_index = 0;
		for (const auto&i : sum._terms)
		{
			indices[current_index] = ((int)(i.first + 1));
			values[current_index] = (i.second._coefficient._value);
			++current_index;
		}
		set_obj_fnex(_lp, size, &values[0], &indices[0]);
	}

	if (_model->_minimize)
		set_minim(_lp);
	else
		set_maxim(_lp);

	set_verbose(_lp, CRITICAL);

	if (!::solve(_lp))
	{
		get_ptr_variables(_lp, &_variable_values);
	}

}

double lp_solve::get_objective_value()
{
	return get_objective(_lp);
}
