---
title: Managing your account
description: Walkthrough of the nick commands
---

Child's NickServ is accessible via `/msg C nick`.

The first thing to do if you didn't already is to register your account:
```
/msg C nick register PASSWORD EMAIL
```

You will then receive a notice from C:
```
C (cserve@geeknode.org): You are now registered.
User mode [+r] by services.geeknode.org
You are now logged in as test. (nick!ident@host)
```

These messages might appear in a status window or tab in your client. For instance, in weechat, it'll be in the first buffer. You can access it using `/buffer 1`.

Your account is now yours. There are multiple options for you to tweak your account. For instance, you might want to `protect` your account:
```
/msg C nick set protect on
```

This option will make sure that anyone taking your account name as a nick will be renamed if they don't identified. This prevents people from squatting your account name.

You can find the list of options on its [dedicated page]({{< ref "/commands/nick#set" >}}).

Now that your account is registerd: [let's register a channel to chat with your friends]({{< ref "usage/manage-channel" >}})!