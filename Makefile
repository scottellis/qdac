# Makefile for mcp4728 Quad DAC test program 

CC = cc
CFLAGS = -Wall

OBJS = qdac.o \
       mcp4728-qdac.o

TARGET = qdac 

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<


.PHONY: clean

clean:
	rm -f $(OBJS) $(TARGET)


