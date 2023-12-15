.PHONY: check clean

check:
	$(MAKE) -C testsuite

clean:
	$(MAKE) -C testsuite clean
