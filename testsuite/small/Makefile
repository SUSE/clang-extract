TESTS=$(wildcard *.c) $(wildcard *.cpp)
TESTS_OBJ=$(patsubst %.c, %.log, $(TESTS)) \
					$(patsubst %.cpp, %.log, $(TESTS))

.PHONY: check
check: $(TESTS_OBJ)

.PHONY: $(TESTS)
%.log: %.c
	@python3 runtest.py $^ -o $@

%.log: %.cpp
	@python3 runtest.py $^ -o $@

clean:
	rm -f *.log
