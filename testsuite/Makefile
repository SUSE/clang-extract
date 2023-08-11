TEST_FOLDERS=ccp small includes

check: $(TEST_FOLDERS)

$(TEST_FOLDERS):
	$(MAKE) -C $@

clean:
	for folder in $(TEST_FOLDERS); do $(MAKE) -C $$folder clean; done;

.PHONY: check clean $(TEST_FOLDERS)
