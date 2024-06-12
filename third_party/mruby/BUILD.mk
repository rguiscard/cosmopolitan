#-*-mode:makefile-gmake;indent-tabs-mode:t;tab-width:8;coding:utf-8-*-┐
#── vi: set noet ft=make ts=8 sw=8 fenc=utf-8 :vi ────────────────────┘

PKGS += THIRD_PARTY_MRUBY

THIRD_PARTY_MRUBY_ARTIFACTS += THIRD_PARTY_MRUBY_A
THIRD_PARTY_MRUBY = $(THIRD_PARTY_MRUBY_A_DEPS) $(THIRD_PARTY_MRUBY_A)
THIRD_PARTY_MRUBY_A = o/$(MODE)/third_party/mruby/libmruby.a
THIRD_PARTY_MRUBY_A_FILES := $(wildcard third_party/mruby/*)
THIRD_PARTY_MRUBY_A_HDRS = $(filter %.h,$(THIRD_PARTY_MRUBY_A_FILES))
THIRD_PARTY_MRUBY_A_SRCS = $(filter %.c,$(THIRD_PARTY_MRUBY_A_FILES))
THIRD_PARTY_MRUBY_A_OBJS = $(THIRD_PARTY_MRUBY_A_SRCS:%.c=o/$(MODE)/%.o)

THIRD_PARTY_MRUBY_A_CHECKS =					\
	$(THIRD_PARTY_MRUBY_A).pkg				

THIRD_PARTY_MRUBY_A_DIRECTDEPS =				\

THIRD_PARTY_MRUBY_A_DEPS :=					\
	$(call uniq,$(foreach x,$(THIRD_PARTY_MRUBY_A_DIRECTDEPS),$($(x))))

$(THIRD_PARTY_MRUBY_A):						\
		third_party/mruby/				\
		$(THIRD_PARTY_MRUBY_A).pkg			\
                $(THIRD_PARTY_MRUBY_A_OBJS)

$(THIRD_PARTY_MRUBY_A).pkg:					\
		$(THIRD_PARTY_MRUBY_A_OBJS)			\
		$(foreach x,$(THIRD_PARTY_MRUBY_A_DIRECTDEPS),$($(x)_A).pkg)

$(THIRD_PARTY_MRUBY_A_OBJS):

THIRD_PARTY_MRUBY_LIBS = $(foreach x,$(THIRD_PARTY_MRUBY_ARTIFACTS),$($(x)))
THIRD_PARTY_MRUBY_SRCS = $(foreach x,$(THIRD_PARTY_MRUBY_ARTIFACTS),$($(x)_SRCS))
THIRD_PARTY_MRUBY_HDRS = $(foreach x,$(THIRD_PARTY_MRUBY_ARTIFACTS),$($(x)_HDRS))
THIRD_PARTY_MRUBY_CHECKS = $(foreach x,$(THIRD_PARTY_MRUBY_ARTIFACTS),$($(x)_CHECKS))
THIRD_PARTY_MRUBY_OBJS = $(foreach x,$(THIRD_PARTY_MRUBY_ARTIFACTS),$($(x)_OBJS))
$(THIRD_PARTY_MRUBY_OBJS): third_party/mruby/BUILD.mk

.PHONY: o/$(MODE)/third_party/mruby
o/$(MODE)/third_party/mruby: $(THIRD_PARTY_MRUBY_CHECKS) $(THIRD_PARTY_MRUBY_A_FILES)
