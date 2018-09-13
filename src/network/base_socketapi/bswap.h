//
// Created by boon on 17-11-6.
//

#ifndef TLRS_BSWAP_H
#define TLRS_BSWAP_H

# if 0 //defined(__GNUC__) && defined(__GLIBC__)
#  include <byteswap.h>
# else /* GNUC & GLIBC */
#  define bswap_64(n) \
      ( (((n) & 0xff00000000000000ull) >> 56) \
      | (((n) & 0x00ff000000000000ull) >> 40) \
      | (((n) & 0x0000ff0000000000ull) >> 24) \
      | (((n) & 0x000000ff00000000ull) >> 8)  \
      | (((n) & 0x00000000ff000000ull) << 8)  \
      | (((n) & 0x0000000000ff0000ull) << 24) \
      | (((n) & 0x000000000000ff00ull) << 40) \
      | (((n) & 0x00000000000000ffull) << 56) )

/* Swap bytes in 32 bit value.  */
#define bswap_32(x) \
     ((((x) & 0xff000000u) >> 24) | (((x) & 0x00ff0000u) >>  8) |         \
           (((x) & 0x0000ff00u) <<  8) | (((x) & 0x000000ffu) << 24))

/* Swap bytes in 16 bit value.  */
#define bswap_16(x) \
     ((((x) >> 8) & 0xffu) | (((x) & 0xffu) << 8))

#endif /* GNUC & GLIBC */

#ifdef __cplusplus
template<typename _Ty>
_Ty byte_swap(const _Ty& _v) {
    switch (sizeof(_Ty)) {
        case 2:
            return bswap_16(_v);
        case 4:
            return bswap_32(_v);
        case 8:
            return bswap_64(_v);
        default:
            return _v;
    }
}
#endif

#endif //TLRS_BSWAP_H