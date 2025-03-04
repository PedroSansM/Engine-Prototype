#pragma once

#include <cstddef>
#include <type_traits>
#include <functional>



namespace DCore
{

struct NullType {};

template <class ...>
struct TypeList
{
	using head = NullType;
	using tail = NullType;

	enum {size = 0};
};

template <class Head, class ...Tail>
struct TypeList<Head, Tail...>
{
	using head = Head;
	using tail =  TypeList<Tail...>;

	enum {size = 1 + tail::size};
};

using EmptyTypeList = TypeList<>;

template <size_t ...>
struct ITypeList
{
	static constexpr size_t size{0};
};

template <size_t Value, size_t ...Values>
struct ITypeList<Value, Values...>
{
	using tail = ITypeList<Values...>;

	static constexpr size_t head{Value};
	static constexpr size_t size{1 + tail::size};
};

template <size_t Index, class ITypeList>
struct ValueAt
{
	static constexpr size_t value{ValueAt<Index - 1, typename ITypeList::tail>::value};
};

template <class ITypeList>
struct ValueAt<0, ITypeList>
{
	static constexpr size_t value{ITypeList::head};
};

template <class ITypeList> 
struct IterateOnITypeList
{
	template <class Func, size_t CurrentIndex = 0>
	void iterate(Func function) const
	{
		if constexpr (CurrentIndex >= ITypeList::size)
		{
			return;
		}
		else
		{
			std::invoke(function, ValueAt<CurrentIndex, ITypeList>::value);
			IterateOnITypeList<ITypeList>{}.iterate<CurrentIndex + 1>(function);
		}
	}
};

template <size_t Index, class ...Types>
struct TypeAt
{};

template <size_t Index, class Type, class ...Types>
struct TypeAt<Index, Type, Types...>
{
	using type = typename TypeAt<Index - 1, Types...>::type;	
};

template <class Type, class ...Types>
struct TypeAt<0, Type, Types...>
{
	using type = Type;
};

template <class TargetClass>
struct ConstructorArgs {};

template <class ...>
struct AreSame
{
	enum {value = true};
};

template <class Type1, class Type2, class ...Types>
struct AreSame<Type1, Type2, Types...>
{
	enum {value = std::is_same<Type1, Type2>::value && AreSame<Type1, Types...>::value};
};

template <class ...>
struct AreSameOr
{
	static constexpr bool value{false};
};

template <class Type1, class Type2, class ...Types>
struct AreSameOr<Type1, Type2, Types...>
{
	static constexpr bool value{std::is_same<Type1, Type2>::value || AreSameOr<Type1, Types...>::value};
};

template <class Type, class ...Types>
struct RuntimeTypes
{
	Type Value;
	RuntimeTypes<Types...> Next;
};

template <class Type>
struct RuntimeTypes<Type>
{
	Type Value;
};

template <class Type, size_t NumberOfTypes>
struct RepeatedRuntimeTypes
{
	Type Value;
	RepeatedRuntimeTypes<Type, NumberOfTypes - 1> Next;
};

template <class Type>
struct RepeatedRuntimeTypes<Type, 0>
{
	Type Value;
};

template <bool Value, class Type1, class Type2>
struct SelectType
{};

template <class Type1, class Type2>
struct SelectType<true, Type1, Type2>
{
	using type = Type1;
};	

template <class Type1, class Type2>
struct SelectType<false, Type1, Type2>
{
	using type = Type2;
};	

}
