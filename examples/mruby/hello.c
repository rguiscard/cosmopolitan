#include <stdlib.h> 
#include <mruby.h>
#include <mruby/compile.h>
#include <mruby/string.h>

static mrb_state *mrb;

mrb_value add_int_method(mrb_state *mrb, mrb_value self)
{
  mrb_int a = 0;
  mrb_int b = 0;

  mrb_get_args(mrb, "ii", &a, &b);
  printf("%d + %d = ", a, b);

  return mrb_fixnum_value(a+b);
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

void run_fork() 
{ 
    int x = 1; 
    pid_t p = fork(); 
      if(p<0){ 
      perror("fork fail"); 
      exit(1); 
    } 
    else if (p == 0) 
        printf("Child = %d\n", getpid()); 
    else 
        printf("Parent = %d\n", getpid()); 

    printf("x = %d\n", x);

    //char code[] = "a = 1; a = a + 1; puts a; puts '\n'";
    char code[] = "$a = $a + 1;\\\n";
    run_mruby(code);

    char next[] = "puts \"$a = #{$a}\\\n\"";
    run_mruby(next);
} 

int main() 
{ 
    mrb = mrb_open();
    mrb_define_method(mrb, mrb->kernel_module, "add_int", add_int_method, MRB_ARGS_REQ(2));

    char code[] = "$a = 9";
    run_mruby(code);

    run_fork(); 

    mrb_close(mrb);
    return 0; 
} 
