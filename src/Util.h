#ifndef UTIL_H
#define UTIL_H

// Utility defines 

#define GCN_STATIC      static
#define GCN_INLINE      inline
#define GCN_CONSTEXPR   constexpr
#define GCN_UNUSED      __attribute__((unused))

// Exit codes

constexpr auto GCN_EXIT_SUCCESS = 0;
constexpr auto GCN_EXIT_START   = 1;

namespace Util
{
	template <typename T, size_t S>
	constexpr auto ArraySize(GCN_UNUSED T (&arr)[S])
	{
		return S;
	}

	template<typename T>
	constexpr auto ToVoidPtr(const T* x)
	{
		return reinterpret_cast<void*>(const_cast<T*>(x));
	}
}

#endif