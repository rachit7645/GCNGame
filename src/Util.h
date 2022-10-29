#ifndef UTIL_H
#define UTIL_H

// Utility defines 

#define GCN_UNUSED      __attribute__((unused))

// Exit codes

enum ExitCodes : int
{
	SUCCESS,
	START
};

namespace Util
{
	// \fn constexpr auto Util::ArraySize(T (&arr)[S])
	// \brief Calculates the size of the given array; Will not compile on non-array types
	// \param T Any type
	// \param S Size
	// \param array Array
	// \returns size_t
	template <typename T, size_t S>
	constexpr auto ArraySize(GCN_UNUSED T (&array)[S])
	{
		return S;
	}

	// \fn constexpr auto Util::RemoveConst(const T* x)
	// \brief Removes const from the given pointer
	// \param T Any type
	// \param R Any type
	// \param x Const Pointer Of Type T
	// \returns R*
	template<typename T, typename R>
	constexpr auto RemoveConst(const T* x)
	{
		return reinterpret_cast<R*>(const_cast<T*>(x));
	}

	// \fn constexpr auto Util::ToVoidPtr(const T* x)
	// \brief Casts the given pointer to void*
	// \param T Any type
	// \param x Const Pointer Of Type T
	// \returns void*
	template<typename T>
	constexpr auto ToVoidPtr(const T* x)
	{
		return RemoveConst<T, void*>(x);
	}
}

#endif