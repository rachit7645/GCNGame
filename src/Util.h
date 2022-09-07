#ifndef UTIL_H
#define UTIL_H

#define GCN_STATIC      static
#define GCN_INLINE      inline
#define GCN_CONSTEXPR   constexpr
#define GCN_UNUSED      __attribute__((unused))
#define GCN_EXIT_SUCESS 0

#define GCN_ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

#endif