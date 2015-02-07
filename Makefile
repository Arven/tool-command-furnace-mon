all: logger spinner
logger:
	gcc -std=gnu99 logger.c -o logger

spinner:
	openspin -I include spinner.spin

clean:
	rm logger spinner.binary 2> /dev/null || true
