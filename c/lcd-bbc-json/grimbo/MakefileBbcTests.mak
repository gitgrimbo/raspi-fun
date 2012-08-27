all:
	gcc -O2 -ljansson -o bbctests bbc_test.c bbc.c url.c str.c
