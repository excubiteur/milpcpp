#ifndef __MILPCPP_ENUMERATE_H__
#define __MILPCPP_ENUMERATE_H__

#include<range/v3/all.hpp>

namespace milpcpp
{

	namespace utils
	{
		template<typename T>
		auto enumerate(const T&v)
		{
			return ranges::view::zip(ranges::view::ints(0), v);
		}
	};

}

#endif
