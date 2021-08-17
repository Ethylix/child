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
18:44 -- C (cserve@geeknode.org): Syntax: HELP CHAN ACCESS FLAGS for help about chanflags
18:44 -- C (cserve@geeknode.org): Syntax: HELP CHAN ACCESS LEVELS for help about old levels system
18:44 -- C (cserve@geeknode.org): The CHAN ACCESS command automatically adapts himself regarding to the AXXFLAGS channel option.
```
### flags
```
18:44 -- C (cserve@geeknode.org): Syntax: CHAN ACCESS #channel [username {+|-}flags]
18:44 -- C (cserve@geeknode.org):
18:44 -- C (cserve@geeknode.org): Following channel flags are available :
18:44 -- C (cserve@geeknode.org):     o   Can get/give/take op
18:44 -- C (cserve@geeknode.org):     O   Get auto-op on join
18:44 -- C (cserve@geeknode.org):     v   Can get voice
18:44 -- C (cserve@geeknode.org):     V   Get auto-voice on join
18:44 -- C (cserve@geeknode.org):     x   Can modify access list
18:44 -- C (cserve@geeknode.org):     i   Can get invited
18:44 -- C (cserve@geeknode.org):     t   Can change topic
18:44 -- C (cserve@geeknode.org):     F   Channel owner, can only be modified with CHAN SET FOUNDER command
18:44 -- C (cserve@geeknode.org):     f   Channel co-owner
18:44 -- C (cserve@geeknode.org):     h   Can get halfop
18:44 -- C (cserve@geeknode.org):     H   Get auto-halfop on join
18:44 -- C (cserve@geeknode.org):     s   Can use CHAN SET commands
18:44 -- C (cserve@geeknode.org):     p   Can get protect status
18:44 -- C (cserve@geeknode.org):     P   Get auto-protect on join (+a mode)
18:44 -- C (cserve@geeknode.org):     N   Can't get any status (op/voice/etc)
18:44 -- C (cserve@geeknode.org):     k   Auto-kick on join
18:44 -- C (cserve@geeknode.org):     b   Auto-kickban on join
18:44 -- C (cserve@geeknode.org):     w   Get auto-owner on join (+q mode)
18:44 -- C (cserve@geeknode.org):
18:44 -- C (cserve@geeknode.org): Channel flags are cumulative. Example: +oOV flags will give auto-op and auto-voice on join.
```

### levels
```
18:47 -- C (cserve@geeknode.org): Syntax: CHAN ACCESS #channel {add|auto|del|list} [nick [level]]
18:47 -- C (cserve@geeknode.org):
18:47 -- C (cserve@geeknode.org): Following access levels are available :
18:47 -- C (cserve@geeknode.org):       9999  Co-Owner. Get +qo modes and inherits channel ownership if the original owner expires.
18:47 -- C (cserve@geeknode.org):       20   Full control over the channel without DROP nor SET FOUNDER
18:47 -- C (cserve@geeknode.org):       10   can use access add/del command and set some options
18:47 -- C (cserve@geeknode.org):        5   auto-op
18:47 -- C (cserve@geeknode.org):        4   auto-halfop
18:47 -- C (cserve@geeknode.org):        3   auto-voice
18:47 -- C (cserve@geeknode.org):        1   can use invite command
18:47 -- C (cserve@geeknode.org):       -1   cannot get opped
18:47 -- C (cserve@geeknode.org):       -2   auto-kicked
18:47 -- C (cserve@geeknode.org):       -3   auto-banned
18:47 -- C (cserve@geeknode.org): The maximum level is 9999.
18:47 -- C (cserve@geeknode.org):
18:47 -- C (cserve@geeknode.org): Usage of AUTO param :
18:47 -- C (cserve@geeknode.org): CHAN ACCESS #channel AUTO nick {op|voice|default|off}
18:47 -- C (cserve@geeknode.org): OP: the user is auto-opped if his/her level >= 5
18:47 -- C (cserve@geeknode.org): VOICE: the user is auto-voiced if his/her level >= 3
18:47 -- C (cserve@geeknode.org): DEFAULT: the user is set the mode corresponding to his level
18:47 -- C (cserve@geeknode.org): OFF: no auto-mode is set
```
## addbot
```
18:48 -- C (cserve@geeknode.org): Syntax: CHAN ADDBOT nick ident host
18:48 -- C (cserve@geeknode.org): Add a bot
```
## assign
```
18:48 -- C (cserve@geeknode.org): Syntax: CHAN ASSIGN #channel botnick
18:48 -- C (cserve@geeknode.org): Assign a bot to a channel. This will automatically set NOJOIN option and make C leaving the channel.
```

## banlist
```
18:49 -- C (cserve@geeknode.org): Syntax: CHAN BANLIST #channel
18:49 -- C (cserve@geeknode.org): List channel bans (added with !tb command)
```

## botlist
```
18:49 -- C (cserve@geeknode.org): Syntax: CHAN BOTLIST
18:49 -- C (cserve@geeknode.org): List all available bots
```

## clearbans
```
18:50 -- C (cserve@geeknode.org): Syntax: CHAN CLEARBANS #channel
18:50 -- C (cserve@geeknode.org): Clear internal channel banlist.
```

## clearchan
```
18:50 -- C (cserve@geeknode.org): Syntax: CHAN CLEARCHAN #channel
18:50 -- C (cserve@geeknode.org): Remove all channel modes, bans, excepts, invex, user status, ...
```

## clearmodes
```
18:50 -- C (cserve@geeknode.org): Syntax: CHAN CLEARMODES #channel
18:50 -- C (cserve@geeknode.org): Remove all channel modes
```

## dehalfop
```
18:50 -- C (cserve@geeknode.org): Syntax: CHAN DEHALFOP #channel [nick]
18:50 -- C (cserve@geeknode.org): Take channel half-operator status
```

## delbot
```
19:09 -- C (cserve@geeknode.org): Syntax: CHAN DELBOT nick
19:09 -- C (cserve@geeknode.org): Delete a bot
```

## deop
```
19:04 -- C (cserve@geeknode.org): Syntax: CHAN DEOP #channel [nick]
19:04 -- C (cserve@geeknode.org): Take channel operator status
```

## devoice
```
19:04 -- C (cserve@geeknode.org): Syntax: CHAN DEVOICE #channel [nick]
19:04 -- C (cserve@geeknode.org): Take voice status
```

## drop
```
19:04 -- C (cserve@geeknode.org): Syntax: CHAN DROP #channel
19:04 -- C (cserve@geeknode.org): Drop specified channel
```

## entrymsg
```
19:04 -- C (cserve@geeknode.org): Syntax: CHAN ENTRYMSG #channel [message]
19:04 -- C (cserve@geeknode.org): Set a welcome message displayed on join. If no parameter is specified, then entrymsg is removed
```

## halfop
```
19:04 -- C (cserve@geeknode.org): Syntax: CHAN HALFOP #channel [nick]
19:04 -- C (cserve@geeknode.org): Give channel half-operator status
```

## info
```
19:06 -- C (cserve@geeknode.org): Syntax: CHAN INFO #channel
19:06 -- C (cserve@geeknode.org): Give some informations for the specified channel
```

## invite
```
19:09 -- C (cserve@geeknode.org): Syntax: CHAN INVITE #channel
19:09 -- C (cserve@geeknode.org): Invite yourself on a channel
```
## kick
```
19:06 -- C (cserve@geeknode.org): Syntax: CHAN KICK #channel nick [reason]
19:06 -- C (cserve@geeknode.org): Kick someone or all users
```
## op
```
19:06 -- C (cserve@geeknode.org): Syntax: CHAN KICK #channel nick [reason]
19:06 -- C (cserve@geeknode.org): Kick someone or all users
```
## register
```
19:06 -- C (cserve@geeknode.org): Syntax: CHAN REGISTER #channel
19:06 -- C (cserve@geeknode.org): Register specified channel
19:06 -- C (cserve@geeknode.org): Chans expire after 5000 days without being used.
```
## resync
```
19:06 -- C (cserve@geeknode.org): Syntax: CHAN RESYNC #channel
19:06 -- C (cserve@geeknode.org): Resync channel access list with channel members
```
## set
### aop
```
19:11 -- C (cserve@geeknode.org): Syntax: CHAN SET #channel aop [on|off]
19:11 -- C (cserve@geeknode.org): WHen the option is enabled, users are automatically opped.
19:11 -- C (cserve@geeknode.org): A minimum level of 20 is required.
```
### autolimit
```
19:11 -- C (cserve@geeknode.org): Syntax: CHAN SET #channel autolimit [limit]
19:11 -- C (cserve@geeknode.org): Forcefully set a limit. The limit depends on the numbers of the members present on the channel
19:11 -- C (cserve@geeknode.org): E.g.: If there is 5 members and an autolimit set to 3, C will set the limit to 8
```
### avoice
```
9:11 -- C (cserve@geeknode.org): Syntax: CHAN SET #channel avoice [on|off]
19:11 -- C (cserve@geeknode.org): When the option is enabled, users are automatically voiced.
19:11 -- C (cserve@geeknode.org): A minimum level of 10 is required.
```
### axxflags
```
19:11 -- C (cserve@geeknode.org): Syntax: CHAN SET #channel axxflags [on|off]
19:11 -- C (cserve@geeknode.org): Enable or disable chanflags system. The CHAN ACCESS command automatically adapts itself regarding to this option. See HELP CHAN ACCESS for more informations about chanflags.
```
### enablemask
```
19:13 -- C (cserve@geeknode.org): Syntax: CHAN SET #channel enablemask [on|off]
19:13 -- C (cserve@geeknode.org): This option allows masks in access list. Example: you can set a level of -2 on *!*@lamerhost.net to autokickban all users matching this mask
19:13 -- C (cserve@geeknode.org): Note that the SECURE option has no effect with masks, and that masks are only effective with auto-voice/halfop/op features (you can't add a mask with a level > 5)
```
### enftopic
```
19:13 -- C (cserve@geeknode.org): Syntax: CHAN SET #channel enftopic [on|off]
19:13 -- C (cserve@geeknode.org): This option defines whether the services enforces the topic defined by the CHAN TOPIC command.
```
### founder
```
19:13 -- C (cserve@geeknode.org): Syntax: CHAN SET #channel founder user
19:13 -- C (cserve@geeknode.org): Change the channel founder.
```
### mass
```
19:13 -- C (cserve@geeknode.org): Syntax: CHAN SET #channel mass [on|off]
19:13 -- C (cserve@geeknode.org): This option enable or disable mass-effect channel comands like !rkick, !rkb, !kick * and !kb *
```
### mlock
```
19:13 -- C (cserve@geeknode.org): Syntax: CHAN SET #channel mlock [modes]
19:13 -- C (cserve@geeknode.org): This define the modes who must be forcefully set. If no param is given after the '+' or the '-', mlock is cleared.
19:13 -- C (cserve@geeknode.org): A minimum level of 20 is required.

```
### noauto
```
19:13 -- C (cserve@geeknode.org): Syntax: CHAN SET #channel mlock [modes]
19:13 -- C (cserve@geeknode.org): This define the modes who must be forcefully set. If no param is given after the '+' or the '-', mlock is cleared.
19:13 -- C (cserve@geeknode.org): A minimum level of 20 is required.

```
### nojoin
```
19:13 -- C (cserve@geeknode.org): Syntax: CHAN SET #channel nojoin [on|off]
19:13 -- C (cserve@geeknode.org): When this option is enabled, C will leave your channel, operating from outside.
19:13 -- C (cserve@geeknode.org): A minimum level of 20 is required.

```
### private
```
19:13 -- C (cserve@geeknode.org): Syntax: CHAN SET #channel nojoin [on|off]
19:13 -- C (cserve@geeknode.org): When this option is enabled, C will leave your channel, operating from outside.
19:13 -- C (cserve@geeknode.org): A minimum level of 20 is required.

```
### protectops
```
19:13 -- C (cserve@geeknode.org): Syntax: CHAN SET #channel protectops [on|off]
19:13 -- C (cserve@geeknode.org): This option prevents ops having an access from being deopped
19:13 -- C (cserve@geeknode.org): The protection can be overridden by the !deop command.
```
### secure
```
19:13 -- C (cserve@geeknode.org): Syntax: CHAN SET #channel secure [on|off]
19:13 -- C (cserve@geeknode.org): If this option is enabled, users who are not identified won't be auto-opped or auto-voiced if the aop or avoice option are set.
```
### strictop
```
19:13 -- C (cserve@geeknode.org): Syntax: CHAN SET #channel strictop [on|off]
19:13 -- C (cserve@geeknode.org): When this option is enabled, only the users having a sufficient level can be opped
19:13 -- C (cserve@geeknode.org): A minimum level of 20 is required.
```

## suspend
```
19:09 -- C (cserve@geeknode.org): Syntax: CHAN SUSPEND #channel username
19:09 -- C (cserve@geeknode.org): Suspend the access level of a user
```
## topic
```
19:09 -- C (cserve@geeknode.org): Syntax: CHAN TOPIC #chan [topic]
19:09 -- C (cserve@geeknode.org): Set the topic
```
## unassign
```
19:09 -- C (cserve@geeknode.org): Syntax: CHAN UNASSIGN #channel
19:09 -- C (cserve@geeknode.org): Unassign a bot from a channel.
```
## unbanall
```
19:09 -- C (cserve@geeknode.org): Syntax: CHAN UNBANALL #channel
19:09 -- C (cserve@geeknode.org): Remove all bans
```
## unsuspend
```
19:10 -- C (cserve@geeknode.org): Syntax: CHAN UNSUSPEND #channel username
19:10 -- C (cserve@geeknode.org): Unsuspend the access level of a user
```
## voice
```
19:10 -- C (cserve@geeknode.org): Syntax: CHAN VOICE #channel [nick]
19:10 -- C (cserve@geeknode.org): Give voice status
```