#
# Automated Testing Framework (atf)
#
# Copyright (c) 2007 The NetBSD Foundation, Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. All advertising materials mentioning features or use of this
#    software must display the following acknowledgement:
#        This product includes software developed by the NetBSD
#        Foundation, Inc. and its contributors.
# 4. Neither the name of The NetBSD Foundation nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND
# CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

main_head()
{
    atf_set "descr" "Tests that the require.config property works"
}
main_body()
{
    srcdir=$(atf_get_srcdir)
    h_cpp=${srcdir}/h_cpp
    h_sh=${srcdir}/h_sh

    for h in ${h_cpp} ${h_sh}; do
        atf_check "${h} -s ${srcdir} -r3 require_config 3>resout" \
                  0 ignore ignore
        atf_check 'grep "skipped" resout' 0 ignore null
        atf_check 'grep "var1 not defined" resout' 0 ignore null

        atf_check "${h} -s ${srcdir} -r3 -v var1=foo \
                   require_config 3>resout" 0 ignore ignore
        atf_check 'grep "skipped" resout' 0 ignore null
        atf_check 'grep "var2 not defined" resout' 0 ignore null

        atf_check "${h} -s ${srcdir} -r3 -v var1=foo -v var2=bar \
                   require_config 3>resout" 0 stdout ignore
        atf_check 'grep "passed" resout' 0 ignore null
        atf_check 'grep "var1: foo" stdout' 0 ignore null
        atf_check 'grep "var2: bar" stdout' 0 ignore null
    done
}

atf_init_test_cases()
{
    atf_add_test_case main
}

# vim: syntax=sh:expandtab:shiftwidth=4:softtabstop=4