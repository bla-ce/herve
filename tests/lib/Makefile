.PHONY: test-lib

test-lib:
	$(MAKE) -C boeuf
	$(MAKE) -C crypto/bcrypt
	$(MAKE) -C data_structures program=array_test
	$(MAKE) -C data_structures program=linked_list_test
	$(MAKE) -C encoding/base64
	$(MAKE) -C encoding/html
	$(MAKE) -C encoding/json
	$(MAKE) -C hash_table
	$(MAKE) -C malloc
	$(MAKE) -C models
	$(MAKE) -C uuid
	$(MAKE) -C utils program=string_test
