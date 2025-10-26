INC_DIR = ..
INCLUDES = $(wildcard $(INC_DIR)/*.inc)
TESTS = $(wildcard *.s)

test: $(TESTS)
	@pass=0; fail=0; \
	for test in $(TESTS); do \
		echo "Running $$test..."; \
		nasm -f elf64 -o "$${test%.s}.o" "$$test" -I$(INC_DIR)/; \
		ld -o "$${test%.s}" "$${test%.s}.o"; \
		"./$${test%.s}"; \
		ret=$$?; \
		if [ $$ret -eq 0 ]; then \
			echo "$$test passed."; \
			pass=$$((pass + 1)); \
		else \
			echo "$$test failed with return code $$ret."; \
			fail=$$((fail + 1)); \
		fi; \
		rm -f "$${test%.s}.o" "$${test%.s}"; \
	done; \
	echo "\nTest Summary:"; \
	echo "Passed: $$pass"; \
	echo "Failed: $$fail"; \
	if [ $$fail -gt 0 ]; then exit 1; fi
