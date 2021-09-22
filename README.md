# Child
[![Child](https://circleci.com/gh/Ethylix/child.svg?style=svg)](https://circleci.com/gh/Ethylix/child) [![codecov](https://codecov.io/gh/ethylix/child/branch/master/graph/badge.svg)](https://codecov.io/gh/ethylix/child) [![IRC #cserv on geeknode](https://img.shields.io/badge/geeknode-%23cserv-brightgreen)](irc://irc.geeknode.org:6697/cserv)

Child is an IRC Service for [UnrealIRCD](https://www.unrealircd.com).

# Dependencies
- `libmysqlclient` (or `libmariadbclient-dev-compat`)
- `libssl`
- `libc`
- `cmake`
- `check`
- `libsodium`
- mysql 5.7
- unrealircd 4.2

# Build
```
apt install libmariadb-dev-compat libsodium-dev libmariadbclient-dev libc-dev libssl-dev cmake check
mkdir build
cd build
cmake ..
make
```

For development, you can use docker-compose to set an environment up: [doc/docker.md](doc/docker.md).

# Usage
```
./child
```

# Configuration
Child expects a configuration file in the current working directory: `child.conf`. You can find an example in [doc/example.conf](doc/example.conf).