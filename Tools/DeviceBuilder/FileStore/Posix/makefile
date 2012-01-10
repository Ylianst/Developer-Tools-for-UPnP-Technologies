
#
# Intel's UPnP MicroStack/1.0, Sample MakeFile */
# Intel Device Builder - Build#{{{BUILD_NUMBER}}}
#

# List all object files here
OFILES = \
	Main.o \
	ILibParsers.o\
	ILibAsyncSocket.o\
	ILibAsyncUDPSocket.o\
	ILibAsyncServerSocket.o\
	ILibWebClient.o\
	ILibWebServer.o\
	//{{{BEGIN_C++}}}UPnPAbstraction.o\
	//{{{END_C++}}}//{{{BEGIN_THREADPOOL}}}ILibThreadPool.o\
	//{{{END_THREADPOOL}}}{{{O_FILES}}}

# Compiler command name
CC = gcc

# Compiler flags applied to all files
CFLAGS = -Wall -D_POSIX -D_DEBUG -D_VERBOSE -DMICROSTACK_NOTLS
 
LIBS = -lpthread -lm //{{{BEGIN_C++}}}-lstdc++//{{{END_C++}}}
# Name of the executable
EXENAME = Main

# Builds all object files and executable
${EXENAME} : $(OFILES)
	$(CC) $(CFLAGS) -o ${EXENAME} $(OFILES) $(LIBS)

# Macro rule for all object files.
$(OFILES) : \
	{{{H_FILES}}}

# Clean up
clean :
	rm -f $(OFILES) ${EXENAME} ${EXENAME}.exe
