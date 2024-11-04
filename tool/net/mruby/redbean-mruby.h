int mrubyRunCode(char *code, char **outbuf);

static void mrubyStart(void);
static void mrubyInit(void);
static void mrubyDestroy(void);
static char* mrubyOnHttpRequest(void);

static bool mruby_has_on_http_request(void);

int test_mruby();

