FROM ubuntu:18.04 as build

RUN apt-get update && \
    apt-get -y install --no-install-recommends gcc make cmake libc-dev libssl-dev pkg-config check libmysqlclient-dev

ADD --chown=1000:1000 . /opt/
WORKDIR /opt/build

USER 1000

RUN cmake /opt 
RUN make

FROM ubuntu:18.04

RUN apt-get update && \
    apt-get -y install libssl-dev libmysqlclient-dev libc-dev

COPY --from=build /opt/build/src/child /

USER 1000

CMD ["./child"]