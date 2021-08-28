#include <time.h>

#include "mongoose.h"
#include "webui/ti.h"
#include "webui/utils.h"

void sendTime(struct mg_connection *c)
{
	mg_http_reply(c, 200, "Content-Type: text/plain; charset=utf-8" HTTP_NEWLINE CACHE_CONTROL_NOCACHE, "%lu", time(NULL));
}
