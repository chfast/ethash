/* ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
 * Copyright 2018 Pawel Bylica.
 * Licensed under the Apache License, Version 2.0. See the LICENSE file.
 */

#if _MSC_VER || __STDC_VERSION__
#define INLINE inline
#else
#define INLINE
#endif

#if _MSC_VER
#define ALWAYS_INLINE __forceinline
#elif defined(__has_attribute) && __STDC_VERSION__
#if __has_attribute(always_inline)
#define ALWAYS_INLINE __attribute__((always_inline))
#endif
#endif

#if !defined(ALWAYS_INLINE)
#define ALWAYS_INLINE
#endif
