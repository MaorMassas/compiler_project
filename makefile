assembler: main.o pre_prossecor.o first_pass.o second_pass.o table.o util.o
	gcc -ansi -Wall -pedantic pre_prossecor.o first_pass.o second_pass.o table.o util.o main.o -o assembler -lm

main.o: main.c pre_prossecor.h
	gcc -c -ansi -Wall -pedantic main.c -o main.o

pre_prossecor.o: pre_prossecor.c pre_prossecor.h first_pass.h
	gcc -c -ansi -Wall -pedantic pre_prossecor.c -o pre_prossecor.o

first_pass.o: first_pass.c first_pass.h util.h table.h
	gcc -c -ansi -Wall -pedantic first_pass.c -o first_pass.o

second_pass.o: second_pass.c second_pass.h table.h util.h
	gcc -c -ansi -Wall -pedantic second_pass.c -o second_pass.o

table.o: table.c table.h util.h
	gcc -c -ansi -Wall -pedantic table.c -o table.o

util.o: util.c util.h
	gcc -c -ansi -Wall -pedantic util.c -o util.o

clean:
	rm -f *.o assembler *.ob *.ent *.ext *.am
