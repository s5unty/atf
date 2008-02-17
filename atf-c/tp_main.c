/*
 * Automated Testing Framework (atf)
 *
 * Copyright (c) 2008 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgement:
 *        This product includes software developed by the NetBSD
 *        Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND
 * CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dynstr.h"
#include "expand.h"
#include "object.h"
#include "sanity.h"
#include "tc.h"
#include "tp.h"
#include "ui.h"

static void usage(void);
static void usage_error(const char *, ...);

static
int
print_tag_ap(const char *tag, bool repeat, size_t col,
             const char *fmt, va_list ap)
{
    int ret;
    atf_dynstr_t dest;

    atf_dynstr_init(&dest);

    if (atf_ui_format_ap(&dest, tag, repeat, col, fmt, ap) != 0) {
        ret = 0;
        goto out;
    }

    ret = printf("%s\n", atf_dynstr_cstring(&dest));

out:
    atf_dynstr_fini(&dest);
    return ret;
}

static
int
print_tag(const char *tag, bool repeat, size_t col, const char *fmt, ...)
{
    int ret;
    va_list ap;

    va_start(ap, fmt);
    ret = print_tag_ap(tag, repeat, col, fmt, ap);
    va_end(ap);

    return ret;
}

static
int
print(const char *msg)
{
    return print_tag("", false, 0, msg);
}

static
void
print_error_ap(const char *fmt, va_list ap)
{
    atf_dynstr_t tag;

    if (atf_dynstr_init_fmt(&tag, "%s: ERROR: ", getprogname()) != 0)
        atf_dynstr_init(&tag);

    print_tag_ap(atf_dynstr_cstring(&tag), true, 0, fmt, ap);

    atf_dynstr_fini(&tag);
}

static
void
print_error(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    print_error(fmt, ap);
    va_end(ap);
}

static
void
usage(void)
{
    print_tag("Usage: ", false, 0,
              "%s [options] [test_case1 [.. test_caseN]]",
              getprogname());
    printf("\n");
    print("This is an independent atf test program.");
    printf("\n");
    print("Available options:");
    print_tag("    -h              ", false, 0,
              "Shows this help message");
    print_tag("    -l              ", false, 0,
              "List test cases and their purpose");
    print_tag("    -r fd           ", false, 0,
              "The file descriptor to which the test program "
              "will send the results of the test cases");
    print_tag("    -s srcdir       ", false, 0,
              "Directory where the test's data files are "
              "located");
    print_tag("    -v var=value    ", false, 0,
              "Sets the configuration variable `var' to `value'");
    printf("\n");
    print("For more details please see atf-test-program(1) and atf(7).");
}

static
void
usage_error(const char *fmt, ...)
{
    va_list ap;
    atf_dynstr_t tag;

    va_start(ap, fmt);
    print_error_ap(fmt, ap);
    va_end(ap);

    atf_dynstr_init_fmt(&tag, "%s: ", getprogname());
    print_tag(atf_dynstr_cstring(&tag), true, 0,
              "Type `%s -h' for more details.", getprogname());
    atf_dynstr_fini(&tag);
}

static
bool
parse_rflag(const char *arg, int *value)
{
    if (strlen(arg) != 1 || !isdigit(arg[0])) {
        usage_error("Invalid value for -r; must be a single digit.");
        return false;
    }

    *value = arg[0] - '0';

    return true;
}

static
atf_error_t
match_tcs(const atf_tp_t *tp, const char *glob, atf_list_t *ids)
{
    atf_error_t err;
    atf_list_citer_t iter;

    err = atf_no_error();
    atf_list_for_each_c(iter, atf_tp_get_tcs(tp)) {
        const atf_tc_t *tc = atf_list_citer_data(iter);
        const char *ident = atf_tc_get_ident(tc);

        if (atf_expand_is_glob(glob)) {
            bool matches;

            atf_expand_matches_glob(glob, ident, &matches);
            if (matches)
                err = atf_list_append(ids, strdup(ident)); // XXX Leak
        } else {
            if (strcmp(glob, tc->m_ident) == 0)
                err = atf_list_append(ids, strdup(ident)); // XXX Leak
        }

        if (atf_is_error(err))
            break;
    }

    return err;
}

static
atf_error_t
filter_tcs(const atf_tp_t *tp, const atf_list_t *globs, atf_list_t *ids)
{
    atf_error_t err;
    atf_list_citer_t iter;

    err = atf_list_init(ids);
    if (atf_is_error(err))
        goto out;

    atf_list_for_each_c(iter, globs) {
        const char *glob = atf_list_citer_data(iter);
        err = match_tcs(tp, glob, ids);
        if (atf_is_error(err)) {
            atf_list_fini(ids);
            goto out;
        }
    }

out:
    return err;
}

static
int
list_tcs(atf_tp_t *tp, atf_list_t *tcnames)
{
    atf_list_t tcs;
    atf_list_iter_t iter;
    size_t col;

    filter_tcs(tp, tcnames, &tcs);

    col = 0;
    atf_list_for_each(iter, &tcs) {
        atf_tc_t *tc = atf_list_iter_data(iter);
        const size_t len = strlen(tc->m_ident);

        if (col < len)
            col = len;
    }
    col += 4;

    atf_list_for_each(iter, &tcs) {
        atf_tc_t *tc = atf_list_iter_data(iter);
        const atf_dynstr_t *descr = atf_tc_get_var(tc, "descr");

        print_tag(tc->m_ident, false, col, "%s", atf_dynstr_cstring(descr));
    }

    atf_list_fini(&tcs);

    return 0;
}

int
atf_tp_main(int argc, char **argv, int (*add_tcs_hook)(atf_tp_t *))
{
    bool lflag;
    int ch, ret;
    atf_tp_t tp;
    int fd;

    atf_init_objects();

    atf_tp_init(&tp);

    fd = STDOUT_FILENO;
    lflag = false;
    while ((ch = getopt(argc, argv, ":hlr:s:v:")) != -1) {
        switch (ch) {
        case 'h':
            usage();
            ret = EXIT_SUCCESS;
            goto out;

        case 'l':
            lflag = true;
            break;

        case 'r':
            {
                if (!parse_rflag(optarg, &fd)) {
                    ret = EXIT_FAILURE;
                    goto out;
                }
            }
            break;

        case 's':
            break;

        case 'v':
            usage_error("foo");
            ret = EXIT_FAILURE;
            goto out;

        case ':':
            break;

        case '?':
        default:
            usage_error("Unknown option -%c.", optopt);
            ret = EXIT_FAILURE;
            goto out;
        }
    }
    argc -= optind;
    argv += optind;

    add_tcs_hook(&tp); /* XXX Handle error */

    {
        atf_list_t tcnames;
        char **argptr;
        char all[] = "*";

        atf_list_init(&tcnames);
        if (argc == 0) {
            atf_list_append(&tcnames, all);
        } else {
            for (argptr = argv; *argptr != NULL; argptr++)
                atf_list_append(&tcnames, *argptr);
        }
        if (lflag)
            ret = list_tcs(&tp, &tcnames);
        else {
            size_t failed;
            atf_list_t matched;

            filter_tcs(&tp, &tcnames, &matched);
            atf_tp_run(&tp, &matched, fd, &failed);
            atf_list_fini(&matched);
            ret = failed > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
        }
        atf_list_fini(&tcnames);
    }

out:
    atf_tp_fini(&tp);

    return ret;
}
