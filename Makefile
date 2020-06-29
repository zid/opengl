SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)

CFLAGS = -O3 -Iinclude/
LDFLAGS = -lopengl32 -lpng -lgdi32

gl: $(OBJ)
	gcc $(OBJ) -o gl $(LDFLAGS)


%.o : %.c
	$(CC) $^ -c -o $@ $(CFLAGS)