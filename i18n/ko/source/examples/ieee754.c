#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#define pack754_32(f) (pack754((f), 32, 8))
#define pack754_64(f) (pack754((f), 64, 11))
#define unpack754_32(i) (unpack754((i), 32, 8))
#define unpack754_64(i) (unpack754((i), 64, 11))

uint64_t pack754(long double f, unsigned bits, unsigned expbits)
{
	long double fnorm;
	int shift;
	long long sign, exp, significand;
	unsigned significandbits = bits - expbits - 1; // 부호 비트 때문에 -1

	if (f == 0.0) return 0; // 이 특수한 경우를 먼저 처리합니다

	// 부호를 확인하고 정규화를 시작합니다
	if (f < 0) { sign = 1; fnorm = -f; }
	else { sign = 0; fnorm = f; }

	// f의 정규화된 형태를 얻고 지수를 추적합니다
	shift = 0;
	while(fnorm >= 2.0) { fnorm /= 2.0; shift++; }
	while(fnorm < 1.0) { fnorm *= 2.0; shift--; }
	fnorm = fnorm - 1.0;

	// 유효숫자 데이터의 이진 형태(부동소수점 아님)를 계산합니다
	significand = fnorm * ((1LL<<significandbits) + 0.5f);

	// 바이어스가 적용된 지수를 얻습니다
	exp = shift + ((1<<(expbits-1)) - 1); // 이동값 + 바이어스

	// 최종 답을 반환합니다
	return (sign<<(bits-1)) | (exp<<(bits-expbits-1)) | significand;
}

long double unpack754(uint64_t i, unsigned bits, unsigned expbits)
{
	long double result;
	long long shift;
	unsigned bias;
	unsigned significandbits = bits - expbits - 1; // 부호 비트 때문에 -1

	if (i == 0) return 0.0;

	// 유효숫자를 꺼냅니다
	result = (i&((1LL<<significandbits)-1)); // 마스크
	result /= (1LL<<significandbits); // 다시 부동소수점으로 변환합니다
	result += 1.0f; // 1을 다시 더합니다

	// 지수를 처리합니다
	bias = (1<<(expbits-1)) - 1;
	shift = ((i>>significandbits)&((1LL<<expbits)-1)) - bias;
	while(shift > 0) { result *= 2.0; shift--; }
	while(shift < 0) { result /= 2.0; shift++; }

	// 부호를 적용합니다
	result *= (i>>(bits-1))&1? -1.0: 1.0;

	return result;
}

int main(void)
{
	float f = 3.1415926, f2;
	double d = 3.14159265358979323, d2;
	uint32_t fi;
	uint64_t di;

	fi = pack754_32(f);
	f2 = unpack754_32(fi);

	di = pack754_64(d);
	d2 = unpack754_64(di);

	printf("float before : %.7f\n", f);
	printf("float encoded: 0x%08" PRIx32 "\n", fi);
	printf("float after  : %.7f\n\n", f2);

	printf("double before : %.20lf\n", d);
	printf("double encoded: 0x%016" PRIx64 "\n", di);
	printf("double after  : %.20lf\n", d2);

	return 0;
}
