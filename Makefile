main: main.c
	cc main.c\
		-o main\
		-Wall\
		-ggdb\
		-lm\
		-lX11 -lXext
