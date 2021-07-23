#!/bin/bash
openssl req -x509 -newkey rsa:4096 -keyout /data/unrealircd/conf/tls/server.key.pem -out /data/unrealircd/conf/tls/server.cert.pem -subj '/CN=localhost' -days 365 -nodes

/data/unrealircd/bin/unrealircd -F
