# 0-1 背包问题 C 语言实验 Makefile
# Windows: mingw32-make 或 make
# Linux/macOS: make

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99
LDFLAGS = -lm

SRCS = common.c data_generator.c brute_force.c dynamic_programming.c \
       greedy.c backtracking.c export_data.c run_experiment.c
OBJS = $(SRCS:.c=.o)
TARGET = run_experiment

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET).exe $(TARGET)
