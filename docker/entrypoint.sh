#!/bin/sh
# entrypoint.sh

set -e
  
until nc -z unrealircd 6999; do
  >&2 echo "unrealircd is unavailable - sleeping"
  sleep 1
done

until nc -z mysql 3306; do
  >&2 echo "mysql is unavailable - sleeping"
  sleep 1
done
  
>&2 echo "Dependencies are up - starting child"
exec /opt/child/child -d -vv

