help:
	@echo "HERVE - Available make targets:"
	@echo ""
	@echo "Examples:"
	@echo "	hello-world: Run hello world server"
	@echo "	echo: Run echo server"
	@echo "	static-content: Run example serving static content"
	@echo "	models: Run example with models"
	@echo "	proxy: Run example with a proxy and 3 upstream servers"
	@echo "	groups: Run example with groups"
	@echo ""
	@echo "Tests:"
	@echo "	test-unit: Run all unit tests"
	@echo ""
	@echo "Benchmark:"
	@echo "	wrk: Run \`wrk\` benchmark"
	@echo ""
	@echo "Usage: make <target>"
	@echo "Example: make echo"

.PHONY: help test-models test-echo test-static-content test-utils test-all wrk echo static-content models proxy groups hello-world

echo:
	$(MAKE) -C examples/echo
	$(MAKE) -C examples/echo run

static-content:
	$(MAKE) -C examples/static-content
	$(MAKE) -C examples/static-content run

models:
	$(MAKE) -C examples/models
	$(MAKE) -C examples/models run

groups:
	$(MAKE) -C examples/groups
	$(MAKE) -C examples/groups run

hello-world:
	$(MAKE) -C examples/hello-world
	$(MAKE) -C examples/hello-world run

proxy:
	$(MAKE) -C examples/proxy
	$(MAKE) -C examples/proxy run-server1&
	$(MAKE) -C examples/proxy run-server2&
	$(MAKE) -C examples/proxy run-server3&
	$(MAKE) -C examples/proxy run-proxy

test-unit:
	$(MAKE) -C tests/unit/boeuf
	$(MAKE) -C tests/unit/encoding/base64
	$(MAKE) -C tests/unit/encoding/html
	$(MAKE) -C tests/unit/encoding/json
	$(MAKE) -C tests/unit/hash_table
	$(MAKE) -C tests/unit/malloc
	$(MAKE) -C tests/unit/models
	$(MAKE) -C tests/unit/utils program=array_test
	$(MAKE) -C tests/unit/utils program=string_test

# TODO: define the other targets
