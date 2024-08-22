SHELL := /bin/bash

.PHONY: all
all: rebuild start

.PHONY: build
build:
	docker build -t audio2waveform .

.PHONY: rebuild
rebuild:
	docker-compose build --force-rm

.PHONY: start
start:
	docker-compose up -d

.PHONY: stop
stop:
	docker-compose stop

.PHONY: restart
restart: stop start

.PHONY: logs
logs:
	docker-compose logs -f --tail 100

.PHONY: clear
clear:
	@(docker-compose kill  && \
	docker-compose rm -f) 

.PHONY: local
local: conan-rebuild local-rebuild local-run

.PHONY: local-rebuild
local-rebuild:
	@(cd build && \
	source conanbuild.sh && \
	cmake -DCMAKE_BUILD_TYPE=Release .. && \
	cmake --build . && \
	source deactivate_conanbuild.sh)

.PHONY: conan-rebuild
conan-rebuild:
	@(rm -rf build/ && \
	mkdir build && \
	cd build && \
	conan install .. --output-folder=. --build=missing)

.PHONY: local-run
local-run:
	@(cd build && \
	./audio2waveform 0.0.0.0 8080)

#  тут тесты не будут работать, потому что api другая

.PHONY: tests
tests:
	@(pip install requests >/dev/null && \
	python3 tests/api.py)