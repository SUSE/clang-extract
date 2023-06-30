TESTS=$(wildcard *.c)
TESTS_OBJ=$(patsubst %, %.out, $(TESTS))

check: $(TESTS_OBJ)
	echo $(TESTS_OBJ)

%.c.out: %.c
	python3 runtest.py $^

