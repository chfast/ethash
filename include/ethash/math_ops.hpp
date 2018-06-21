#pragma once
namespace ethash
{

//#define ROTR32(x, n)  (((0U + (x)) << (32 - (n))) | ((x) >> (n)))  // Assumes that x is uint32_t and 0 < n < 32

#define ROTL(x,n,w) (((x) << (n)) | ((x) >> ((w) - (n))))
#define ROTL32(x,n) ROTL(x,n,32)	/* 32 bits word */

#define ROTR(x,n,w) (((x) >> (n)) | ((x) << ((w) - (n))))

#define ROTR32(x,n) ROTR(x,n,32)	/* 32 bits word */

#define min_(a,b) ((a<b) ? a : b)
//#define mul_hi(a, b) __umulhi(a, b)
uint32_t mul_hi (uint32_t a, uint32_t b){
    uint64_t result = (uint64_t) a * (uint64_t) b;
    return  (uint32_t) (result>>32);
}
//#define clz(a) __clz(a)
uint32_t clz (uint32_t a){
    uint32_t result = 0;
    for(int i=31;i>=0;i--){
        if(((a>>i)&1) == 0)
            result ++;
        else
            break;
    }
    return result;
}
//#define popcount(a) __popc(a)
uint32_t popcount(uint32_t a) {
	uint32_t result = 0;
	for (int i = 31; i >= 0; i--) {
		if (((a >> i) & 1) == 1)
			result++;
	}
	return result;
}
}
