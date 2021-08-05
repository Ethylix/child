#!/bin/sh
# entrypoint.sh

set -e

cmake -DCMAKE_BUILD_TYPE=Debug /opt/child
make

until nc -z unrealircd 6999; do
  >&2 echo "unrealircd is unavailable - sleeping"
  sleep 1
done

until nc -z mysql 3306; do
  >&2 echo "mysql is unavailable - sleeping"
  sleep 1
done
  
>&2 echo "Dependencies are up - starting child"
#exec gdbserver localhost:9999 /opt/child/src/child -d -vv
exec /opt/child/src/child -d -vv
