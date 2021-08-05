# Docker
Our docker-compose file will start:
- mysql
- unrealircd
- child

It provisions mysql with the schema in `schema.sql` and links Child as `services.geeknode.org` to unrealircd (`test.geeknode.org`). You can connect to unrealircd on port `6697` (tls self-signed) and `/oper test test`.

# Start the stack
```
docker-compose up
```

If you want to rebuild the container and recreate mysql/unrealircd, use:
```
docker-compose up --build --force-recreate
```

## Stop the stack
```
docker-compose down
```
