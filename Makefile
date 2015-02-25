all: logger spinner xbuffer
logger:
	gcc -std=gnu99 logger.c -o logger

xbuffer:
	gcc -std=gnu99 xbuffer.c -o xbuffer

spinner:
	openspin -I include spinner.spin

clean:
	rm logger spinner.binary 2> /dev/null || true
