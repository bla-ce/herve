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
	@echo ""
	@echo ""
	@echo "Usage: make <target>"
	@echo "Example: make echo"

.PHONY: help echo static-content models proxy groups hello-world

echo:
	$(MAKE) -C echo
	$(MAKE) -C echo run

static-content:
	$(MAKE) -C static-content
	$(MAKE) -C static-content run

models:
	$(MAKE) -C models
	$(MAKE) -C models run

groups:
	$(MAKE) -C groups
	$(MAKE) -C groups run

hello-world:
	$(MAKE) -C hello-world
	$(MAKE) -C hello-world run

proxy:
	$(MAKE) -C proxy
	$(MAKE) -C proxy run-server1&
	$(MAKE) -C proxy run-server2&
	$(MAKE) -C proxy run-server3&
	$(MAKE) -C proxy run-proxy
