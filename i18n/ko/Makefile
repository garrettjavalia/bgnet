BOOK_DIR := $(abspath ../..)
OUTPUT ?= $(CURDIR)/dist
override OUTPUT := $(abspath $(OUTPUT))
IMAGE ?= bgbspd-ko-builder
DOCKER ?= $(shell command -v docker 2>/dev/null || if test -x /Applications/Docker.app/Contents/Resources/bin/docker; then printf '%s' /Applications/Docker.app/Contents/Resources/bin/docker; else printf '%s' docker; fi)
FORMAT ?= all
PDF_TARGETS ?= all
JOBS ?= 2

.PHONY: all build stage docker docker-build html pdf status clean pristine test
all build stage: docker

docker-build:
	$(DOCKER) build -f "$(CURDIR)/Dockerfile" -t "$(IMAGE)" "$(BOOK_DIR)"

docker: docker-build
	mkdir -p "$(OUTPUT)"
	$(DOCKER) run --rm --network none \
		-v "$(BOOK_DIR):/guide:ro" -v "$(OUTPUT):/output" \
		"$(IMAGE)" --format "$(FORMAT)" --pdf-targets "$(PDF_TARGETS)" --jobs "$(JOBS)"

html:
	$(MAKE) docker FORMAT=html

pdf:
	$(MAKE) docker FORMAT=pdf

status: docker-build
	$(DOCKER) run --rm --network none -v "$(BOOK_DIR):/guide:ro" "$(IMAGE)" --status

clean pristine: docker-build
	mkdir -p "$(OUTPUT)"
	$(DOCKER) run --rm --network none \
		-v "$(BOOK_DIR):/guide:ro" -v "$(OUTPUT):/output" "$(IMAGE)" --clean

test:
	PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s . -p 'test_docker_entrypoint.py' -v
