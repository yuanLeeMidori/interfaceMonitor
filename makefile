#    UNX511 - Assignment 1 - Nov.2020
#    
#    makefile
# 
#    Student name  : Yuan-Hsi Lee
#    Student number: 106936180
#    Student email : ylee174@myseneca.ca

CC=g++
CFLAGS=-I
CFLAGS+=-Wall
FILES1=interfaceMonitor.cpp
FILES2=networkMonitor.cpp

interfaceMonitor1: $(FILES1)
	$(CC) $(CFLAGS) $^ -o $@

interfaceMonitor2: $(FILES1)
	$(CC) $(CFLAGS) $^ -o $@

networkMonitor: $(FILES2)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f *.o interfaceMonitor1 interfaceMonitor2 networkMonitor

all: interfaceMonitor1 interfaceMonitor2 networkMonitor
