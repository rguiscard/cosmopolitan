#define USE_MRUBY
#define REDBEAN "redbean-mruby"

#include "tool/net/mruby/redbean-mruby.h"
#include "tool/net/redbean.c"

#include <mruby.h>
#include <mruby/compile.h>
#include <mruby/string.h>
#include <mruby/hash.h>

static mrb_state *mrb;
bool has_onhttprequest = false;

static bool mruby_has_on_http_request() {
  return has_onhttprequest;
}

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
//        printf("=> %s\n", string_from_mruby_value(result));
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
      printf("==============\n", code);
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

mrb_value redbean_get_path(mrb_state* mrb, mrb_value self) {
  return mrb_str_new(mrb, url.path.p, url.path.n);
}

mrb_value redbean_get_effective_path(mrb_state* mrb, mrb_value self) {
  return mrb_str_new(mrb, effectivepath.p, effectivepath.n);
}

mrb_value redbean_get_method(mrb_state* mrb, mrb_value self) {
  char method[9] = {0};
  WRITE64LE(method, cpm.msg.method);
  return mrb_str_new_cstr(mrb, method);
}

mrb_value redbean_get_body(mrb_state* mrb, mrb_value self) {
     printf("redbean_get_body! %d\n", payloadlength);
     return self;
}

mrb_value redbean_get_http_version(mrb_state* mrb, mrb_value self) {
  return mrb_fixnum_value(cpm.msg.version);
}

mrb_value redbean_get_scheme(mrb_state* mrb, mrb_value self) {
  if (url.scheme.p) {
    return mrb_str_new(mrb, url.scheme.p, url.scheme.n);
  } else {
    return mrb_nil_value();
  }
}

mrb_value redbean_get_params(mrb_state* mrb, mrb_value self) {
  struct UrlParams *h = &url.params;
  size_t i;
  mrb_value hash = mrb_hash_new(mrb);
  for (i = 0; i < h->n; ++i) {
    mrb_value k = mrb_str_new(mrb, h->p[i].key.p, h->p[i].key.n);
    if (h->p[i].val.p) {
      mrb_value v = mrb_str_new(mrb, h->p[i].val.p, h->p[i].val.n);
      mrb_hash_set(mrb, hash, k, v);
    } else {
      mrb_hash_set(mrb, hash, k, mrb_nil_value());
    }
  }
  // FIXME: if two identical keys exist, latter one will override former one.
  // http://test.dev/?bar=123&bar=456 => {bar: 456}. 123 will be override.
  return hash;
}

mrb_value redbean_get_header(mrb_state* mrb, mrb_value self) {
  int h;
  const char *key;
  size_t i, keylen;
  mrb_value arg;
  mrb_get_args(mrb, "S", &arg);
  key = string_from_mruby_value(arg);
  keylen = strlen(key);
//  OnlyCallDuringRequest(L, "GetHeader");
//  key = luaL_checklstring(L, 1, &keylen);
  if ((h = GetHttpHeader(key, keylen)) != -1) {
    if (cpm.msg.headers[h].a) {
//      return LuaPushHeader(L, &cpm.msg, inbuf.p, h);
      char *t;
      size_t m;
      mrb_value s;
      if (!kHttpRepeatable[h]) {
        t = DecodeLatin1(inbuf.p + cpm.msg.headers[h].a, cpm.msg.headers[h].b - cpm.msg.headers[h].a, &m); /// need to be freed
      } else {
        size_t vallen;
        char *val = FoldHeader(&cpm.msg, inbuf.p, h, &vallen);
        t = DecodeLatin1(val, vallen, &m); // need to be freed.
      }
      // printf("redbean_get_header! %d:%s\n", m, t);
      s = mrb_str_new_cstr(mrb, t); // possible nil string ?
      free(t);
      return s;
    }
  } else {
    for (i = 0; i < cpm.msg.xheaders.n; ++i) {
      char *t;
      size_t m;
      mrb_value s;
      if (SlicesEqualCase(
              key, keylen, inbuf.p + cpm.msg.xheaders.p[i].k.a,
              cpm.msg.xheaders.p[i].k.b - cpm.msg.xheaders.p[i].k.a)) {
	// need to be freed
	t = DecodeLatin1(inbuf.p + cpm.msg.xheaders.p[i].v.a, cpm.msg.xheaders.p[i].v.b - cpm.msg.xheaders.p[i].v.a, &m);
	s = mrb_str_new_cstr(mrb, t);
	free(t);
        return s;
      }
    }
  }
  return mrb_nil_value();
}

static void mrubyStart(void) {
    mrb = mrb_open();

    mrb_define_method(mrb, mrb->kernel_module, "get_body", redbean_get_body, MRB_ARGS_NONE());
    mrb_define_method(mrb, mrb->kernel_module, "get_path", redbean_get_path, MRB_ARGS_NONE());
    mrb_define_method(mrb, mrb->kernel_module, "get_effective_path", redbean_get_effective_path, MRB_ARGS_NONE());
    mrb_define_method(mrb, mrb->kernel_module, "get_method", redbean_get_method, MRB_ARGS_NONE());
    mrb_define_method(mrb, mrb->kernel_module, "get_params", redbean_get_params, MRB_ARGS_NONE());
    mrb_define_method(mrb, mrb->kernel_module, "get_scheme", redbean_get_scheme, MRB_ARGS_NONE());
    mrb_define_method(mrb, mrb->kernel_module, "get_http_version", redbean_get_http_version, MRB_ARGS_NONE());
    mrb_define_method(mrb, mrb->kernel_module, "get_header", redbean_get_header, MRB_ARGS_REQ(1));
}

static void mrubyInit(void) {
  mrubyRunAsset("/.init.mrb", true);

  if(mrb_obj_respond_to(mrb, mrb->object_class, mrb_intern_cstr(mrb, "on_http_request"))) {
    has_onhttprequest = true;
  }
}

static void mrubyDestroy(void) {
    mrb_close(mrb);
}

static char* mrubyOnHttpRequest(void) {
  const char *data = "Returned from OnHttpRequest";
  effectivepath.p = url.path.p;
  effectivepath.n = url.path.n;
  mrb_value result = mrb_load_string(mrb, "on_http_request");
  data = string_from_mruby_value(result);
//  size_t size;
//  OnlyCallDuringRequest(L, "Write");
//  if (!lua_isnil(L, 1)) {
//    data = luaL_checklstring(L, 1, &size);
  appendd(&cpm.outbuf, data, strlen(data));
//  }
  UseOutput();
  return CommitOutput(SetStatus(200, "OK"));
}

int test_mruby()
{
    char code[] = "puts 9 + 9";
    run_mruby(code);

    return 0;
}
