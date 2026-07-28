#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

// 지원하는 컴파일러가 있다면 이 정확한 타입들을 사용할 수 있습니다:
#ifdef __STDC_IEC_60559_TYPES__
typedef _Float32_t float32_t;
typedef _Float64_t float64_t;
#else
// 아니면 직접 정의합니다.
// 아키텍처에 따라 다릅니다! 하지만 아마 다음과 같을 것입니다.
typedef float float32_t;
typedef double float64_t;
#endif

// float와 double을 포장하기 위한 매크로:
#define pack754_32(f) (pack754((f), 32, 8))
#define pack754_64(f) (pack754((f), 64, 11))
#define unpack754_32(i) (unpack754((i), 32, 8))
#define unpack754_64(i) (unpack754((i), 64, 11))

/*
** pack754() -- 부동소수점 수를 IEEE-754 형식으로 포장합니다
*/ 
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

/*
** unpack754() -- IEEE-754 형식에서 부동소수점 수를 풀어냅니다
*/ 
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

/*
** packi16() -- 16비트 int를 char 버퍼에 저장합니다(htons()처럼)
*/ 
void packi16(uint8_t *buf, int16_t i)
{
	uint16_t i2 = i;

	*buf++ = i2>>8; *buf++ = i2;
}

/*
** packi32() -- 32비트 int를 char 버퍼에 저장합니다(htonl()처럼)
*/ 
void packi32(uint8_t *buf, int32_t i)
{
	uint32_t i2 = i;

	*buf++ = i2>>24; *buf++ = i2>>16;
	*buf++ = i2>>8;  *buf++ = i2;
}

/*
** packi64() -- 64비트 int를 char 버퍼에 저장합니다(htonl()처럼)
*/ 
void packi64(uint8_t *buf, int64_t i)
{
	uint64_t i2 = i;

	*buf++ = i2>>56; *buf++ = i2>>48;
	*buf++ = i2>>40; *buf++ = i2>>32;
	*buf++ = i2>>24; *buf++ = i2>>16;
	*buf++ = i2>>8;  *buf++ = i2;
}

/*
** unpacki16() -- char 버퍼에서 16비트 int를 풀어냅니다(ntohs()처럼)
*/ 
int16_t unpacki16(uint8_t *buf)
{
	uint16_t i2 = ((uint16_t)buf[0]<<8) | buf[1];
	int16_t i;

	// 부호 없는 수를 부호 있는 수로 바꿉니다
	if (i2 <= 0x7fffu) { i = i2; }
	//else { i = -(int16_t)((uint16_t)0xffff - i2 + (uint16_t)1u); }
	else { i = -1 - (uint16_t)(0xffffu - i2); }

	return i;
}

/*
** unpacki32() -- char 버퍼에서 32비트 int를 풀어냅니다(ntohl()처럼)
*/ 
int32_t unpacki32(uint8_t *buf)
{
	uint32_t i2 = ((uint32_t)buf[0]<<24) | ((uint32_t)buf[1]<<16) |
	              ((uint32_t)buf[2]<<8)  | buf[3];
	int32_t i;

	// 부호 없는 수를 부호 있는 수로 바꿉니다
	if (i2 <= 0x7fffffffu) { i = i2; }
	else { i = -1 - (int32_t)(0xffffffffu - i2); }

	return i;
}

/*
** unpacki64() -- char 버퍼에서 64비트 int를 풀어냅니다(ntohl()처럼)
*/ 
int64_t unpacki64(uint8_t *buf)
{
	uint64_t i2 = ((uint64_t)buf[0]<<56) | ((uint64_t)buf[1]<<48) |
	              ((uint64_t)buf[2]<<40) | ((uint64_t)buf[3]<<32) |
	              ((uint64_t)buf[4]<<24) | ((uint64_t)buf[5]<<16) |
	              ((uint64_t)buf[6]<<8)  | buf[7];
	int64_t i;

	// 부호 없는 수를 부호 있는 수로 바꿉니다
	if (i2 <= 0x7fffffffffffffffu) { i = i2; }
	else { i = -1 -(int64_t)(0xffffffffffffffffu - i2); }

	return i;
}

/*
** pack() -- 형식 문자열이 지시한 방식으로 버퍼에 데이터를 저장합니다
**
**  c - 8비트 부호 있는 int     h - 16비트 부호 있는 int
**  l - 32비트 부호 있는 int    f - 32비트 float
**  L - 64비트 부호 있는 int    F - 64비트 float
**  s - 문자열(16비트 길이가 자동으로 앞에 붙습니다)
*/ 
int32_t pack(uint8_t *buf, char *format, ...)
{
	va_list ap;
	int16_t h;
	int32_t l;
	int64_t L;
	int8_t c;
	float32_t f;
	float64_t F;
	char *s;
	int32_t size = 0, len;

	va_start(ap, format);

	for(; *format != '\0'; format++) {
		switch(*format) {
		case 'h': // 16비트
			size += 2;
			h = (int16_t)va_arg(ap, int); // 승격됨
			packi16(buf, h);
			buf += 2;
			break;

		case 'l': // 32비트
			size += 4;
			l = va_arg(ap, int32_t);
			packi32(buf, l);
			buf += 4;
			break;

		case 'L': // 64비트
			size += 8;
			L = va_arg(ap, int64_t);
			packi64(buf, L);
			buf += 8;
			break;

		case 'c': // 8비트
			size += 1;
			c = (int8_t)va_arg(ap, int); // 승격됨
			*buf++ = (c>>0)&0xff;
			break;

		case 'f': // float
			size += 4;
			f = (float32_t)va_arg(ap, double); // 승격됨
			l = pack754_32(f); // IEEE 754로 변환
			packi32(buf, l);
			buf += 4;
			break;

		case 'F': // float-64
			size += 8;
			F = (float64_t)va_arg(ap, float64_t);
			L = pack754_64(F); // IEEE 754로 변환
			packi64(buf, L);
			buf += 8;
			break;

		case 's': // 문자열
			s = va_arg(ap, char*);
			len = strlen(s);
			size += len + 2;
			packi16(buf, len);
			buf += 2;
			memcpy(buf, s, len);
			buf += len;
			break;
		}
	}

	va_end(ap);

	return size;
}

/*
** unpack() -- 형식 문자열이 지시한 방식으로 데이터를 버퍼에 풀어냅니다
*/
void unpack(uint8_t *buf, char *format, ...)
{
	va_list ap;
	int16_t *h;
	int32_t *l;
	int64_t *L;
	int32_t pf;
	int64_t pF;
	int8_t *c;
	float32_t *f;
	float64_t *F;
	char *s;
	int32_t len, count, maxstrlen=0;

	va_start(ap, format);

	for(; *format != '\0'; format++) {
		switch(*format) {
		case 'h': // 16비트
			h = va_arg(ap, int16_t*);
			*h = unpacki16(buf);
			buf += 2;
			break;

		case 'l': // 32비트
			l = va_arg(ap, int32_t*);
			*l = unpacki32(buf);
			buf += 4;
			break;

		case 'L': // 64비트
			L = va_arg(ap, int64_t*);
			*L = unpacki64(buf);
			buf += 8;
			break;

		case 'c': // 8비트
			c = va_arg(ap, int8_t*);
			if (*buf <= 0x7f) { *c = *buf;}
			else { *c = -1 - (uint8_t)(0xffu - *buf); }
			buf++;
			break;

		case 'f': // float
			f = va_arg(ap, float32_t*);
			pf = unpacki32(buf);
			buf += 4;
			*f = unpack754_32(pf);
			break;

		case 'F': // float-64
			F = va_arg(ap, float64_t*);
			pF = unpacki64(buf);
			buf += 8;
			*F = unpack754_64(pF);
			break;

		case 's': // 문자열
			s = va_arg(ap, char*);
			len = unpacki16(buf);
			buf += 2;
			if (maxstrlen > 0 && len > maxstrlen) count = maxstrlen - 1;
			else count = len;
			memcpy(s, buf, count);
			s[count] = '\0';
			buf += len;
			break;

		default:
			if (isdigit(*format)) { // 최대 문자열 길이를 추적합니다
				maxstrlen = maxstrlen * 10 + (*format-'0');
			}
		}

		if (!isdigit(*format)) maxstrlen = 0;
	}

	va_end(ap);
}

//#define DEBUG
#ifdef DEBUG
#include <limits.h>
#include <float.h>
#include <assert.h>
#endif

int main(void)
{
#ifndef DEBUG
	uint8_t buf[1024];
	int8_t magic;
	int16_t monkeycount;
	int32_t altitude;
	float32_t absurdityfactor;
	char *s = "Great unmitigated Zot!  You've found the Runestaff!";
	char s2[96];
	int16_t packetsize, ps2;

	packetsize = pack(buf, "chhlsf", (int8_t)'B', (int16_t)0,
            (int16_t)37, (int32_t)-5, s, (float32_t)-3490.6677);
	packi16(buf+1, packetsize); // 재미삼아 패킷 크기를 저장합니다

	printf("packet is %" PRId32 " bytes\n", packetsize);

	unpack(buf, "chhl96sf", &magic, &ps2, &monkeycount, &altitude,
            s2, &absurdityfactor);

	printf("'%c' %" PRId32" %" PRId16 " %" PRId32
			" \"%s\" %f\n", magic, ps2, monkeycount,
			altitude, s2, absurdityfactor);

#else
	uint8_t buf[1024];

	int x;

	int64_t k, k2;
	int64_t test64[14] = { 0, -0, 1, 2, -1, -2, LLONG_MAX>>1, LLONG_MAX-1, LLONG_MAX, LLONG_MIN+1, LLONG_MIN, 9007199254740991, 9007199254740992, 9007199254740993 };

	int32_t i, i2;
	int32_t test32[14] = { 0, -0, 1, 2, -1, -2, INT_MAX>>1, INT_MAX-1, INT_MAX, INT_MIN+1, INT_MIN, 0, 0, 0 };

	int16_t j, j2;
	int16_t test16[14] = { 0, -0, 1, 2, -1, -2, SHRT_MAX>>1, SHRT_MAX-1, SHRT_MAX, SHRT_MIN+1, SHRT_MIN, 0, 0, 0 };

	// 부동소수점 타입이 맞는지 확인하기 위해 아주 기본적인 설정을 합니다:
	if (sizeof(float32_t) != 4 || sizeof(float64_t) != 8) {
		char *f32 = NULL, *f64 = NULL;

		if (sizeof(float) == 4) { f32 = "float"; }
		else if (sizeof(double) == 4) { f32 = "double"; }
		else if (sizeof(long double) == 4) { f32 = "long double"; }
				
		if (sizeof(float) == 8) { f64 = "float"; }
		else if (sizeof(double) == 8) { f64 = "double"; }
		else if (sizeof(long double) == 8) { f64 = "long double"; }

		if (f32 == NULL || f64 == NULL) {
			printf("I can't find the following size floating point types:%s%s\n\n", f32==NULL?" 32-bit":"", f64==NULL?" 64-bit":"");
			printf("Change the typedefs at the top of this source to the right types.\n");
			return 1;
		}

		printf("Please modify this source so the following typedefs are at the top:\n\n");
		printf("typedef %s float32_t;\n", f32);
		printf("typedef %s float64_t;\n", f64);

		return 1;
	}

	for(x = 0; x < 14; x++) {
		k = test64[x];
		pack(buf, "L", k);
		unpack(buf, "L", &k2);

		if (k2 != k) {
			printf("64: %" PRId64 " != %" PRId64 "\n", k, k2);
			printf("  before: %016" PRIx64 "\n", k);
			printf("  after:  %016" PRIx64 "\n", k2);
			printf("  buffer: %02hhx %02hhx %02hhx %02hhx "
				" %02hhx %02hhx %02hhx %02hhx\n", 
				buf[0], buf[1], buf[2], buf[3],
				buf[4], buf[5], buf[6], buf[7]);
		} else {
			//printf("64: OK: %" PRId64 " == %" PRId64 "\n", k, k2);
		}

		i = test32[x];
		pack(buf, "l", i);
		unpack(buf, "l", &i2);

		if (i2 != i) {
			printf("32: %" PRId32 " != %" PRId32 "\n", i, i2);
		} else {
			//printf("32: OK: %" PRId32 " == %" PRId32 "\n", i, i2);
		}

		j = test16[x];
		pack(buf, "h", j);
		unpack(buf, "h", &j2);

		if (j2 != j) {
			printf("16: %" PRId16 " != %" PRId16 "\n", j, j2);
		} else {
			//printf("16: OK: %" PRId16 " == %" PRId16 "\n", j, j2);
		}
	}

	{
		float64_t testf64[7] = { 0.0, 1.0, -1.0, DBL_MIN*2, DBL_MAX/2, DBL_MIN, DBL_MAX };
		float64_t f,f2;

		for (i = 0; i < 7; i++) {
			f = testf64[i];
			pack(buf, "F", f);
			unpack(buf, "F", &f2);

			if (f2 != f) {
				printf("f64: %f != %f\n", f, f2);
				printf("  before: %016" PRIx64 "\n", *((uint64_t*)&f));
				printf("  after:  %016" PRIx64 "\n", *((uint64_t*)&f2));
				printf("  buffer: %02hhx %02hhx %02hhx %02hhx "
					" %02hhx %02hhx %02hhx %02hhx\n", 
					buf[0], buf[1], buf[2], buf[3],
					buf[4], buf[5], buf[6], buf[7]);
			} else {
				//printf("f64: OK: %f == %f\n", f, f2);
			}
		}
	}
#endif

	return 0;
}
