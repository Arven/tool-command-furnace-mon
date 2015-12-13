all: logger spinner xbuffer logfmt
logger:
	gcc -std=gnu99 logger.c -o logger

retrolog:
	gcc -g -std=gnu99 retrolog.c -o retrolog

xbuffer:
	gcc -std=gnu99 xbuffer.c -o xbuffer

spinner:
	openspin -I include spinner.spin

clean:
	rm logger spinner.binary 2> /dev/null || true

run: spinner
	p1load -r spinner.binary 
