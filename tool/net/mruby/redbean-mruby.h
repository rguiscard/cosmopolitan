int mrubyRunCode(char *code, char **outbuf);

static void mrubyStart(void);
static void mrubyInit(void);
static void mrubyDestroy(void);
static void mrubyOnHttpRequest(void);

int test_mruby();

