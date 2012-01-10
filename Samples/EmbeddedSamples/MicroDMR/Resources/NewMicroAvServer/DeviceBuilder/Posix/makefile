
#
# Intel's UPnP MicroStack/1.0, Sample MakeFile */
# Intel Device Builder - Build#1.0.1189.22189
#

# List all object files here
OFILES = \
	Main.o \
	ILibParsers.o\
	ILibHTTPClient.o\
	UpnpMicroStack.o

# Compiler command name
CC = gcc

# Compiler flags applied to all files
CFLAGS = -Os -Wall -D_POSIX -D_DEBUG -D_VERBOSE -lpthread 
 
# Name of the executable
EXENAME = Main

# Builds all object files and executable
${EXENAME} : $(OFILES)
	$(CC) $(CFLAGS) -o ${EXENAME} $(OFILES)

# Macro rule for all object files.
$(OFILES) : \
	UpnpMicroStack.h

# Clean up
clean :
	rm -f $(OFILES) ${EXENAME} ${EXENAME}.exe
