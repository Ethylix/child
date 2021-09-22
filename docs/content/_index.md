---
title: Welcome to Child documentation
description: Child is a simple modular service for your [UnrealIRCD](https://unrealircd.org) IRC network. It was built for [GeekNode](https://geeknode.org) and has been serving it ever since!
---

<!-- markdownlint-capture -->
<!-- markdownlint-disable MD033 -->

<span class="badge-placeholder">[![Child](https://circleci.com/gh/Ethylix/child.svg?style=svg)](https://circleci.com/gh/Ethylix/child)</span>
<span class="badge-placeholder">[![codecov](https://codecov.io/gh/ethylix/child/branch/master/graph/badge.svg)](https://codecov.io/gh/ethylix/child)</span>
<span class="badge-placeholder">[![GitHub release](https://img.shields.io/github/v/release/ethylix/child)](https://github.com/ethylix/child/releases/latest)</span>
<span class="badge-placeholder">[![GitHub contributors](https://img.shields.io/github/contributors/ethylix/child)](https://github.com/ethylix/child/graphs/contributors)</span>
<span class="badge-placeholder">[![License: GPL-2.0](https://img.shields.io/github/license/ethylix/child)](https://github.com/ethylix/child/blob/main/LICENSE)</span>
<span class="badge-placeholder">[![IRC #cserv on geeknode](https://img.shields.io/badge/geeknode-%23cserv-brightgreen)](irc://irc.geeknode.org:6697/cserv)</span>

<!-- markdownlint-restore -->



{{< toc >}}

## What is Child?

Child is a simple modular service for your [UnrealIRCD](https://unrealircd.org) IRC network. It was built for [GeekNode](https://geeknode.org) and has been serving it ever since!

Child behaves differently from other services like Anope or  Atheme: instead of having multiple service accounts (NickServ, ChanServ, OperServ...), Child handles everything under its nickname **C**.


## Register your user
Make sure you are using the nick you want to register, if not: `/nick NICK` and then register it:
```
/msg C nick register PASSWORD EMAIL
```

C will send you a message confirming that you are registered:
```
2021-07-20 22:18:01     --      C: You are now registered.
```

There are a few options that need you opting in:
- `protect`: if someones uses your nick, they will be renamed to a temporary nick unless they authenticate under 60 seconds.
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
