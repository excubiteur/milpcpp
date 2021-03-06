#include<milpcpp/glpk.h>
#include<milpcpp/milpcpp.h>

#include <milpcpp/enumerate.h>

#include <glpk.h>

#include<variant>

using namespace milpcpp;

glpk::glpk(model * m) :_model(m)
{
}

glpk::~glpk()
{
	glp_delete_prob(_lp);
}

double glpk::get_variable_value(size_t absolute_index)
{
	return glp_get_col_prim(_lp, (int) absolute_index + 1);
}


void glpk::solve()
{
	_lp = glp_create_prob();
	int var_count = (int)_model->variable_count();

	glp_add_cols(_lp, var_count);

	for (int i = 1; i <= var_count; ++i)
	{
		glp_set_col_kind(_lp, i, GLP_CV);
		glp_set_col_name(_lp, i, _model->variable_name(i - 1).c_str());
		if (_model->has_lower_bound(i-1) && _model->has_upper_bound(i-1))
		{
			glp_set_col_bnds(_lp, i, GLP_DB, 
				_model->get_lower_bound(i-1), 
				_model->get_upper_bound(i-1));
		}
		else if(_model->has_lower_bound(i-1))
		{
			glp_set_col_bnds(_lp, i, GLP_LO,
				_model->get_lower_bound(i-1),
				0);
		}
		else if (_model->has_upper_bound(i-1))
		{
			glp_set_col_bnds(_lp, i, GLP_UP,
				0,
				_model->get_upper_bound(i-1));
		}
		else
		{
			glp_set_col_bnds(_lp, i, GLP_FR, 0, 0);
		}

	}

	glp_add_rows(_lp, (int)_model->_constraints.size());

	int current_row = 0;
	for (const auto & c : _model->_constraints)
	{
		++current_row;

		int type;
		if (c._lower_bound.has_value() && c._upper_bound.has_value())
		{
			if(c._lower_bound >= c._upper_bound)
				type = GLP_FX;
			else
				type = GLP_DB;
		}
		else if (c._upper_bound.has_value())
		{
			type = GLP_UP;
		}
		else if (c._lower_bound.has_value())
		{
			type = GLP_LO;
		}
		else
		{
			type = GLP_FR;
		}

		const auto & e = c._expression;

		if (std::holds_alternative<expressions::sum>(e))
		{
			const auto&sum = std::get<expressions::sum>(e);
			double lower = c._lower_bound.value_or(0) - sum._constant_term._value;
			double upper = c._upper_bound.value_or(0) - sum._constant_term._value;
			int size = (int)sum._terms.size();
			std::vector<int> indices(size + 1);
			std::vector<double> values(size + 1);

			auto terms = // range/iterator of (variable index, coeffient) pairs
				sum._terms | 
				ranges::view::transform([](auto e) { 
					return std::make_pair(e.first, e.second._coefficient._value);
				});

			for (const auto&[current_index, term ] : utils::enumerate(terms))
			{
				const auto&[index, value] = term;
				indices[current_index+1] = (int)index + 1;
				values[current_index+1] = value;
			}
			glp_set_row_bnds(_lp, current_row , type, lower, upper);
			glp_set_mat_row(_lp, current_row, size, &indices[0], &values[0]);

		}
		else if (std::holds_alternative<expressions::term>(e))
		{
			const auto&term = std::get<expressions::term>(e);
			std::vector<int> indices(2);
			std::vector<double> values(2);
			indices[1] = (int)term._variable.absolute_index() + 1;
			values[1] = term._coefficient._value;
			glp_set_row_bnds(_lp, current_row, type, c._lower_bound.value(), c._upper_bound.value());
			glp_set_mat_row(_lp, current_row, 1, &indices[0], &values[0]);
		}
		else if (std::holds_alternative<expressions::variable>(e))
		{
			const auto&var = std::get<expressions::variable>(e);
			std::vector<int> indices(2);
			std::vector<double> values(2);
			indices[1] = (int)var.absolute_index() + 1;
			values[1] = 1;
			glp_set_row_bnds(_lp, current_row, type, c._lower_bound.value(), c._upper_bound.value());
			glp_set_mat_row(_lp, current_row, 1, &indices[0], &values[0]);
		}
		else
		{

		}
	}

	if(_model->_minimize)
		glp_set_obj_dir(_lp, GLP_MIN);
	else
		glp_set_obj_dir(_lp, GLP_MAX);

	const auto& objective = _model->_objective;
	if (std::holds_alternative<expressions::sum>(objective))
	{
		const auto&sum = std::get<expressions::sum>(objective);
		glp_set_obj_coef(_lp, 0, sum._constant_term._value);
		for (const auto&term : sum._terms)
		{
			glp_set_obj_coef(_lp, (int)term.first + 1, term.second._coefficient._value);
		}
	}

	glp_smcp parm;
	glp_init_smcp(&parm);
	glp_simplex(_lp, &parm);
}

double glpk::get_objective_value()
{
	return glp_get_obj_val(_lp);
}
