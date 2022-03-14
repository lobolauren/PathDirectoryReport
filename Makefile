CFLAGS = -Wpedantic -std=gnu99
CC = gcc

all:  FSreport 

FSreport: FSreport.o 
	$(CC) $(CFLAGS) FSreport.c -o FSreport

clean:
		rm *.o
