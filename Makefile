CC = cc
CFLAGS = -std=c89 -Wall -Wextra -pedantic
PTHREAD = -pthread

all: sequencial paralelo

sequencial:
	$(CC) $(CFLAGS) src/contaObjetosSequencial.c -o sequencial

paralelo:
	$(CC) $(CFLAGS) $(PTHREAD) src/contaObjetosParalelo.c -o paralelo

clean:
	rm -f sequencial paralelo