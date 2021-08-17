---
title: Nick Commands
description: This page describes the nick commands
---

{{< hint info >}}
You can always check each command help using `/msg C nick help COMMAND`
{{< /hint >}}

{{< toc >}}

## drop
```
/msg C nick drop [ACCOUNTNAME]
```

Drop your account.

## ghost
```
/msg C nick ghost ACCOUNTNAME PASSWORD
```

Kill a ghosted nick. This is handy when one loses connectivity and their previous session is still holding a nick. This command will kill the previous session for you to be able to use that nick.

## identify
```
/msg C nick identify PASSWORD
```

Identifies as the current nick.

## info
```
/msg C nick info NICK
```

Provides information about an account.

## link
{{< hint danger >}}
**Deprecated**
Starting with Child v2.0, you can be logged in and still change your nick. This was not the case in the past.
{{< /hint >}}
```
/msg C nick link ACCOUNTNAME PASSWORD
```

Link your account to another account. By doing so, the current account will be able to use the remote account access lists. For instance, if the remote account has an auto op on a channel, the linked account will as well. You can change the account the accesses are shared from by setting the master one:
```
/msg C nick set master ACCOUNTNAME
```

## register
```
/msg C nick register PASSWORD EMAIL
```

Register your account. 

## requestpassword
```
/msg C nick requestpassword ACCOUNTNAME
```

Reset your password, the new one will be sent over to your email address.

## set
```
/msg C nick set OPTION VALUE
```

Set an option on your account.

### cloak
```
/msg C nick set cloak [on|off]
```
Default: off

Enable or disable host cloaking. Your cloak will be xiu.users.geeknode.org. Note that the cloak never overrides your vhost.

### email
```
/msg C nick set email EMAIL
```
Set your email address. This is required for resetting your password via `requestpassword`.

### hideemail
```
/msg C nick set hideemail [on|off]
```
Default: on

Set your email address. This is required for resetting your password via `requestpassword`.

### master
{{< hint danger >}}
**Deprecated**
Starting with Child v2.0, you can be logged in and still change your nick. This was not the case in the past.
{{< /hint >}}
```
/msg C nick set master ACCOUNTNAME
```

Set the nick whose access list is shared when several accounts are linked.

### noauto
```
/msg C nick set noauto [on|off]
```
Default: off

Prevent setting modes on ident.

### password
```
/msg C nick set password [ACCOUNTNAME] PASSWORD
```

Change your password, or the password of the specified account.

### private
```
/msg C nick set private [on|off]
```
Default: off

Hide your account information (level, status, lastseen etc).

### protect
```
/msg C nick set hideemail [on|off]
```
Default: off

Enable or disable the nick protection. When the protection is enabled, if someone take your nick and does not identify, their nick will be changed after a specified time. See `help set timeout` for the time limit.

### timeout
```
/msg C nick set timeout SECONDS
```
Default: 60

Set the time limit in seconds for the nick protection. Setting the time to 0 will reset it to its default value.

## unlink
{{< hint danger >}}
**Deprecated**
Starting with Child v2.0, you can be logged in and still change your nick. This was not the case in the past.
{{< /hint >}}
```
/msg C nick set unlink
```

Unlink your account from another account.