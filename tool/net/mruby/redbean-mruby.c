#define USE_MRUBY
#define REDBEAN "redbean-mruby"

#include "tool/net/mruby/redbean-mruby.h"
#include "tool/net/redbean.c"

#include <mruby.h>
#include <mruby/compile.h>
#include <mruby/string.h>

static mrb_state *mrb;

char* string_from_mruby_value(mrb_value result) {
  char *output = NULL;
  if (mrb_nil_p(result)) {
    output = ""; // nil
  } else if (mrb_string_p(result)) {
    // string
    output = mrb_str_to_cstr(mrb, result);
  } else {
    // others
    output = mrb_str_to_cstr(mrb, mrb_inspect(mrb, result));
  }
  return output;
}

mrb_value run_mruby(char *code) {
    mrb_value result = mrb_load_string(mrb, code);

    if (mrb->exc) {
        mrb_print_error(mrb);
    } else {
        printf("=> %s\n", string_from_mruby_value(result));
    }

    return result;
}

static bool mrubyRunAsset(const char *path, bool mandatory) {
  int status;
  struct Asset *a;
  /*const*/ char *code;
  size_t pathlen, codelen;
  pathlen = strlen(path);
  if ((a = GetAsset(path, pathlen))) {
    if ((code = FreeLater(LoadAsset(a, &codelen)))) {
      printf("mrubyRunAsset => %s\n", path);
      printf("===\n %s\n ===\n", code);
//      lua_State *L = GL;
      effectivepath.p = (void *)path;
      effectivepath.n = pathlen;
      mrb_value result = run_mruby(code);
      if (mrb->exc) {
          mrb_print_error(mrb);
          if (mandatory) {
            exit(1);
          }
      } else {
          printf("=> %s\n", string_from_mruby_value(result));
      }
////      DEBUGF("(lua) LuaRunAsset(%`'s)", path);
//      status = luaL_loadbuffer(
//          L, code, codelen,
//          FreeLater(xasprintf("@%s%s", a->file ? "" : "/zip", path)));
//      if (status != LUA_OK || LuaCallWithTrace(L, 0, 0, NULL) != LUA_OK) {
//        LogLuaError("lua code", lua_tostring(L, -1));
//        lua_pop(L, 1);  // pop error
//        if (mandatory)
//          exit(1);
//      }
    }
  }
  return !!a;
}

int mrubyRunCode(char *code, char **outbuf) {
    mrb_value result = mrb_load_string(mrb, code);

    if (mrb->exc) {
        mrb_print_error(mrb);
    } else {
	char *output;
        if (mrb_nil_p(result)) {
	    output = ""; // nil
        } else if (mrb_string_p(result)) {
	    // string
            output = mrb_str_to_cstr(mrb, result);
        } else {
            // others
            output = mrb_str_to_cstr(mrb, mrb_inspect(mrb, result));
        }
        appends(outbuf, output);
    }

    return 0;
}

static void mrubyStart(void) {
    mrb = mrb_open();
}

static void mrubyInit(void) {
  mrubyRunAsset("/.init.mrb", true);
}

static void mrubyDestroy(void) {
    mrb_close(mrb);
}

static void mrubyOnHttpRequest(void) {
    printf("onHttpRequest => \n");
}

int test_mruby()
{
    char code[] = "puts 9 + 9";
    run_mruby(code);

    return 0;
}
