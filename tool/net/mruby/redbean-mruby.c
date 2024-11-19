#define USE_MRUBY
#define REDBEAN "redbean-mruby"

#include "tool/net/mruby/redbean-mruby.h"
#include "tool/net/redbean.c"

#include <mruby.h>
#include <mruby/compile.h>
#include <mruby/array.h>
#include <mruby/hash.h>
#include <mruby/string.h>

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
//      printf("mrubyRunAsset => %s\n", path);
//      printf("===\n %s\n ===\n", code);
//      printf("==============\n", code);
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

mrb_value redbean_load_asset(mrb_state* mrb, mrb_value self) {
  void *data;
  struct Asset *a;
  const char *path;
  size_t size, pathlen;

  mrb_get_args(mrb, "s", &path, &pathlen);
  printf("load_asset %s %d\n", path, pathlen);

  // redbean.c CheckPath
  if (pathlen == 0) {
    // not actually used in redbean.c
    path = url.path.p;
    pathlen = url.path.n;
  } else {
    if (!IsReasonablePath(path, pathlen)) {
      WARNF("(srvr) bad path %`'.*s", pathlen, path);
      // raise exception ?
      __builtin_unreachable();
      return self;
    }
  }

  if ((a = GetAsset(path, pathlen))) {
    if (!a->file && !IsCompressed(a)) {
      /* fast path: this avoids extra copy */
      data = ZIP_LFILE_CONTENT(zmap + a->lf);
      size = GetZipLfileUncompressedSize(zmap + a->lf);
      if (Verify(data, size, ZIP_LFILE_CRC32(zmap + a->lf))) {
//        lua_pushlstring(L, data, size);
        return mrb_str_new_cstr(mrb, data);
      }
      // any error from Verify has already been reported
    } else if ((data = LoadAsset(a, &size))) {
//      lua_pushlstring(L, data, size);
      mrb_value mdata = mrb_str_new_cstr(mrb, data);
      free(data);
      return mdata;
    } else {
      WARNF("(srvr) could not load asset: %`'.*s", pathlen, path);
    }
  } else {
    WARNF("(srvr) could not find asset: %`'.*s", pathlen, path);
  }
  return mrb_nil_value();
}

static char *getLuaResponse(void) {
  return cpm.luaheaderp ? cpm.luaheaderp : SetStatus(200, "OK");
}

// LuaRespond
static int respondWithCode(int code, const char **reason, char *R(unsigned, const char *)) {
  char *p;
  size_t reasonlen;
//  OnlyCallDuringRequest(L, "Respond");
  if (!(100 <= code && code <= 999)) {
    //luaL_argerror(L, 1, "bad status code");
    // raise error ?
    __builtin_unreachable();
  }

  if (*reason == NULL) {
    cpm.luaheaderp = R(code, GetHttpReason(code));
  } else {
    printf("reason not null\n");
  }
  return 0;

  // FIXME: need to handle -else-
  if (*reason == NULL) {
//  if (lua_isnoneornil(L, 2)) {
    printf("reason null\n");
    return 0;
    const char *t = GetHttpReason(code);
    printf("reason %s\n", t);
    return 0;
    cpm.luaheaderp = R(code, GetHttpReason(code));
  } else {
    printf("reason not null\n");
//    reason = lua_tolstring(L, 2, &reasonlen);
    reasonlen = strlen(*reason);
    if ((p = EncodeHttpHeaderValue(*reason, MIN(reasonlen, 128), 0))) {
      cpm.luaheaderp = R(code, p);
      free(p);
    } else {
      // raise error ?
      // luaL_argerror(L, 2, "invalid");
      __builtin_unreachable();
    }
  }
  return 0;
}

mrb_value redbean_set_status(mrb_state* mrb, mrb_value self) {
//  LuaRespond(L, SetStatus);
  printf("redbean_set_status\n");
  int code;
  const char *reason;
  mrb_get_args(mrb, "i|z!", &code, &reason);
  printf("(%d) %s\n", code, reason);
  respondWithCode(code, &reason, SetStatus);
  return self;
}

mrb_value redbean_set_header(mrb_state* mrb, mrb_value self) {
  int h;
  char *eval;
  char *p, *q;
  const char *key, *val;
  size_t keylen, vallen, evallen;
  // OnlyCallDuringRequest(L, "SetHeader");
  mrb_get_args(mrb, "ss!", &key, &keylen, &val, &vallen);
  printf("%s (%d): %s (%d)\n", key, keylen, val, vallen);
  if (vallen == 0) // if (!val)
    return self;
  if ((h = GetHttpHeader(key, keylen)) == -1) {
    if (!IsValidHttpToken(key, keylen)) {
      printf("invalid http token %s (%d)\n", key, keylen);
      // raise error (mrb_raise) ?
      __builtin_unreachable();
    }
  }
  if (!(eval = EncodeHttpHeaderValue(val, vallen, &evallen))) {
    // raise error (mrb_raise) ?
    __builtin_unreachable();
  }
  p = getLuaResponse(); // mruby version of GetLuaResponse()
  while (p - hdrbuf.p + keylen + 2 + evallen + 2 + 512 > hdrbuf.n) {
    hdrbuf.n += hdrbuf.n >> 1;
    q = xrealloc(hdrbuf.p, hdrbuf.n);
    cpm.luaheaderp = p = q + (p - hdrbuf.p);
    hdrbuf.p = q;
  }
  switch (h) {
    case kHttpConnection:
      connectionclose = SlicesEqualCase(eval, evallen, "close", 5);
      break;
    case kHttpContentType:
      p = AppendContentType(p, eval);
      break;
    case kHttpReferrerPolicy:
      cpm.referrerpolicy = FreeLater(strdup(eval));
      break;
    case kHttpServer:
      cpm.branded = true;
      p = AppendHeader(p, "Server", eval);
      break;
    case kHttpExpires:
    case kHttpCacheControl:
      cpm.gotcachecontrol = true;
      p = AppendHeader(p, key, eval);
      break;
    case kHttpXContentTypeOptions:
      cpm.gotxcontenttypeoptions = true;
      p = AppendHeader(p, key, eval);
      break;
    default:
      p = AppendHeader(p, key, eval);
      break;
  }
  cpm.luaheaderp = p;
  free(eval);
  return self;
}

mrb_value redbean_get_zip_paths(mrb_state* mrb, mrb_value self) {
  char *path;
  uint8_t *zcf;
  size_t n, pathlen, prefixlen;
  const char* prefix;
  if (mrb_get_argc(mrb) > 0) {
    mrb_get_args(mrb, "s", &prefix, &prefixlen);
  } else {
    prefix = "";
    prefixlen = 0;
  }
  printf("prefix %s %d\n", prefix, prefixlen);

  n = GetZipCdirRecords(zcdir);
  mrb_value m_ary = mrb_ary_new(mrb);
  for (zcf = zmap + GetZipCdirOffset(zcdir); n--;
       zcf += ZIP_CFILE_HDRSIZE(zcf)) {
    CHECK_EQ(kZipCfileHdrMagic, ZIP_CFILE_MAGIC(zcf));
    path = GetAssetPath(zcf, &pathlen);
    if (prefixlen == 0 || startswith(path, prefix)) {
//      printf("path %s\n", path);
      mrb_ary_push(mrb, m_ary, mrb_str_new_cstr(mrb, path));
//      lua_pushlstring(L, path, pathlen);
//      lua_seti(L, -2, ++i);
    }
    free(path);
  }
  return m_ary;
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
    mrb_define_method(mrb, mrb->kernel_module, "set_header", redbean_set_header, MRB_ARGS_REQ(2));
    mrb_define_method(mrb, mrb->kernel_module, "set_status", redbean_set_status, MRB_ARGS_REQ(2));

    mrb_define_method(mrb, mrb->kernel_module, "load_asset", redbean_load_asset, MRB_ARGS_REQ(1));
    mrb_define_method(mrb, mrb->kernel_module, "get_zip_paths", redbean_get_zip_paths, MRB_ARGS_OPT(1));
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

  if (mrb->exc) {
    mrb_print_backtrace(mrb);
  }

//  mrb_value result = mrb_load_string(mrb, "on_http_request");
  mrb_value result = mrb_funcall(mrb, mrb_top_self(mrb), "on_http_request", 0);
  data = string_from_mruby_value(result);
//  size_t size;
//  OnlyCallDuringRequest(L, "Write");
//  if (!lua_isnil(L, 1)) {
//    data = luaL_checklstring(L, 1, &size);
  appendd(&cpm.outbuf, data, strlen(data));
  UseOutput(); // Not sure this is required
  return CommitOutput(getLuaResponse());
}

int test_mruby()
{
    char code[] = "puts 9 + 9";
    run_mruby(code);

    return 0;
}
