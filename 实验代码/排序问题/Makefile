CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99
SRCS = common.c data_generator.c bubble_sort.c merge_sort.c quick_sort.c run_experiment.c
TARGET = run_experiment

.PHONY: all run plot clean

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

run: $(TARGET)
	./$(TARGET)
	python plot_results.py

plot:
	python plot_results.py

clean:
	rm -f $(TARGET) $(TARGET).exe
