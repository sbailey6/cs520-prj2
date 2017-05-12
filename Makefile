SRC_FILES = MOESI.c makefile

all:
	gcc -g MOESI.c -o main

clean:
	rm -rf main *~
	
valgrind:
	make; valgrind --leak-check=full --show-leak-kinds=all ./main target;
	
run:
	gcc MOESI.c -o main; ./main;

submit : $(SRC_FILES)
	mkdir rrolsto1-sbailey6-pr2
	cp $(SRC_FILES) rrolsto1-sbailey6-pr2
	tar -zcvf rrolsto1-sbailey6-pr2.tar.gz rrolsto1-sbailey6-pr2
	rm -r rrolsto1-sbailey6-pr2
