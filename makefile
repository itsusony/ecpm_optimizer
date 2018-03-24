build:
	cd src; gcc -Wall -levent -o ../bin/ecpm_optimizer ecpm.c; cd -
clean:
	rm -f bin/*
