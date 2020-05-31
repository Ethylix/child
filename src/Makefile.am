bin_PROGRAMS = child
child_SOURCES = botserv.c base64.c channel.c child.c commands.c core.c db.c filter.c hashmap.c loadconf.c md5.c mem.c modules.c net.c partyline.c parseline.c string_utils.c trust.c user.c

module:
	$(CC) -o modules/$(MODFILE).so $(CFLAGS) -I.. $(LDFLAGS) -fPIC -shared modules/$(MODFILE).c
