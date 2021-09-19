FROM rikorose/gcc-cmake:gcc-9

ENV PROM_VERSION=0.1.3

RUN apt-get update \
    && apt-get -y install check libssl1.1 libmariadb3 libc6 libsodium-dev libmicrohttpd-dev netcat gdb gdbserver \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# libprom/libpromhttp
RUN curl -L https://github.com/digitalocean/prometheus-client-c/releases/download/v${PROM_VERSION}/libprom-dev-${PROM_VERSION}-Linux.deb -o /tmp/libprom-dev.deb && \
    curl -L https://github.com/digitalocean/prometheus-client-c/releases/download/v${PROM_VERSION}/libpromhttp-dev-${PROM_VERSION}-Linux.deb -o /tmp/libpromhttp-dev.deb && \
    dpkg -i /tmp/libprom-dev.deb /tmp/libpromhttp-dev.deb
    
ADD . /opt/
RUN useradd -d /opt/child -m child
WORKDIR /opt/child

COPY docker/entrypoint.sh /
RUN chmod +x /entrypoint.sh

USER child
VOLUME /opt/child

ENTRYPOINT ["/entrypoint.sh"]
