#ifndef __MILPCPP_UTILS_H__
#define __MILPCPP_UTILS_H__

#include<type_traits>

namespace milpcpp
{

	namespace utils
	{
		template <typename T>
		struct function_traits
			: public function_traits<decltype(&T::operator())>
		{};

		template <typename ClassType, typename ReturnType, typename... Args>
		struct function_traits<ReturnType(ClassType::*)(Args...) const>
		{
			enum { arity = sizeof...(Args) };

			typedef ReturnType result_type;

			template <size_t i>
			struct arg
			{
				typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
			};
		};
	};

}

#endif
