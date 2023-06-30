TESTS=$(wildcard *.c)
TESTS_OBJ=$(patsubst %.c, %.log, $(TESTS))

.PHONY: check
check: $(TESTS_OBJ)

.PHONY: $(TESTS)
%.log: %.c
	@python3 runtest.py $^ -o $@

clean:
	rm *.log
