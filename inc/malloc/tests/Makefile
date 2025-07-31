INC_DIR = ..
INCLUDES = $(wildcard $(INC_DIR)/*.inc)
TESTS = $(wildcard *.s)

test: $(TESTS)
	@pass=0; fail=0; \
	for test in $(TESTS); do \
		echo "${YELLOW}Running $$test...${NC}"; \
		nasm -f elf64 -o "$${test%.s}.o" "$$test" -I$(INC_DIR)/; \
		ld -o "$${test%.s}" "$${test%.s}.o"; \
		"./$${test%.s}"; \
		ret=$$?; \
		if [ $$ret -eq 0 ]; then \
			echo "${GREEN}$$test passed.${NC}"; \
			pass=$$((pass + 1)); \
		else \
			echo "${RED}$$test failed with return code $$ret.${NC}"; \
			fail=$$((fail + 1)); \
		fi; \
		rm -f "$${test%.s}.o" "$${test%.s}"; \
	done; \
	echo "\n${YELLOW}Test Summary:${NC}"; \
	echo "${GREEN}Passed: $$pass${NC}"; \
	echo "${RED}Failed: $$fail${NC}"; \
	if [ $$fail -gt 0 ]; then exit 1; fi

