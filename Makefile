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
local: conan-rebuild local-rebuild

.PHONY: local-rebuild
local-rebuild:
	@(cd build && \
	source conanbuild.sh && \
	cmake -DCMAKE_BUILD_TYPE=Release .. && \
	cmake --build . && \
	source deactivate_conanbuild.sh && \
	./audio2waveform ./../DSG_BS_174_F_Bass_08.wav)

.PHONY: conan-rebuild
conan-rebuild:
	@(rm -rf build/ && \
	mkdir build && \
	cd build && \
	conan install .. --output-folder=. --build=missing)


# docker run -it audio2waveform bash