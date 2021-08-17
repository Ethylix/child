---
title: Managing a channel
description: Walkthrough of the chan commands
---

{{< hint info >}}
To execute on this, you need to have [registered your account]({{< ref "usage/manage-account">}}) first.
{{< /hint >}}

{{< toc >}}

## Register a channel
You now have your account and want to register your channel. To do so, you need to be `+o` (op) on a channel that is not registered yet.
```
/join #newchannel
```

By entering the channel, the server will give you `+o`. You can check that you are op on the channel by looking at the nicklist. If you have an `@` in front of your nickname, you are op on the channel.

To register the channel:
```
/msg C chan register #newchannel
```

The channel is now yours, you can check its information by using [the `info` command]({{< ref "commands/chan#info" >}}):
```
/msg C chan info #newchannel
```

Which will reply:
```
C (cserve@geeknode.org): Informations for channel #newchannel:
C (cserve@geeknode.org): Founder: YOU
C (cserve@geeknode.org): Options: Secure Axxflags 
C (cserve@geeknode.org): Registration time: Sun Aug  16 18:51:58 2021
```

## Manage access lists
Your channel is now very popular and you need help in moderating it and would like to have your friend `Lord` be op. There are two ways to manage ACL (access lists) on channels using Child: levels and flags. Using flags is easier though levels are giving the most flexibility as they help in setting up a stronger hierarchy between members. We will use flags for simplicity.

For instance, if we wanted to make sure that our friend `Lord` was auto-oped every time they joined the channel, we'd use:
```
/msg C chan access flags #newchannel Lord +O
```

That's it, flags are as simple as that. You can find the list of flags and their meaning in [this page]({{< ref "commands/chan#flags" >}}).

Child is also able to take orders directly from the channel via a set of commands [described here]({{< ref "commands/bot" >}}). For instance, if I wanted to op `Lord` on my channel, I could do:
```
!op Lord
```

Which would get Child to op Lord: 
```
Mode #cserv [+o Lord] by C 
```

## Moderating
A big part of owning a channel is to make sure that it stays peaceful. It is your space and you dictate the rules. From time to time, you might get users that are not abidding by those rules. As an op on the channel, you might need to kick or ban people. You can do so using Child:
```
!kick BadUser
!ban BadUser
```

There are plenty other commands available for you to use to moderate your channel available on [this page]({{< ref "commands/bot" >}}).

