CC = gcc
CFLAGS = -g -O0 -std=c99 -Wall -pedantic -lpthread 
SOURCE = hw13.c
OBJ = $(patsubst %.c, %.o, $(SOURCE))
EXE = hw13
VALGRIND = valgrind --tool=memcheck --leak-check=yes --track-origins=yes 
RESULTS = out.txt
MEMTXT = mem.txt
VERB = -v

.SILENT:
all: $(EXE)

$(EXE): $(SOURCE)
	@echo "Compiling hw12.c"
	$(CC) $(CFLAGS) $(SOURCE) -o $(EXE)

test: $(EXE) 
	@echo "Running tests"
	@echo "Will take about 8-10 minutes"
	@echo "Running ./ hw13 -t 1 -s -v"
	@echo "./hw13 -t 1 -s -v" > $(RESULTS)
	-./$(EXE) -t 1 -s -v >> $(RESULTS) 2>&1
	@echo " " >> $(RESULTS)
	@echo "Running ./13 -t 5 s"
	@echo "./hw13 -t 5 -s" >> $(RESULTS)
	-./$(EXE) -t 5 -s >> $(RESULTS) 2>&1
	@echo " " >> $(RESULTS)
	@echo "Running ./hw13 -t 8 -v"
	@echo "./hw13 -t 8 -v" >> $(RESULTS)
	-./$(EXE) -t 8 -v >> $(RESULTS) 2>&1
	@echo " " >> $(RESULTS)
	@echo "Running ./hw13 -t 0"
	@echo "./hw13 -t 0" >> $(RESULTS)
	-./$(EXE) -t 0 >> $(RESULTS) 2>&1
	@echo " " >> $(RESULTS)
	@echo "Running ./hw13 -t 9"
	@echo "./hw13 -t 9" >> $(RESULTS)
	-./$(EXE) -t 9 >> $(RESULTS) 2>&1
	@echo "check out.txt for results"

.PHONY: mem clean test all help
mem: $(EXE)
	@echo "running valgrind, will take about 1 minute"
	-$(VALGRIND) ./$(EXE) -t 8 -f -s > $(MEMTXT) 2>&1
	@echo "valgrind output in mem.txt"

clean: 
	-rm -f $(EXE) $(RESULTS) $(MEMTXT)

help:
	@echo "make options are: all, clean, mem, test"

