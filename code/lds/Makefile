.PHONY : all clean cleanobj

C =gcc -c $< -o $@
OBJ = main.o node2.o node1.o

all : main

main : main.o node2.o node1.o main.lds
	gcc -lc main.o node2.o node1.o -o $@ -T main.lds

main.o : main.c
	$(C)
node2.o : node2.c
	$(C)
node1.o : node1.c
	$(C)

cleanobj :
	rm -rf $(OBJ)

clean :
	rm -rf $(OBJ) main