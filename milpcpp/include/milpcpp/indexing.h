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
			typedef std::string lookup_type;

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

		template<typename X>
		struct range_bound
		{
			static long _value;
			static void set_value(long value) { _value = value; }
			static long value() { return _value; }
		};

		template<typename X>
		long range_bound<X>::_value = 0;
	};

	template<long _Lower, typename _End>
	class range
	{
		size_t _offset = 0;
	public:
		typedef long lookup_type;

		range(){ }
		range(size_t offset) { _offset = offset; }
		template<long I, typename T> operator range<I, T>() const { return range<I, T>(_offset + (_Lower - I)); }
		range<_Lower, _End> operator-(long rhs) const { return range<_Lower, _End>(_offset - rhs); }
		long name() const { return _Lower + (long)_offset; }
		static size_t size() { return _End::value() - _Lower + 1; }
		size_t raw_index() const { return _offset;  }
		static std::string name(size_t offset) { return std::to_string(_Lower + offset); }
		static range<_Lower, _End> begin() { return range<_Lower, _End>(0); }
		static range<_Lower, _End> end() { return range<_Lower, _End>(_End::_value - _Lower + 1); }
		range<_Lower, _End>&operator++() { ++_offset; return *this; }
		bool operator!=(const range<_Lower, _End>&other) const { return _offset != other._offset;  }
		range<_Lower, _End> operator*() const { return *this;  }
		static size_t index_of(long index_name) { return index_name - _Lower; }
	};

	template<typename T>
	inline T first() { return T(0); }

	template<typename T>
	inline T prev(const T&i) { 
		size_t index = i.raw_index();
		if (index <= 0)
		{
			throw indexing::invalid_index("Index already at lower bound!");
		}
		return T(index -1); 
	}

	template<typename T>
	inline long ord(const T&i) {
		size_t index = i.raw_index();
		return long(index + 1);
	}
}

#endif
