#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <cstring>
#include <inttypes.h>
#include <coda/url.hpp>
#include <coda/url_parser.hpp>
#include <coda/attr.h>

#define URLBUF (8192)
#define ENCBUF (URLBUF * 3 + 1)

#define URLQ_UPARSE  1
#define URLQ_USPLIT  2
#define URLQ_DECODE  3
#define URLQ_ENCODE  4

typedef struct urlq_getopt urlq_getopt_t;
typedef int (* urlq_handle_t) (const char *url, size_t sz, const urlq_getopt_t *opts);

struct urlq_getopt
{
    unsigned action:3;
    unsigned domain:1;
    unsigned uflags:2;

    coda_url parent;
    urlq_handle_t handle;
};

static struct option options [] =
{
    { "decode"       , 0, NULL, 'd' },
    { "encode"       , 0, NULL, 'e' },
    { "parse"        , 0, NULL, 'p' },
    { "refer"        , 1, NULL, 'r' },
    { "split"        , 0, NULL, 's' },
    { "print-domain" , 0, NULL, 'P' },
    { "refer-domain" , 0, NULL, 'R' },
    { "fetch-domain" , 0, NULL, 'S' },
    {  NULL          , 0, NULL,  0  },
};

int urlq_getopt_usage(int argc CODA_UNUSED, char **argv)
{
    fprintf(stderr,
        "Usage:                                                    \n"
        "  %s -d [URL]...                                          \n"
        "  %s -e [URL]...                                          \n"
        "  %s -p[PRSr:s] [URL]...                                  \n"
        "                                                          \n"
        "Options:                                                  \n"
        "  -d, --decode       : urldecode url                      \n"
        "  -e, --encode       : urlencode url                      \n"
        "                                                          \n"
        "  -p, --parse        : parse url                          \n"
        "  -r, --refer  <url> : use <url> as refer                 \n"
        "  -s, --split        : split url (UNIMPLEMENTED)          \n"
        "                                                          \n"
        "  -P, --print-domain : print domain                       \n"
        "  -R, --refer-domain : refer in the same domain           \n"
        "  -S, --slash-normal : slash at the end makes urls differ \n"
        "                                                          \n"
    , argv[0], argv[0], argv[0]);

    return 0;
}

int urlq_getopt_parse(int argc, char **argv, urlq_getopt_t *opts)
{
    int c;

    while (-1 != (c = getopt_long(argc, argv, "depr:sPRS", options, NULL)))
    {
        switch (c)
        {
            case 'd': opts->action = URLQ_DECODE; break;
            case 'e': opts->action = URLQ_ENCODE; break;
            case 'p': opts->action = URLQ_UPARSE; break;
            case 's': opts->action = URLQ_USPLIT; break;

            case 'r':
                if (0 != opts->parent.create_absolute(optarg, strlen(optarg), CODA_URL_FETCHABLE))
                {
                    fprintf(stderr, "%s: can't parse --refer argument: %s\n",
                        argv[0], optarg);

                    return -1;
                }
                break;

            case 'P': opts->domain = 1; break;
            case 'R': opts->uflags |= CODA_URL_SUBDOMAIN; break;
            case 'S': opts->uflags |= CODA_URL_FETCHABLE; break;

            default:
                return -1;
        }
    }

    return 0;
}

int urlq_decode(const char *url, size_t sz, const urlq_getopt_t *opts CODA_UNUSED)
{
    char out [URLBUF];
    coda_urldec(out, url, sz);

    printf("%s\n", out);
    return 0;
}

int urlq_encode(const char *url, size_t sz, const urlq_getopt_t *opts CODA_UNUSED)
{
    char out [ENCBUF];
    coda_urlenc(out, url, sz);

    printf("%s\n", out);
    return 0;
}

int urlq_uparse(const char *url, size_t sz, const urlq_getopt_t *opts)
{
    int rc;
    coda_url out;

    if (opts->parent.empty())
    {
        rc = out.create_absolute(url, sz, opts->uflags);
    }
    else
    {
        rc = out.create_relative(url, sz, opts->uflags, opts->parent);
    }

    if (0 != rc)
    {
        return -1;
    }

    printf("%s%s%s\n", opts->domain ? out.get_domain() : "",
        opts->domain ? "|" : "", out.get_url());

    return 0;
}

int urlq_usplit(const char *url CODA_UNUSED, size_t sz CODA_UNUSED, const urlq_getopt_t *opts CODA_UNUSED)
{
    return 0;
}

int urlq_load(urlq_getopt_t *opts)
{
    switch (opts->action)
    {
        case URLQ_DECODE: opts->handle = urlq_decode; break;
        case URLQ_ENCODE: opts->handle = urlq_encode; break;
        case URLQ_UPARSE: opts->handle = urlq_uparse; break;

        default:
            return -1;
    }

    return 0;
}

int main(int argc, char **argv)
{
    int i;

    urlq_getopt_t opts;
    std::memset(static_cast<void*>(&opts), 0, sizeof(opts));

    if (0 != urlq_getopt_parse(argc, argv, &opts))
    {
        urlq_getopt_usage(argc, argv);
        exit(EXIT_FAILURE);
    }

    if (0 != urlq_load(&opts))
    {
        urlq_getopt_usage(argc, argv);
        exit(EXIT_FAILURE);
    }

    for (i = optind; i < argc; ++i)
    {
        opts.handle(argv[i], strlen(argv[i]), &opts);
    }

    if (optind < argc)
    {
        return 0;
    }

    while (!feof(stdin) && !ferror(stdin))
    {
        char url [URLBUF];
        size_t sz;

        if (NULL == fgets(url, URLBUF, stdin))
        {
            break;
        }

        sz = strcspn(url, "\r\n");  /* optional */
        url[sz] = 0;                /* optional */

        opts.handle(url, sz, &opts);
    }

    return 0;
}

