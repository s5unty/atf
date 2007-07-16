//
// Automated Testing Framework (atf)
//
// Copyright (c) 2007 The NetBSD Foundation, Inc.
// All rights reserved.
//
// This code is derived from software contributed to The NetBSD Foundation
// by Julio M. Merino Vidal, developed as part of Google's Summer of Code
// 2007 program.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. All advertising materials mentioning features or use of this
//    software must display the following acknowledgement:
//        This product includes software developed by the NetBSD
//        Foundation, Inc. and its contributors.
// 4. Neither the name of The NetBSD Foundation nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND
// CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
// IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
// IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#include <atf.hpp>

#include "atfprivate/expand.hpp"

ATF_TEST_CASE(matches_glob_plain);
ATF_TEST_CASE_HEAD(matches_glob_plain)
{
    set("descr", "Tests the matches_glob function by using plain text "
                 "strings as patterns only.");
}
ATF_TEST_CASE_BODY(matches_glob_plain)
{
    ATF_CHECK( atf::matches_glob("", ""));
    ATF_CHECK(!atf::matches_glob("a", ""));
    ATF_CHECK(!atf::matches_glob("", "a"));

    ATF_CHECK( atf::matches_glob("ab", "ab"));
    ATF_CHECK(!atf::matches_glob("abc", "ab"));
    ATF_CHECK(!atf::matches_glob("ab", "abc"));
}

ATF_TEST_CASE(matches_glob_star);
ATF_TEST_CASE_HEAD(matches_glob_star)
{
    set("descr", "Tests the matches_glob function by using the '*' meta-"
                 "character as part of the pattern.");
}
ATF_TEST_CASE_BODY(matches_glob_star)
{
    ATF_CHECK( atf::matches_glob("*", ""));
    ATF_CHECK( atf::matches_glob("*", "a"));
    ATF_CHECK( atf::matches_glob("*", "ab"));

    ATF_CHECK(!atf::matches_glob("a*", ""));
    ATF_CHECK( atf::matches_glob("a*", "a"));
    ATF_CHECK( atf::matches_glob("a*", "aa"));
    ATF_CHECK( atf::matches_glob("a*", "ab"));
    ATF_CHECK( atf::matches_glob("a*", "abc"));
    ATF_CHECK(!atf::matches_glob("a*", "ba"));

    ATF_CHECK( atf::matches_glob("*a", "a"));
    ATF_CHECK( atf::matches_glob("*a", "ba"));
    ATF_CHECK(!atf::matches_glob("*a", "bc"));
    ATF_CHECK(!atf::matches_glob("*a", "bac"));

    ATF_CHECK( atf::matches_glob("*ab", "ab"));
    ATF_CHECK( atf::matches_glob("*ab", "aab"));
    ATF_CHECK( atf::matches_glob("*ab", "aaab"));
    ATF_CHECK( atf::matches_glob("*ab", "bab"));
    ATF_CHECK(!atf::matches_glob("*ab", "bcb"));
    ATF_CHECK(!atf::matches_glob("*ab", "bacb"));

    ATF_CHECK( atf::matches_glob("a*b", "ab"));
    ATF_CHECK( atf::matches_glob("a*b", "acb"));
    ATF_CHECK( atf::matches_glob("a*b", "acdeb"));
    ATF_CHECK(!atf::matches_glob("a*b", "acdebz"));
    ATF_CHECK(!atf::matches_glob("a*b", "zacdeb"));
}

ATF_TEST_CASE(matches_glob_question);
ATF_TEST_CASE_HEAD(matches_glob_question)
{
    set("descr", "Tests the matches_glob function by using the '?' meta-"
                 "character as part of the pattern.");
}
ATF_TEST_CASE_BODY(matches_glob_question)
{
    ATF_CHECK(!atf::matches_glob("?", ""));
    ATF_CHECK( atf::matches_glob("?", "a"));
    ATF_CHECK(!atf::matches_glob("?", "ab"));

    ATF_CHECK( atf::matches_glob("?", "b"));
    ATF_CHECK( atf::matches_glob("?", "c"));

    ATF_CHECK( atf::matches_glob("a?", "ab"));
    ATF_CHECK( atf::matches_glob("a?", "ac"));
    ATF_CHECK(!atf::matches_glob("a?", "ca"));

    ATF_CHECK( atf::matches_glob("???", "abc"));
    ATF_CHECK( atf::matches_glob("???", "def"));
    ATF_CHECK(!atf::matches_glob("???", "a"));
    ATF_CHECK(!atf::matches_glob("???", "ab"));
    ATF_CHECK(!atf::matches_glob("???", "abcd"));
}

ATF_TEST_CASE(expand_glob_base);
ATF_TEST_CASE_HEAD(expand_glob_base)
{
    set("descr", "Tests the expand_glob function with random patterns.");
}
ATF_TEST_CASE_BODY(expand_glob_base)
{
    std::set< std::string > candidates;
    candidates.insert("foo");
    candidates.insert("bar");
    candidates.insert("baz");
    candidates.insert("foobar");
    candidates.insert("foobarbaz");
    candidates.insert("foobarbazfoo");

    std::set< std::string > exps;

    exps = atf::expand_glob("foo", candidates);
    ATF_CHECK_EQUAL(exps.size(), 1);
    ATF_CHECK(exps.find("foo") != exps.end());

    exps = atf::expand_glob("bar", candidates);
    ATF_CHECK_EQUAL(exps.size(), 1);
    ATF_CHECK(exps.find("bar") != exps.end());

    exps = atf::expand_glob("foo*", candidates);
    ATF_CHECK_EQUAL(exps.size(), 4);
    ATF_CHECK(exps.find("foo") != exps.end());
    ATF_CHECK(exps.find("foobar") != exps.end());
    ATF_CHECK(exps.find("foobarbaz") != exps.end());
    ATF_CHECK(exps.find("foobarbazfoo") != exps.end());

    exps = atf::expand_glob("*foo", candidates);
    ATF_CHECK_EQUAL(exps.size(), 2);
    ATF_CHECK(exps.find("foo") != exps.end());
    ATF_CHECK(exps.find("foobarbazfoo") != exps.end());

    exps = atf::expand_glob("*foo*", candidates);
    ATF_CHECK_EQUAL(exps.size(), 4);
    ATF_CHECK(exps.find("foo") != exps.end());
    ATF_CHECK(exps.find("foobar") != exps.end());
    ATF_CHECK(exps.find("foobarbaz") != exps.end());
    ATF_CHECK(exps.find("foobarbazfoo") != exps.end());

    exps = atf::expand_glob("ba", candidates);
    ATF_CHECK_EQUAL(exps.size(), 0);

    exps = atf::expand_glob("ba*", candidates);
    ATF_CHECK_EQUAL(exps.size(), 2);
    ATF_CHECK(exps.find("bar") != exps.end());
    ATF_CHECK(exps.find("baz") != exps.end());

    exps = atf::expand_glob("*ba", candidates);
    ATF_CHECK_EQUAL(exps.size(), 0);

    exps = atf::expand_glob("*ba*", candidates);
    ATF_CHECK_EQUAL(exps.size(), 5);
    ATF_CHECK(exps.find("bar") != exps.end());
    ATF_CHECK(exps.find("baz") != exps.end());
    ATF_CHECK(exps.find("foobar") != exps.end());
    ATF_CHECK(exps.find("foobarbaz") != exps.end());
    ATF_CHECK(exps.find("foobarbazfoo") != exps.end());
}

ATF_TEST_CASE(expand_glob_tps);
ATF_TEST_CASE_HEAD(expand_glob_tps)
{
    set("descr", "Tests the expand_glob function with patterns that match "
                 "typical test program names.  This is just a subcase "
                 "of expand_base, but it is nice to make sure that "
                 "it really works.");
}
ATF_TEST_CASE_BODY(expand_glob_tps)
{
    std::set< std::string > candidates;
    candidates.insert("Atffile");
    candidates.insert("h_foo");
    candidates.insert("t_foo");
    candidates.insert("t_bar");
    candidates.insert("t_baz");
    candidates.insert("foo_helper");
    candidates.insert("foo_test");
    candidates.insert("bar_test");
    candidates.insert("baz_test");

    std::set< std::string > exps;

    exps = atf::expand_glob("t_*", candidates);
    ATF_CHECK_EQUAL(exps.size(), 3);
    ATF_CHECK(exps.find("t_foo") != exps.end());
    ATF_CHECK(exps.find("t_bar") != exps.end());
    ATF_CHECK(exps.find("t_baz") != exps.end());

    exps = atf::expand_glob("*_test", candidates);
    ATF_CHECK_EQUAL(exps.size(), 3);
    ATF_CHECK(exps.find("foo_test") != exps.end());
    ATF_CHECK(exps.find("bar_test") != exps.end());
    ATF_CHECK(exps.find("baz_test") != exps.end());
}

ATF_INIT_TEST_CASES(tcs)
{
    tcs.push_back(&matches_glob_plain);
    tcs.push_back(&matches_glob_star);
    tcs.push_back(&matches_glob_question);
    tcs.push_back(&expand_glob_base);
    tcs.push_back(&expand_glob_tps);
}