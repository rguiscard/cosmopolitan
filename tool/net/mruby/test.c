#include <stdlib.h> 
#include <mruby.h>
#include <mruby/compile.h>
#include <mruby/string.h>

static mrb_state *mrb;

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
    mrb = mrb_open();

    char code[] = "puts 9 + 9";
    run_mruby(code);

    mrb_close(mrb);
    return 0; 
} 
