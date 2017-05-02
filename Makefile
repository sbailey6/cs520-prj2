all:
	gcc -g MOESI.c -o main

clean:
	rm -rf main *~
	
valgrind:
	make; valgrind --leak-check=full --show-leak-kinds=all ./main target;
	
run:
	gcc MOESI.c -o main; ./main;
