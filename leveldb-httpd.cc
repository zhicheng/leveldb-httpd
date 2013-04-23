#include "leveldb-httpd.h"

void 
kv(struct evhttp_request *req, void *arg)
{
	struct evbuffer *evbuff;
	const char *uri = evhttp_request_get_uri(req);
	const char *path = &uri[1];
	std::string key(path);
	std::string value;

	struct evkeyvalq *output_headers = evhttp_request_get_output_headers(req);
	evhttp_add_header(output_headers, "Content-Type", "text/plain; charset=UTF-8");
	evhttp_add_header(output_headers, "Server", SERVER_SIGNATURE);

	if (req->type == EVHTTP_REQ_GET) {
		leveldb::Status s = db->Get(leveldb::ReadOptions(), key, &value);
		if (!s.ok()) {
			evhttp_send_error(req, HTTP_NOTFOUND, "NOT FOUND");
			return;
		}
		evbuff = evbuffer_new();
		assert(evbuff != NULL);
		evbuffer_add_printf(evbuff, "%s", value.c_str());
		evhttp_send_reply(req, HTTP_OK, "OK", evbuff);
		evbuffer_free(evbuff);
	} else if (req->type == EVHTTP_REQ_POST || req->type == EVHTTP_REQ_PUT) {
		unsigned char *input_buffer = evbuffer_pullup(req->input_buffer, -1);
		size_t len = evbuffer_get_length(req->input_buffer);
		char *buffer = (char *)malloc(len + 1);
		memcpy(buffer, input_buffer, len);
		buffer[len] = 0;

		value = buffer;

		leveldb::Status s = db->Put(leveldb::WriteOptions(), key, value);
		free(buffer);

		if (!s.ok()) {
			evhttp_send_error(req, HTTP_NOTFOUND, "NOT FOUND");
			return;
		}
		evhttp_send_reply(req, HTTP_OK, "OK", NULL);
	} else if (req->type == EVHTTP_REQ_DELETE) {
		leveldb::Status s = db->Delete(leveldb::WriteOptions(), key);
		if (!s.ok()) {
			evhttp_send_error(req, HTTP_NOTFOUND, "NOT FOUND");
			return;
		}
		evhttp_send_reply(req, HTTP_OK, "OK", NULL);
	} else {
		evhttp_send_error(req, HTTP_BADREQUEST, "BADREQUEST");
	}
	//fprintf(stderr, "end\n");
}

int main(int argc, char **argv)
{
	signal(SIGPIPE, SIG_IGN);
	leveldb::Options dboptions;
	dboptions.create_if_missing = true;
	leveldb::Status status = leveldb::DB::Open(dboptions, "test.db", &db);
	assert(status.ok());

	struct event_base *evbase = NULL;
	struct evhttp *evhttp = NULL;

	evbase = event_base_new();
	evhttp = evhttp_new(evbase);

	evhttp_bind_socket(evhttp, "0.0.0.0", 8080);
	evhttp_set_timeout(evhttp, 100);
	evhttp_set_gencb(evhttp, kv, NULL);

	event_base_dispatch(evbase);

	evhttp_free(evhttp);
	delete db;

	return 0;
}
