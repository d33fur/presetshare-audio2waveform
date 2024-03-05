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
    pip3 install conan --upgrade && \
    conan profile detect && \
    conan install .. --output-folder=. --build=missing && \
    source conanbuild.sh && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build . && \
    source deactivate_conanbuild.sh


# RUN apt-get update
# RUN apt-get install -y python3-pip
# RUN apt-get install -y pkg-config
# RUN pip3 install conan --upgrade
# RUN conan profile detect
# RUN conan install .. --output-folder=. --build=missing -c tools.system.package_manager:mode=install
# RUN source conanbuild.sh
# RUN cmake -DCMAKE_BUILD_TYPE=Release ..
# RUN cmake --build .
# RUN source deactivate_conanbuild.sh

FROM ubuntu:22.04

RUN groupadd dev && useradd -g dev dev
USER dev
COPY --chown=dev:dev --from=build /service/build/audio2waveform /service/

CMD ["/service/audio2waveform"]
EXPOSE 8080