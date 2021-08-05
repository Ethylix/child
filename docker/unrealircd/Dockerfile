FROM debian:stable-slim
ARG VERSION=5.2.1.1
ARG LIBPROM_VERSION=0.1.3

# Services
EXPOSE 6666
# Plaintext
EXPOSE 6667
# SSL/TLS
EXPOSE 6697

RUN apt-get update && \
	apt-get install -y gdb build-essential curl libssl-dev ca-certificates libcurl4-openssl-dev zlib1g sudo python3 && \
  apt-get clean

RUN mkdir /data && useradd -r -d /data unrealircd && chown unrealircd:unrealircd /data

COPY entrypoint.sh /

USER unrealircd

RUN cd /data && curl -s --location https://www.unrealircd.org/unrealircd4/unrealircd-${VERSION}.tar.gz | tar xz && \
	cd unrealircd-${VERSION} 
  
COPY config.settings /data/unrealircd-${VERSION}/

RUN cd /data/unrealircd-${VERSION} && ./Config -quick

RUN cd /data/unrealircd-${VERSION} && make && \
    make install && \
	  cd /data && \
  #	rm -rf unrealircd-${VERSION} && \
	  chmod +x /data/unrealircd/unrealircd

WORKDIR /data/unrealircd

ENTRYPOINT /entrypoint.sh
