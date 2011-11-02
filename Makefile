FILES = run_server.c modules/*.c include/*.h
INPUT = run_server.c modules/*.c
FLAGS = -Wall -g -pthread
OUTPUT = run_server

run_server: $(FILES)
	gcc -o $(OUTPUT) $(FLAGS) $(INPUT)

clean: 
	rm -f run_server
