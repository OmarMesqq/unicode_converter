BINARY:=utfConverter

debug:
	gcc -g -Wall -Wextra main.c convutf.c utils.c -o $(BINARY)

release:
	gcc -O3 -Wall -Wextra main.c convutf.c utils.c -o $(BINARY)
	