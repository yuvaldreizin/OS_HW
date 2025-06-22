CC = gcc
CFLAGS = -g -Wall -Werror -pedantic-errors -pthread -DNDEBUG
# CFLAGS = -std=c99 -g -Wall -Werror -pedantic-errors -pthread -DNDEBUG
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
TARGET = bank

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(TARGET) $(OBJS)
