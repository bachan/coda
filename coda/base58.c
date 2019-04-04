#include "base58.h"
#include <stdlib.h>
#include <limits.h>

static inline unsigned char to_uchar (char ch)
{
    return ch;
}

void coda_base58_encode (const char* in, size_t inlen, char* out, size_t outlen)
{
    static const char b58str[58] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

    while (outlen && inlen && !*in)
    {
        *out++ = '1';
        outlen--;

        in++;
        inlen--;
    }

    unsigned b58len = inlen * 138 / 100 + 1; // log(256) / log(58), rounded up.
    char* b58_begin = NULL;
    char* b58_curr = NULL;
    b58_begin = b58_curr = (char*)calloc(b58len, sizeof(unsigned char));

    while (inlen)
    {
        unsigned int carry = (unsigned char)*in++;
        int i;
        for (i = b58len - 1; carry || i >= 0; i--)
        {
            carry += 256 * b58_curr[i];
            b58_curr[i] = carry % 58;
            carry /= 58;
        }
        inlen--;
    }

    // Skip leading zeroes in base58 result.
    while (b58len && !*b58_curr)
    {
        b58_curr++;
        b58len--;
    }

    while (outlen && b58len)
    {
        *out++ = b58str[(unsigned char)*b58_curr++];
        b58len--;
        outlen--;
    }

    if (outlen) *out = '\0';

    free(b58_begin);
}

size_t coda_base58_encode_alloc (const char *in, size_t inlen, char **out)
{
    size_t zeroes = 0;
    size_t nonzero_inlen = inlen;
    while (nonzero_inlen && !in[zeroes])
    {
        zeroes++;
        nonzero_inlen--;
    }

    size_t outlen = zeroes + nonzero_inlen * 138 / 100 + 2;

    *out = malloc (outlen);
    if (!*out)
        return outlen;

    coda_base58_encode (in, inlen, *out, outlen);
    return outlen - 1;
}

#define B58(_)                 \
  ((_) == '1' ? 0               \
   : (_) == '2' ? 1				\
   : (_) == '3' ? 2				\
   : (_) == '4' ? 3				\
   : (_) == '5' ? 4				\
   : (_) == '6' ? 5				\
   : (_) == '7' ? 6				\
   : (_) == '8' ? 7				\
   : (_) == '9' ? 8				\
   : (_) == 'A' ? 9				\
   : (_) == 'B' ? 10				\
   : (_) == 'C' ? 11				\
   : (_) == 'D' ? 12				\
   : (_) == 'E' ? 13				\
   : (_) == 'F' ? 14				\
   : (_) == 'G' ? 15				\
   : (_) == 'H' ? 16				\
   : (_) == 'J' ? 17				\
   : (_) == 'K' ? 18				\
   : (_) == 'L' ? 19				\
   : (_) == 'M' ? 20				\
   : (_) == 'N' ? 21				\
   : (_) == 'P' ? 22				\
   : (_) == 'Q' ? 23				\
   : (_) == 'R' ? 24				\
   : (_) == 'S' ? 25				\
   : (_) == 'T' ? 26				\
   : (_) == 'U' ? 27				\
   : (_) == 'V' ? 28				\
   : (_) == 'W' ? 29				\
   : (_) == 'X' ? 30				\
   : (_) == 'Y' ? 31				\
   : (_) == 'Z' ? 32				\
   : (_) == 'a' ? 33				\
   : (_) == 'b' ? 34				\
   : (_) == 'c' ? 35				\
   : (_) == 'd' ? 36				\
   : (_) == 'e' ? 37				\
   : (_) == 'f' ? 38				\
   : (_) == 'g' ? 39				\
   : (_) == 'h' ? 40				\
   : (_) == 'i' ? 41				\
   : (_) == 'j' ? 42				\
   : (_) == 'k' ? 43				\
   : (_) == 'm' ? 44				\
   : (_) == 'n' ? 45				\
   : (_) == 'o' ? 46				\
   : (_) == 'p' ? 47				\
   : (_) == 'q' ? 48				\
   : (_) == 'r' ? 49				\
   : (_) == 's' ? 50				\
   : (_) == 't' ? 51				\
   : (_) == 'u' ? 52				\
   : (_) == 'v' ? 53				\
   : (_) == 'w' ? 54				\
   : (_) == 'x' ? 55				\
   : (_) == 'y' ? 56				\
   : (_) == 'z' ? 57				\
   : -1)

static const signed char b58[0x100] =
{
    B58 (0), B58 (1), B58 (2), B58 (3),
    B58 (4), B58 (5), B58 (6), B58 (7),
    B58 (8), B58 (9), B58 (10), B58 (11),
    B58 (12), B58 (13), B58 (14), B58 (15),
    B58 (16), B58 (17), B58 (18), B58 (19),
    B58 (20), B58 (21), B58 (22), B58 (23),
    B58 (24), B58 (25), B58 (26), B58 (27),
    B58 (28), B58 (29), B58 (30), B58 (31),
    B58 (32), B58 (33), B58 (34), B58 (35),
    B58 (36), B58 (37), B58 (38), B58 (39),
    B58 (40), B58 (41), B58 (42), B58 (43),
    B58 (44), B58 (45), B58 (46), B58 (47),
    B58 (48), B58 (49), B58 (50), B58 (51),
    B58 (52), B58 (53), B58 (54), B58 (55),
    B58 (56), B58 (57), B58 (58), B58 (59),
    B58 (60), B58 (61), B58 (62), B58 (63),
    B58 (64), B58 (65), B58 (66), B58 (67),
    B58 (68), B58 (69), B58 (70), B58 (71),
    B58 (72), B58 (73), B58 (74), B58 (75),
    B58 (76), B58 (77), B58 (78), B58 (79),
    B58 (80), B58 (81), B58 (82), B58 (83),
    B58 (84), B58 (85), B58 (86), B58 (87),
    B58 (88), B58 (89), B58 (90), B58 (91),
    B58 (92), B58 (93), B58 (94), B58 (95),
    B58 (96), B58 (97), B58 (98), B58 (99),
    B58 (100), B58 (101), B58 (102), B58 (103),
    B58 (104), B58 (105), B58 (106), B58 (107),
    B58 (108), B58 (109), B58 (110), B58 (111),
    B58 (112), B58 (113), B58 (114), B58 (115),
    B58 (116), B58 (117), B58 (118), B58 (119),
    B58 (120), B58 (121), B58 (122), B58 (123),
    B58 (124), B58 (125), B58 (126), B58 (127),
    B58 (128), B58 (129), B58 (130), B58 (131),
    B58 (132), B58 (133), B58 (134), B58 (135),
    B58 (136), B58 (137), B58 (138), B58 (139),
    B58 (140), B58 (141), B58 (142), B58 (143),
    B58 (144), B58 (145), B58 (146), B58 (147),
    B58 (148), B58 (149), B58 (150), B58 (151),
    B58 (152), B58 (153), B58 (154), B58 (155),
    B58 (156), B58 (157), B58 (158), B58 (159),
    B58 (160), B58 (161), B58 (162), B58 (163),
    B58 (164), B58 (165), B58 (166), B58 (167),
    B58 (168), B58 (169), B58 (170), B58 (171),
    B58 (172), B58 (173), B58 (174), B58 (175),
    B58 (176), B58 (177), B58 (178), B58 (179),
    B58 (180), B58 (181), B58 (182), B58 (183),
    B58 (184), B58 (185), B58 (186), B58 (187),
    B58 (188), B58 (189), B58 (190), B58 (191),
    B58 (192), B58 (193), B58 (194), B58 (195),
    B58 (196), B58 (197), B58 (198), B58 (199),
    B58 (200), B58 (201), B58 (202), B58 (203),
    B58 (204), B58 (205), B58 (206), B58 (207),
    B58 (208), B58 (209), B58 (210), B58 (211),
    B58 (212), B58 (213), B58 (214), B58 (215),
    B58 (216), B58 (217), B58 (218), B58 (219),
    B58 (220), B58 (221), B58 (222), B58 (223),
    B58 (224), B58 (225), B58 (226), B58 (227),
    B58 (228), B58 (229), B58 (230), B58 (231),
    B58 (232), B58 (233), B58 (234), B58 (235),
    B58 (236), B58 (237), B58 (238), B58 (239),
    B58 (240), B58 (241), B58 (242), B58 (243),
    B58 (244), B58 (245), B58 (246), B58 (247),
    B58 (248), B58 (249), B58 (250), B58 (251),
    B58 (252), B58 (253), B58 (254), B58 (255)
};


#if UCHAR_MAX == 255
# define uchar_in_range(c) true
#else
# define uchar_in_range(c) ((c) <= 255)
#endif

bool coda_isbase58 (char ch)
{
    return uchar_in_range (to_uchar (ch)) && 0 <= b58[to_uchar (ch)];
}

bool coda_base58_decode (const char* in, size_t inlen, char* out, size_t *outlen)
{
    *outlen = 0;
    while (inlen && *in == '1')
    {
        *out++ = 0;
        (*outlen)++;

        in++;
        inlen--;
    }

    unsigned b256len = inlen * 733 / 1000 + 1; // log(58) / log(256), rounded up.
    char *b256_begin = NULL;
    char *b256_curr = NULL;
    b256_begin = b256_curr = (char*)calloc (b256len, sizeof(char));
    if (!b256_begin)
        return false;

    while (inlen)
    {
        if (!coda_isbase58 (*in))
            break;

        unsigned int carry = (unsigned char)b58[to_uchar (*in++)];
        int i;
        for (i = b256len - 1; i >= 0; i--)
        {
            carry += 58 * (unsigned char)b256_curr[i];
            b256_curr[i] = carry % 256;
            carry /= 256;
        }
        inlen--;
    }

    if (inlen != 0)
    {
        free(b256_begin);
        return false;
    }

    while (b256len && !*b256_curr)
    {
        b256_curr++;
        b256len--;
    }

    *outlen += b256len;
    while (b256len)
    {
        *out++ = *b256_curr++;
        b256len--;
    }

    free(b256_begin);
    return true;
}

bool coda_base58_decode_alloc (const char *in, size_t inlen, char **out, size_t *outlen)
{
    size_t zeroes = 0;
    size_t b256len = inlen;
    while (b256len && in[zeroes] == '1')
    {
        zeroes++;
        b256len--;
    }

    size_t needlen = zeroes + b256len * 733 / 1000 + 1;
    *out = malloc (needlen);
    if (!*out)
        return true;

    if (!coda_base58_decode (in, inlen, *out, &needlen))
    {
        free (*out);
        *out = NULL;
        return false;
    }

    if (outlen)
        *outlen = needlen;

    return true;
}
