OBJ += server.o
OBJ += sqlite3.o



server: $(OBJ)
	gcc $(OBJ) -o $@  -lpthread -ldl

%.o:%.c
	gcc -c $^ -o $@ $(FLAGS)

.PHONY:clean
clean:
	rm server *.o -rfv