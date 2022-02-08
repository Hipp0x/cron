#### HOW TO USE ####
# To add a file to cassini :
# - add the dir/name.o to the line CASSINI_OBJFILES
# - don't forget the .o instead of .c => the object files are created
#   automatically from the source files
#
# Don't touch anything else, the variables are here to do the work for you

CC ?= gcc
CLFAGS ?= -Wall

# tell the linker where to find the .h files
LIBINCLUDE = include
CFLAGS += -I$(LIBINCLUDE)

# all the object files cassini needs
CASSINI_OBJFILES = src/cassini.o src/timing-text-io.o src/pipes.o src/write-request.o src/read-reply.o src/print-reply.o src/common-read.o

# all the object files saturnd needs
SATURND_OBJFILES = src/saturnd.o src/read-request.o src/write-reply.o src/pipes.o src/common-read.o src/folder.o src/create-task.o src/common-folder.o src/run_task.o


%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# compile both the client and the daemon
all: cassini saturnd

# the client
cassini: $(CASSINI_OBJFILES)
	$(CC) $(CFLAGS) -o cassini $(CASSINI_OBJFILES)

# the daemon
saturnd: $(SATURND_OBJFILES)
	$(CC) $(CFLAGS) -o saturnd $(SATURND_OBJFILES)

# to remove the compiled files
.PHONY: clean
clean:
	rm -f cassini $(CASSINI_OBJFILES)
	rm -f saturnd $(SATURND_OBJFILES)
