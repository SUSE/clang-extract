DIRS = $(sort $(dir $(wildcard */)))
TEST_FOLDERS=$(filter-out lib/,$(DIRS))

check: $(TEST_FOLDERS)

$(TEST_FOLDERS):
	$(MAKE) -C $@

clean:
	for folder in $(TEST_FOLDERS); do $(MAKE) -C $$folder clean; done;

.PHONY: check clean $(TEST_FOLDERS)
