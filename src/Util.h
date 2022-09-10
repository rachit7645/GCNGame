#ifndef UTIL_H
#define UTIL_H

// Utility defines 

#define GCN_STATIC      static
#define GCN_INLINE      inline
#define GCN_CONSTEXPR   constexpr
#define GCN_UNUSED      __attribute__((unused))

// Exit codes

#define GCN_EXIT_SUCESS 0
#define GCN_EXIT_START  1

// Macros

// TODO: Make type safe template
#define GCN_ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#define U8_TO_VOID_PTR(x) (reinterpret_cast<void*>(const_cast<u8*>(x)))

#endif