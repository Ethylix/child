---
title: Install Child
description: How to get Child up and running on your network
---
{{< hint warning >}}
**Current limitations**
- Child doesn't support linking over TLS/SSL to an ircd. The current recommendation is to link to a local ircd on the local interface (127.0.0.1).
- Child only supports MySQL 5.7
{{< /hint >}}

{{< toc >}}

## Dependencies
Install dependencies, example debian-based OSes:
```
apt install git libmariadb-dev-compat libmariadbclient-dev libc-dev libssl-dev cmake check
```

## Compile child
```
git clone https://github.com/Ethylix/child.git
cd child
mkdir build
cd build
cmake ..
make
```

Child will now live in `./build/`.

## Prepare MySQL
First, install MySQL 5.7 on your OS.

Then create a database and a user for Child.
```
mysql> CREATE DATABASE child;
mysql> CREATE USER 'child'@'localhost' IDENTIFIED BY 'longpasswordformysql';
mysql> GRANT ALL PRIVILEGES ON child.* TO 'child'@'localhost';
mysql> FLUSH PRIVILEGES;
```

Once done, you need to import our schema in the database.
{{< expand >}}
{{< include file="static/schema.sql" language="sql" markdown=false >}}
{{< /expand >}}

```
mysql -u child -p < schema.sql
```


## Configure unrealircd
On all the IRCD in the network, you need to configure a few things. For example, if your service is called `services.geeknode.org`:
{{< expand >}}
```
// U:Line, needed for Child to act as a service 
ulines {
    "services.geeknode.org";
};

// aliases to help users use C
// doing /ns something will be translated to /msg C nick something
alias cs {
    format "" {
        target "C";
        type services;
        parameters "CHAN %1-";
    };
    type command;
};
alias ns {
    format "" {
        target "C";
        type services;
        parameters "NICK %1-";
    };
    type command;
};
alias os {
    format "" {
        target "C";
        type services;
        parameters "OPER %1-";
    };
    type command;
};
alias hs {
    format "" {
        target "C";
        type services;
        parameters "HOST %1-";
    };
    type command;
};
alias "identify" {
    format "^[^#]" {
        nick "C";
        type services;
        parameters "NICK IDENTIFY %1-";
    };
    type command;
};

// Appoint Child as an sasl server
set {
    sasl-server "services.geeknode.org";
};
```
{{< /expand >}}

Last, on the ircd Child will connect to, you need to create the corresponding link configuration:
{{< expand >}}
```
// listen on a local unencrypted port for Child to connect
listen
{
	ip 127.0.0.1;
	port 7001;
	options
	{
		serversonly;
	};
};

link services.geeknode.org
{
        incoming {
                mask *;
        };

        outgoing {
                bind-ip ::ffff:127.0.0.1;
                hostname ::ffff:127.0.0.1;
                port 7001;
                options { };
        };
        password "LONGPASSWORD";
        hub *;
        class servers;
};
```
{{< /expand >}}

Rehash the configuration on all your ircds.

## Configure Child
Create a file in `build/` called `child.conf` and put the following inside:
{{< expand >}}
{{< include file="static/child.conf" language="ini" markdown=false >}}
{{< /expand >}}
Update the values as needed.

## Start Child
```
cd build/
./child
```

## Register your first user
The first user registered becomes the service owner with the highest level. To register your account:
```
/msg C nick register
```

C should reply:
```
C (cserve@geeknode.org): You have now the level 1000
C (cserve@geeknode.org): You are now registered.
```

Child is now installed!