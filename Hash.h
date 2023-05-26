#ifndef HASH_DEF_INCLUDE
#define HASH_DEF_INCLUDE

#include "Utils.h"

inline int32_t
HashLookup(uint64_t hash, int exp, int32_t idx) {
    uint32_t mask = ((uint32_t)1 << exp) - 1; // create mask with exp ones starting from lsb.
    uint32_t step = (uint32_t)((hash >> (64 - exp)) | 1); // moves the exp msb bits of the hash in the exp lsb bits, set lsb bit.
    return (idx + step) & mask;
}

inline uint64_t
HashString(Byte *s, int32_t len) {
    uint64_t h = 0x100;
    for (int32_t i = 0; i < len; i++) {
        h ^= s[i] & 255;
        h *= 1111111111111111111;
    }
    return h ^ h>>32;
}

#endif // Header file

#ifdef HASH_IMPL

int32_t HashLookup(uint64_t hash, int exp, int32_t idx);
uint64_t HashString(Byte *s, int32_t len);

#undef HASH_IMPL
#endif
