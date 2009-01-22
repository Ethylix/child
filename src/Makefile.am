bin_PROGRAMS = child
child_SOURCES = botserv.c base64.c channel.c child.c commands.c db.c filter.c hash.c loadconf.c md5.c mem.c modules.c mysql.c net.c partyline.c parseline.c str.c trust.c user.c

module:
	$(CC) -o modules/$(MODFILE).so $(CFLAGS) -I.. $(LDFLAGS) -fPIC -shared modules/$(MODFILE).c
