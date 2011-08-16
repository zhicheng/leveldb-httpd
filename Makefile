all:
	g++ -o leveldb-httpd -I/usr/local/include -L/usr/local/lib -levent leveldb-httpd.cc -lleveldb -I./include
