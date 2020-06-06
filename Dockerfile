FROM gcc:7.5 as build

RUN apt-get update \
    && apt-get -y install cmake check \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

ADD . /opt/
RUN mkdir -p /opt/build
WORKDIR /opt/build

RUN cmake /opt 
RUN make

FROM ubuntu:18.04

RUN apt-get update \
    && apt-get -y install libssl1.1 libmariadb3 libc6 netcat \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

COPY --from=build --chown=1000:1000 /opt/build/src/child /opt/child/
COPY --from=build --chown=1000:1000 /opt/build/src/modules /opt/child/src/modules
COPY --chown=1000:1000 docker/entrypoint.sh /
RUN chmod +x /entrypoint.sh

USER 1000

WORKDIR /opt/child

CMD ["/entrypoint.sh"]
