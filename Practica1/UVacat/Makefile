build: UVacat.c
	gcc -o UVacat UVacat.c -Wall -Werror

debug: UVacat.c
	gcc -o UVacat_debug UVacat.c -Wall -Werror -g
	gdb ./UVacat_debug

run: ./UVacat
	./UVacat

clear:
	rm ./UVacat ./UVacat_debug
