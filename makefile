CC = g++
CFLAG = -g -Wall

all: coordinator palin

%.o: %.cpp
	$(CC) $(CFLAG) -c $< -o $@

coordinator: coordinator.o
	$(CC) $(CFLAG) $< -o $@

palin: palin.o
	$(CC) $(CFLAG) $< -o $@

clean:
	rm -f *.o coordinator palin
