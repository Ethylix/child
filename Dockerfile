FROM gcc:7.5 as build

ENV CMAKE_VERSION 3.16.8

RUN apt-get update \
    && apt-get -y install check libmicrohttpd-dev \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

ADD https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-Linux-x86_64.tar.gz /
RUN tar xf /cmake-${CMAKE_VERSION}-Linux-x86_64.tar.gz -C /opt/
RUN /opt/cmake-${CMAKE_VERSION}-Linux-x86_64/bin/cmake --version
RUN rm -rf /cmake-${CMAKE_VERSION}-Linux-x86_64.tar.gz

RUN mkdir -p /opt/child/build
ADD . /opt/child
WORKDIR /opt/child/build

RUN /opt/cmake-${CMAKE_VERSION}-Linux-x86_64/bin/cmake /opt/child
RUN make

FROM ubuntu:18.04

RUN apt-get update \
    && apt-get -y install libssl1.1 libmariadb3 libc6 netcat libmicrohttpd12 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

COPY --from=build --chown=1000:1000 /opt/child/build/src/child /opt/child/
COPY --from=build --chown=1000:1000 /opt/child/build/src/modules /opt/child/src/modules
COPY --chown=1000:1000 docker/entrypoint.sh /
RUN chmod +x /entrypoint.sh

USER 1000

WORKDIR /opt/child

CMD ["/entrypoint.sh"]
