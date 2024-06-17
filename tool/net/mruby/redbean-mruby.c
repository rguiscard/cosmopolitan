#define USE_MRUBY
#define REDBEAN "redbean-mruby"

#include "tool/net/mruby/redbean-mruby.h"
#include "tool/net/redbean.c"

#include <mruby.h>
#include <mruby/compile.h>
#include <mruby/string.h>

static mrb_state *mrb;

static void mrubyStart(void) {
    mrb = mrb_open();
}

static void mrubyInit(void) {
}

static void mrubyDestroy(void) {
    mrb_close(mrb);
}

mrb_value run_mruby(char *code) {
    mrb_value result = mrb_load_string(mrb, code);

    if (mrb->exc) {
        mrb_print_error(mrb);
    } else {
        printf("=> %s\n", mrb_str_to_cstr(mrb, mrb_inspect(mrb, result)));
        /*
        if (mrb_nil_p(result)) {
        } else if (mrb_string_p(result)) {
            printf("=> %s\n", mrb_str_to_cstr(mrb, result));
        } else {
            printf("return other value\n");
        }
        */
    }

    return result;
}

int test_mruby()
{
    char code[] = "puts 9 + 9";
    run_mruby(code);

    return 0;
}
