check: ccp small

small:
	$(MAKE) -C small

ccp:
	$(MAKE) -C ccp

clean:
	$(MAKE) -C small clean
	rm -f testrun.*

.PHONY: check small ccp clean
