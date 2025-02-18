build: UVagrep.c
	gcc -o UVagrep UVagrep.c -Wall -Werror

debug: UVagrep.c
	gcc -o UVagrep_debug UVagrep.c -Wall -Werror -g
	gdb UVagrep_debug

clear: UVagrep | UVagrep_debug
	rm UVagrep UVagrep_debug
