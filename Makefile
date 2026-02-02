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
	$(MAKE) -C tests/lib/boeuf
	$(MAKE) -C tests/lib/encoding/base64
	$(MAKE) -C tests/lib/encoding/html
	$(MAKE) -C tests/lib/encoding/json
	$(MAKE) -C tests/lib/hash_table
	$(MAKE) -C tests/lib/malloc
	$(MAKE) -C tests/lib/models
	$(MAKE) -C tests/lib/utils program=array_test
	$(MAKE) -C tests/lib/utils program=string_test

# TODO: define the other targets
