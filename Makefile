CC		= @gcc
RM		= @rm
MKDIR	= @mkdir
ECHO 	= @echo
INSTALL = @install

OE		= o

vpath	%.c ../
vpath	%.h ../

PROG	= nokout_daemon
DEPS	= PCD8544.h

OBJS	= nokout_daemon.$(OE)
OBJS	+= PCD8544.$(OE)

OBJDIR	= obj

_OBJS	= $(patsubst %,$(OBJDIR)/%,$(OBJS))
LIBS	= -lwiringPi

INCLUDE = -I. -I/include -I../

CFLAGS	= -Wall
ifeq ($(DEBUG),yes)
CFLAGS	+= -g
CFLAGS	+= -O0
else
CFLAGS	+= -O3
endif


.PHONY: all clean

all: $(OBJDIR) $(_OBJS)
	$(ECHO) "Building $(PROG)"
	$(CC) -o $(PROG) $(_OBJS) $(INCLUDE) $(LIBS)

install: $(PROG)
	$(INSTALL) -m 0655 $(PROG) /usr/sbin
	$(INSTALL) -m 0755 nokout /etc/init.d
	@update-rc.d -f nokout start 80 2 3 4 5 . stop 30 0 1 6

uninstall:
	$(RM) -f /usr/sbin/$(PROG)
	@update-rc.d -f nokout remove
	$(RM) -f /etc/init.d/nokout

$(OBJDIR)/%.o: %.c $(DEPS)
	$(ECHO) "Compiling $<"
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJDIR):
	$(MKDIR) -p $(OBJDIR)
clean:
	$(RM) -fr $(PROG)
	$(RM) -fr $(OBJDIR)
	$(RM) -fr '*~'
