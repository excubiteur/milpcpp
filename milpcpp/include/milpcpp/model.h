#ifndef __MILPCPP_MODEL_H__
#define __MILPCPP_MODEL_H__

#include<algorithm>
#include<string>
#include<vector>

#include<milpcpp/bounds.h>

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

		static void set_objective(expression&& e) { _context->_objective = std::move(e); }
		static void add_constraints(std::vector<constraint>&&c) 
		{ 
			std::move(c.begin(), c.end(), std::back_inserter(_context->_constraints));
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
}

#endif
