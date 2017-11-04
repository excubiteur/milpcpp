#ifndef __MILPCPP_INDEXING_H__
#define __MILPCPP_INDEXING_H__

#include<exception>
#include<map>
#include<string>
#include<vector>

namespace milpcpp
{
	namespace indexing
	{
		struct invalid_index : std::runtime_error
		{
			invalid_index(const std::string& what) :runtime_error(what) {}
		};

		class index_set
		{
			std::vector<std::string> _elements;
			std::map<std::string, size_t> _lookup;
		public:
			void add(const std::string&name)
			{
				_lookup[name] = _elements.size();
				_elements.push_back(name);
			}
			size_t size() const { return _elements.size(); }
			size_t index_of(const std::string&name)
			{
				auto index = _lookup.find(name);
				if (index == _lookup.end())
					throw invalid_index(name);
				return index->second;
			}
			std::string name(size_t raw_index) const { return _elements[raw_index]; }
		};

		template<typename T>
		class index
		{
			size_t _raw_index;
		public:
			index(size_t raw_index) : _raw_index(raw_index) {}
			size_t raw_index() const { return _raw_index; }
			static index_set * _index_set;
			static void add(const std::string&name)
			{
				_index_set->add(name);
			}
			static size_t size() { return _index_set->size(); }
			static size_t index_of(const std::string&name) { return _index_set->index_of(name); }
			static std::string name(size_t raw_index) { return _index_set->name(raw_index); }
			std::string name() const { return name(_raw_index); }
		};

		template<typename T>
		index_set * index<T>::_index_set = nullptr;

	};


}

#endif
