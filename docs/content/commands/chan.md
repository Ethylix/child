---
title: Chan Commands
description: This page describes the chan commands
---

{{< hint info >}}
You can always check each command help using `/msg C chan help COMMAND`
{{< /hint >}}

{{< toc >}}

## access
```
Syntax: HELP CHAN ACCESS FLAGS for help about chanflags
Syntax: HELP CHAN ACCESS LEVELS for help about old levels system
The CHAN ACCESS command automatically adapts himself regarding to the AXXFLAGS channel option.
```
### flags
```
Syntax: CHAN ACCESS #channel [username {+|-}flags]

Following channel flags are available :
    o   Can get/give/take op
    O   Get auto-op on join
    v   Can get voice
    V   Get auto-voice on join
    x   Can modify access list
    i   Can get invited
    t   Can change topic
    F   Channel owner, can only be modified with CHAN SET FOUNDER command
    f   Channel co-owner
    h   Can get halfop
    H   Get auto-halfop on join
    s   Can use CHAN SET commands
    p   Can get protect status
    P   Get auto-protect on join (+a mode)
    N   Can't get any status (op/voice/etc)
    k   Auto-kick on join
    b   Auto-kickban on join
    w   Get auto-owner on join (+q mode)

Channel flags are cumulative. Example: +oOV flags will give auto-op and auto-voice on join.
```

### levels
```
Syntax: CHAN ACCESS #channel {add|auto|del|list} [nick [level]]

Following access levels are available :
      9999  Co-Owner. Get +qo modes and inherits channel ownership if the original owner expires.
      20   Full control over the channel without DROP nor SET FOUNDER
      10   can use access add/del command and set some options
       5   auto-op
       4   auto-halfop
       3   auto-voice
       1   can use invite command
      -1   cannot get opped
      -2   auto-kicked
      -3   auto-banned
The maximum level is 9999.

Usage of AUTO param :
CHAN ACCESS #channel AUTO nick {op|voice|default|off}
OP: the user is auto-opped if his/her level >= 5
VOICE: the user is auto-voiced if his/her level >= 3
DEFAULT: the user is set the mode corresponding to his level
OFF: no auto-mode is set
```
## addbot
```
Syntax: CHAN ADDBOT nick ident host
Add a bot
```
## assign
```
Syntax: CHAN ASSIGN #channel botnick
Assign a bot to a channel. This will automatically set NOJOIN option and make C leaving the channel.
```

## banlist
```
Syntax: CHAN BANLIST #channel
List channel bans (added with !tb command)
```

## botlist
```
Syntax: CHAN BOTLIST
List all available bots
```

## clearbans
```
Syntax: CHAN CLEARBANS #channel
Clear internal channel banlist.
```

## clearchan
```
Syntax: CHAN CLEARCHAN #channel
Remove all channel modes, bans, excepts, invex, user status, ...
```

## clearmodes
```
Syntax: CHAN CLEARMODES #channel
Remove all channel modes
```

## dehalfop
```
Syntax: CHAN DEHALFOP #channel [nick]
Take channel half-operator status
```

## delbot
```
Syntax: CHAN DELBOT nick
Delete a bot
```

## deop
```
Syntax: CHAN DEOP #channel [nick]
Take channel operator status
```

## devoice
```
Syntax: CHAN DEVOICE #channel [nick]
Take voice status
```

## drop
```
Syntax: CHAN DROP #channel
Drop specified channel
```

## entrymsg
```
Syntax: CHAN ENTRYMSG #channel [message]
Set a welcome message displayed on join. If no parameter is specified, then entrymsg is removed
```

## halfop
```
Syntax: CHAN HALFOP #channel [nick]
Give channel half-operator status
```

## info
```
Syntax: CHAN INFO #channel
Give some informations for the specified channel
```

## invite
```
Syntax: CHAN INVITE #channel
Invite yourself on a channel
```
## kick
```
Syntax: CHAN KICK #channel nick [reason]
Kick someone or all users
```
## op
```
Syntax: CHAN KICK #channel nick [reason]
Kick someone or all users
```
## register
```
Syntax: CHAN REGISTER #channel
Register specified channel
Chans expire after 5000 days without being used.
```
## resync
```
Syntax: CHAN RESYNC #channel
Resync channel access list with channel members
```
## set
### aop
```
Syntax: CHAN SET #channel aop [on|off]
WHen the option is enabled, users are automatically opped.
A minimum level of 20 is required.
```
### autolimit
```
Syntax: CHAN SET #channel autolimit [limit]
Forcefully set a limit. The limit depends on the numbers of the members present on the channel
E.g.: If there is 5 members and an autolimit set to 3, C will set the limit to 8
```
### avoice
```
Syntax: CHAN SET #channel avoice [on|off]
When the option is enabled, users are automatically voiced.
A minimum level of 10 is required.
```
### axxflags
```
Syntax: CHAN SET #channel axxflags [on|off]
Enable or disable chanflags system. The CHAN ACCESS command automatically adapts itself regarding to this option. See HELP CHAN ACCESS for more informations about chanflags.
```
### enablemask
```
Syntax: CHAN SET #channel enablemask [on|off]
This option allows masks in access list. Example: you can set a level of -2 on *!*@lamerhost.net to autokickban all users matching this mask
Note that the SECURE option has no effect with masks, and that masks are only effective with auto-voice/halfop/op features (you can't add a mask with a level > 5)
```
### enftopic
```
Syntax: CHAN SET #channel enftopic [on|off]
This option defines whether the services enforces the topic defined by the CHAN TOPIC command.
```
### founder
```
Syntax: CHAN SET #channel founder user
Change the channel founder.
```
### mass
```
Syntax: CHAN SET #channel mass [on|off]
This option enable or disable mass-effect channel comands like !rkick, !rkb, !kick * and !kb *
```
### mlock
```
Syntax: CHAN SET #channel mlock [modes]
This define the modes who must be forcefully set. If no param is given after the '+' or the '-', mlock is cleared.
A minimum level of 20 is required.

```
### noauto
```
Syntax: CHAN SET #channel mlock [modes]
This define the modes who must be forcefully set. If no param is given after the '+' or the '-', mlock is cleared.
A minimum level of 20 is required.

```
### nojoin
```
Syntax: CHAN SET #channel nojoin [on|off]
When this option is enabled, C will leave your channel, operating from outside.
A minimum level of 20 is required.

```
### private
```
Syntax: CHAN SET #channel nojoin [on|off]
When this option is enabled, C will leave your channel, operating from outside.
A minimum level of 20 is required.

```
### protectops
```
Syntax: CHAN SET #channel protectops [on|off]
This option prevents ops having an access from being deopped
The protection can be overridden by the !deop command.
```
### secure
```
Syntax: CHAN SET #channel secure [on|off]
If this option is enabled, users who are not identified won't be auto-opped or auto-voiced if the aop or avoice option are set.
```
### strictop
```
Syntax: CHAN SET #channel strictop [on|off]
When this option is enabled, only the users having a sufficient level can be opped
A minimum level of 20 is required.
```

## suspend
```
Syntax: CHAN SUSPEND #channel username
Suspend the access level of a user
```
## topic
```
Syntax: CHAN TOPIC #chan [topic]
Set the topic
```
## unassign
```
Syntax: CHAN UNASSIGN #channel
Unassign a bot from a channel.
```
## unbanall
```
Syntax: CHAN UNBANALL #channel
Remove all bans
```
## unsuspend
```
Syntax: CHAN UNSUSPEND #channel username
Unsuspend the access level of a user
```
## voice
```
Syntax: CHAN VOICE #channel [nick]
Give voice status
```