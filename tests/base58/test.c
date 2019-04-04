#include "coda/base58.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* If fails can return anything greater than 0xF */
static unsigned char to_nibble (char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + c - 'a';
    return 0xFF;
}

static int bytes_to_hex(const char* in, size_t inlen, char** out, size_t* outlen)
{
    static const char* hex = "0123456789abcdef";

    *outlen = inlen * 2 + 1;
    *out = (char*)malloc (*outlen);
    if (!*out)
    {
        *outlen = 0;
        return -1;
    }

    size_t i = 0;
    while(i < *outlen-1)
    {
        (*out)[i++] = hex[(*in >> 4) & 0xF];
        (*out)[i++] = hex[*in++ & 0xF];
    }
    (*out)[*outlen-1] = '\0';

    return 0;
}

static int hex_to_bytes (const char* in, char** out, size_t* outlen)
{
    size_t inlen = strlen (in);
    *outlen = inlen / 2;

    *out = (char*)malloc (*outlen);
    if (!*out)
    {
        *outlen = 0;
        return -1;
    }

    unsigned char high;
    size_t i = *outlen-1;
    while (inlen)
    {
        (*out)[i] = to_nibble (in[--inlen]);
        if ((*out)[i] > 16)
            break;
        if (!inlen)
            break;
        high = to_nibble (in[--inlen]);
        if (high > 16)
            break;
        (*out)[i--] += 16 * high;
    }

    if (inlen > 0)
    {
        free (*out);
        return -1;
    }

    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage:\n  %s <test_in.txt> <test_control.txt>", argv[0]);
        return -1;
    }

    FILE* file_in = fopen (argv[1], "r");
    if (file_in == NULL)
    {
        fprintf (stderr, "Cannot open file with test inputs.\n");
        return -1;
    }

    FILE* file_ctrl = fopen (argv[2], "r");
    if (file_ctrl == NULL)
    {
        fprintf(stderr, "Cannot open file with test inputs.\n");
        fclose(file_in);
        return -1;
    }
    
    char* line_in = NULL;
    size_t line_in_n = 0;
    ssize_t line_in_len = 0;

    char* line_ctrl = NULL;
    size_t line_ctrl_n = 0;
    ssize_t line_ctrl_len = 0;

    char* test_value = NULL;
    size_t test_value_len = 0;

    char* encoded = NULL;
    char* decoded = NULL;
    size_t decoded_len = 0;

    char* decoded_hex = NULL;
    size_t decoded_hex_len = 0;

    int exit_code = 0;

    while ((line_in_len = getline (&line_in, &line_in_n, file_in)) != -1)
    {
        if (line_in[line_in_len-1] == '\n')
            line_in[line_in_len-1] = '\0';

        if ((line_ctrl_len = getline (&line_ctrl, &line_ctrl_n, file_ctrl)) == -1)
        {
            fprintf(stderr, "Malformed test input: each test input shoud have corresponding output\n");
            exit_code = -1;
            break;
        }

        if (line_ctrl[line_ctrl_len-1] == '\n')
            line_ctrl[line_ctrl_len-1] = '\0';

        if (hex_to_bytes (line_in, &test_value, &test_value_len) != 0)
        {
            fprintf(stderr, "Malformed test input: input isn't hex string.\n");
            exit_code = -1;
            break;
        }

        coda_base58_encode_alloc (test_value, test_value_len, &encoded);

        if (strcmp (encoded, line_ctrl))
        {
            fprintf(stderr, "Test failed (encode): %s != %s\n", encoded, line_ctrl);
            exit_code = -1;
            break;
        }

        coda_base58_decode_alloc (encoded, strlen(encoded), &decoded, &decoded_len);
        bytes_to_hex (decoded, decoded_len, &decoded_hex, &decoded_hex_len);

        if (strcmp (decoded_hex, line_in))
        {
            fprintf(stderr, "Test failed (decode): %s != %s\n", decoded_hex, line_in);
            exit_code = -1;
            break;
        }

        free (test_value);
        free (decoded_hex);
        free (encoded);
        free (decoded);

        test_value = NULL;
        decoded_hex = NULL;
        encoded = NULL;
        decoded = NULL;
    }

    if (test_value)
        free (test_value);
    if (decoded_hex)
        free (decoded_hex);
    if (encoded)
        free (encoded);
    if (decoded)
        free (decoded);
    if (line_in)
        free (line_in);
    if (line_ctrl)
        free (line_ctrl);

    fclose (file_in);
    fclose (file_ctrl);

    return exit_code;
}
