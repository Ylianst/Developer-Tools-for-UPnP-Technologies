
#
# Intel's UPnP Micro Control Point/1.0, Sample MakeFile */
# Intel DeviceBuilder Build#1.0.1189.31920
#

# List all object files here
OFILES = \
	Main.o \
	RNDControlPoint.o\
	ILibHTTPClient.o\
	ILibSSDPClient.o\
	ILibParsers.o\
	ILibMiniWebServer.o\

# Compiler command name
CC = gcc

# Compiler flags applied to all files
CFLAGS = -Os -Wall -D_POSIX -D_DEBUG -D_VERBOSE -D_MT_CONNECTION_MANAGER -D_MT_RENDERINGCONTROL -D_MT_AVTRANSPORT -lpthread
 
# Name of the executable
EXENAME = Main

# Builds all object files and executable
${EXENAME} : $(OFILES)
	$(CC) $(CFLAGS) -o ${EXENAME} $(OFILES)

# Macro rule for all object files.
$(OFILES) : \
	RNDControlPoint.h ILibParsers.h ILibHTTPClient.h ILibSSDPClient.h ILibMiniWebServer.h

# Clean up
clean :
	rm -f $(OFILES) ${EXENAME} ${EXENAME}.exe
