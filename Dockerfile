FROM ubuntu:22.04 AS build

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Moscow

COPY . /service
WORKDIR /service/build

SHELL ["/bin/bash", "-c"]

RUN apt-get update && \
    apt-get install -y \
        pkg-config \
        python3-pip && \
    pip3 install --upgrade \ 
        cmake \
        conan  && \
    conan profile detect && \
    conan install .. --output-folder=. --build=missing -c tools.system.package_manager:mode=install && \
    source conanbuild.sh && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build . && \
    source deactivate_conanbuild.sh

FROM ubuntu:22.04

RUN groupadd dev && useradd -g dev dev
USER dev
COPY --chown=dev:dev --from=build /service/build/audio2waveform /service/

CMD ["/service/audio2waveform"]
EXPOSE 8080