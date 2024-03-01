FROM ubuntu:22.04 AS build

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Moscow

COPY . /service
WORKDIR /service/build

SHELL ["/bin/bash", "-c"]

RUN pwd
RUN apt-get update
RUN apt-get install -y python3-pip
RUN apt-get install -y ffmpeg
RUN pip3 install conan --upgrade
RUN conan profile detect
RUN conan install .. --output-folder=. --build=missing -c tools.system.package_manager:mode=install
RUN source conanbuild.sh
RUN cmake -DCMAKE_BUILD_TYPE=Release ..
RUN cmake --build .
RUN source deactivate_conanbuild.sh

FROM ubuntu:22.04

RUN groupadd dev && useradd -g dev dev
USER dev
COPY --chown=dev:dev --from=build /service/build/audio2waveform /service/

CMD ["/service/audio2waveform", "0.0.0.0", "8080"]
EXPOSE 8080