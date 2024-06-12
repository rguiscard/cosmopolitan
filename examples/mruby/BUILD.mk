#-*-mode:makefile-gmake;indent-tabs-mode:t;tab-width:8;coding:utf-8-*-┐
#── vi: set noet ft=make ts=8 sw=8 fenc=utf-8 :vi ────────────────────┘

PKGS += EXAMPLES_MRUBY

EXAMPLES_MRUBY := $(wildcard examples/mruby/*)
EXAMPLES_MRUBY_SRCS = $(filter %.c,$(EXAMPLES_MRUBY_FILES))
EXAMPLES_MRUBY_HDRS = $(filter %.h,$(EXAMPLES_MRUBY_FILES))
EXAMPLES_MRUBY_INCS = $(filter %.inc,$(EXAMPLES_MRUBY_FILES))

EXAMPLES_MRUBY_OBJS =							\
	$(EXAMPLES_MRUBY:%.c=o/$(MODE)/%.o)

EXAMPLES_MRUBY_BINS =							\
	$(EXAMPLES_MRUBY_COMS)						\
	$(EXAMPLES_MRUBY_COMS:%=%.dbg)

EXAMPLES_MRUBY_COMS =							\
	o/$(MODE)/examples/mruby/hello					\

EXAMPLES_MRUBY_CHECKS =							\
	o/$(MODE)/examples/mruby/hello.pkg				\
	$(EXAMPLES_MRUBY_HDRS:%=o/$(MODE)/%.ok)

EXAMPLES_MRUBY_DIRECTDEPS =                                             \
        LIBC_CALLS                                                      \
        LIBC_FMT                                                        \
        LIBC_INTRIN                                                     \
        LIBC_LOG                                                        \
        LIBC_MEM                                                        \
        LIBC_NEXGEN32E                                                  \
        LIBC_NT_IPHLPAPI                                                \
        LIBC_NT_KERNEL32                                                \
        LIBC_NT_WS2_32                                                  \
        LIBC_PROC                                                       \
        LIBC_RUNTIME                                                    \
        LIBC_SOCK                                                       \
        LIBC_STDIO                                                      \
        LIBC_STR                                                        \
        LIBC_SYSV                                                       \
        LIBC_SYSV_CALLS                                                 \
        LIBC_THREAD                                                     \
        LIBC_TINYMATH                                                   \
        LIBC_X                                                          \
	THIRD_PARTY_MRUBY						\

EXAMPLES_MRUBY_DEPS :=							\
	$(call uniq,$(foreach x,$(EXAMPLES_MRUBY_DIRECTDEPS),$($(x))))

o/$(MODE)/examples/mruby/mruby.pkg:						\
		$(EXAMPLES_MRUBY_OBJS)					\
		$(foreach x,$(EXAMPLES_MRUBY_DIRECTDEPS),$($(x)_A).pkg)

o/$(MODE)/examples/mruby/%.dbg:						\
		$(EXAMPLES_MRUBY_DEPS)					\
		o/$(MODE)/examples/mruby/%.o					\
		o/$(MODE)/examples/mruby/mruby.pkg				\
		$(CRT)							\
		$(APE_NO_MODIFY_SELF)
	@$(APELINK)

o/$(MODE)/examples/mruby/hello.o: private                                  \
                CFLAGS += -I third_party/mruby/                                         

o/$(MODE)/examples/mruby/hello.dbg:						\
		$(EXAMPLES_MRUBY_DEPS)					\
		o/$(MODE)/examples/mruby/hello.o				\
		o/$(MODE)/examples/mruby/hello.pkg				\
		$(CRT)							\
		$(APE_NO_MODIFY_SELF)
	@$(APELINK)

.PHONY: o/$(MODE)/examples/mruby
o/$(MODE)/examples/mruby:							\
		$(EXAMPLES_MRUBY_BINS)					\
		$(EXAMPLES_MRUBY_CHECKS)
