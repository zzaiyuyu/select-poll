
.PHONY:all

all:select_server poll_server

select_server:select_server.c
	gcc -o $@ $^
poll_server:
