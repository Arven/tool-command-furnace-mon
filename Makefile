all: logger spinner xbuffer logfmt retrolog
logger: logger.c
	gcc -std=gnu99 logger.c -o logger

retrolog: retrolog.c
	gcc -g -std=gnu99 retrolog.c -o retrolog

xbuffer: xbuffer.c
	gcc -std=gnu99 xbuffer.c -o xbuffer

spinner: spinner.spin
	openspin -I include spinner.spin

clean:
	rm logger spinner.binary 2> /dev/null || true

run: spinner
	p1load -r spinner.binary 
