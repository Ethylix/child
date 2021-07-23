FROM gcc:7.5

RUN apt-get update \
    && apt-get -y install cmake check libssl1.1 libmariadb3 libc6 netcat gdb gdbserver \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

ADD . /opt/
RUN useradd -d /opt/child -m child
WORKDIR /opt/child

COPY docker/entrypoint.sh /
RUN chmod +x /entrypoint.sh

USER child
VOLUME /opt/child

ENTRYPOINT ["/entrypoint.sh"]
