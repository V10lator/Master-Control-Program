#pragma once

#include "mongoose.h"

#define HTTP_STRING "Master Control Program"

static inline void httpFastReply(struct mg_connection *c, int err)
{
	const char *str = mg_http_status_code_str(err);

	mg_http_reply(c, err, "Content-Type: text/html; charset=utf-8\r\n", "<html><head><title>%i %s</title></head><body><center><h1>%i %s</h1><hr>" HTTP_STRING  "</center></body></html><!-- We need some padding cause some browsers are stupid ant want to render their shit instead of this beautiful page we worked so much on... -->",
		err, str,
		err, str
	);
}

static inline void httpReplyJson(struct mg_connection *c, void *data) // TODO
{
	mg_http_reply(c, 200, "Content-Type: application/json\r\n", "TODO");
}
