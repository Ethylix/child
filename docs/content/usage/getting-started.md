---
title: Getting Started
description: Quick start at registering an account and a channel
---

{{< hint info >}}
This page is a quickstart and doesn't describe all of Child capabilities.
{{< /hint >}}

{{< toc >}}

Child behaves differently from other services like Anope or  Atheme: instead of having multiple service accounts (NickServ, ChanServ, OperServ...), C handles everything under its name.

This page describes how to register an account, identify yourself, register and manage simple access control on it.

## Register your user
Make sure you are using the nick you want to register, if not: `/nick NICK` and then register it:
```
/msg C nick register PASSWORD EMAIL
```

C will send you a message confirming that you are registered:
```
2021-07-20 22:18:01     --      C: You are now registered.
```

There is a few options that need you opting in:
- `protect`: if someones uses your nick, they will be renamed to a temporary nick unless they authenticate under 30 seconds.
- `private`: hide your account information

To activate any of these options, use:
```
/msg C nick set OPTION on
```

There is a lot more options, you can check them out [here]({{< ref "/commands/nick#set" >}}).

## Identify yourself
Make sure you're using your account name as your nick. If not: `/nick ACCOUNTNAME`.
```
/msg C nick identify PASSWORD
```

## Register a channel
```
/msg C chan register #channel
```

C will then join your channel.

That's it, you are now the owner of your account and channel. Want to learn more? Check out these pages:
- [Manage your account]({{< ref "usage/manage-account" >}})
- [Manage a channel]({{< ref "usage/manage-channel" >}})


