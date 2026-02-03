help:
	@echo "HERVE - Available make targets:"
	@echo ""
	@echo "Tests:"
	@echo "	test-unit: Run all unit tests"
	@echo ""
	@echo "Usage: make <target>"
	@echo "Example: make test-unit"

.PHONY: help test-unit

test-unit:
	$(MAKE) -C lib/boeuf
	$(MAKE) -C lib/encoding/base64
	$(MAKE) -C lib/encoding/html
	$(MAKE) -C lib/encoding/json
	$(MAKE) -C lib/hash_table
	$(MAKE) -C lib/malloc
	$(MAKE) -C lib/models
	$(MAKE) -C lib/utils program=array_test
	$(MAKE) -C lib/utils program=string_test
