#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>

#include <event2/event.h>
#include <event2/http.h>
#include <event2/http_struct.h>
#include <event2/buffer.h>
#include <sys/queue.h>

#include <assert.h>
#include <iostream>
#include "leveldb/db.h"

#define SERVER_SIGNATURE "leveldb-http server v 0.0.1"

leveldb::DB *db;

