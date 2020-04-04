.PHONY:all

all: pc

pc: pc.c
	gcc -o $@ $^ ../httpd.c -I ../

clean:
	@rm -rf pc
