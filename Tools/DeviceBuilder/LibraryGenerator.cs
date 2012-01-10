/*   
Copyright 2006 - 2010 Intel Corporation

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

using System;
using System.IO;
using System.Net;
using System.Text;
using System.Collections;
using System.Net.Sockets;
using OpenSource.UPnP;

namespace UPnPStackBuilder
{
	/// <summary>
	/// Summary description for EmbeddedCGenerator.
	/// </summary>
	public class LibraryGenerator
	{
		public enum PLATFORMS 
		{
			WINDOWS,
			POSIX
		}

		public enum SUBTARGETS
		{
			NONE,
			PPC2003,
			NUCLEUS,
			PSOS
		}

		public enum LANGUAGES 
		{
			C,
			CPP
		}

		public PLATFORMS Platform = PLATFORMS.POSIX;
		public SUBTARGETS SubTarget = SUBTARGETS.NONE;
		public LANGUAGES Language = LANGUAGES.C;

		public delegate void LogOutputHandler(object sender, string msg);
		public event LogOutputHandler OnLogOutput;

		public string VersionString = "";
		public int WinSock = 1;

		private static string cl = "\r\n";
		public string CodeNewLine 
		{
			get {return cl;}
			set {cl = value;}
		}
		public string CallingConvention = "";
		public string CallPrefix = "UPnP";
		public string CallLibPrefix = "ILib";
		public string CodeTab = "\t";
		public string License = "";
		public string ClassName = "MicroStack";

		private string pc_SockType = "int";
//		private string pc_TimeType = "struct timeval";
		private string pc_SockClose = "close";
//		private string pc_stricmp = "strncasecmp";
		private string pc_methodPrefix = "UPnP";
		private string pc_methodLibPrefix = "ILib";
		private string pc_methodPrefixDef = "UPnP";
//		private string pc_inline = "";
//		private string pc_classPrefix = "";
//		private static CodeProcessor PrivateClassDeclarations;
//		private static CodeProcessor PublicClassDeclarations;

		public LibraryGenerator()
		{
		}
		
		private void Log(string msg) 
		{
			if (OnLogOutput != null) OnLogOutput(this,msg);
		}

		private class CodeProcessor 
		{
			public StringBuilder SB;
			int ident = 0;
			string temp = "";
			static public int Setting = 0;
			bool cppCommentStyle;
//			public CodeProcessor ClassDefinitions;
//			public CodeProcessor PublicClassDefinitions;
			public string CodeTab = "\t";

			public CodeProcessor(StringBuilder sb, bool cppCommentStyle) 
			{
				SB = sb;
				this.cppCommentStyle = cppCommentStyle;
			}

			public override string ToString() 
			{
				return SB.ToString() + temp;
			}

			public void Comment(string comment) 
			{
				if (cppCommentStyle == true) 
				{
					Append("// " + comment + cl);
				} 
				else 
				{
					Append("/* " + comment + " */" + cl);
				}
			}

//			public void Define(string code)
//			{
//				this.Append(code + cl);
//				ClassDefinitions.Append(code + ";" + cl);
//			}
//
//			public void DefinePublic(string code)
//			{
//				this.Append(code + cl);
//				PublicClassDefinitions.Append(code + ";" + cl);
//			}

			public void Append(string code) 
			{
				code.Replace("\t"," ");
				temp = temp + code;

				int pos = temp.IndexOf(cl);
				while (pos >= 0) 
				{
					// Fetch the code line
					string codeline = temp.Substring(0,pos);

					// Trim the code
					codeline.Trim();
					while (codeline.StartsWith("\t")) {codeline = codeline.Substring(1);}
					while (codeline.StartsWith(" ")) {codeline = codeline.Substring(1);}
					while (codeline.StartsWith("\t")) {codeline = codeline.Substring(1);}
					while (codeline.StartsWith(" ")) {codeline = codeline.Substring(1);}

					// Ident the code
					int diff = 0;
					if (codeline.StartsWith("}") || codeline.StartsWith("};")) diff = -1;
					if (Setting == 0) 
					{
						for (int i=0;i<(ident + diff);i++) 
						{
							codeline = CodeTab + codeline;
						}
					}

					// Update ident value
					int p = codeline.IndexOf("{");
					while (p >= 0) 
					{
						ident++;
						p = codeline.IndexOf("{",p+1);
					}

					p = codeline.IndexOf("}");
					while (p >= 0) 
					{
						ident--;
						p = codeline.IndexOf("}",p+1);
					}

					// Write code out
					SB.Append(codeline + cl);
					temp = temp.Substring(pos + cl.Length);
					pos = temp.IndexOf(cl);
				}
			}
		}

		private void AddLicense(CodeProcessor cs,string filename) 
		{
			string l = License;
			l = l.Replace("<FILE>",filename);
			cs.Append(l);
		}
		public bool Build_UPnPMiniWebServer(DirectoryInfo outputDirectory)
		{
			StreamWriter writer;

			if (this.Platform == PLATFORMS.POSIX) 
			{
				if (this.SubTarget == SUBTARGETS.NUCLEUS) 
				{
					pc_SockType = "int";
//					pc_TimeType = "Timeval";
					pc_SockClose = "NU_Close_Socket";
//					pc_stricmp = "strncasecmp";					
				} 
				else
				{
					pc_SockType = "int";
//					pc_TimeType = "struct timeval";
					pc_SockClose = "close";
//					pc_stricmp = "strncasecmp";
				}
			}

			if (this.Platform == PLATFORMS.WINDOWS) 
			{
				pc_SockType = "SOCKET";
//				pc_TimeType = "unsigned int";
				pc_SockClose = "closesocket";
//				pc_stricmp = "_strnicmp";
			}

			if (this.Language == LANGUAGES.C)
			{
				pc_methodPrefix = CallPrefix;
				pc_methodLibPrefix = CallLibPrefix;
				pc_methodPrefixDef = CallingConvention + CallPrefix;
			}


			CodeProcessor cs = new CodeProcessor(new StringBuilder(),this.Language == LANGUAGES.CPP);

			/* Build UPnPMiniWebServer.h */
			AddLicense(cs,pc_methodPrefix+"MiniWebServer.h");

			cs.Append(cl);

			cs.Append("#ifndef __"+pc_methodPrefix+"MiniWebServer__"+cl);
			cs.Append("#define __"+pc_methodPrefix+"MiniWebServer__"+cl);

			cs.Append(cl);
			cs.Comment("Forward Declaration");
			cs.Append("struct packetheader;"+cl+cl);

			cs.Append("void* "+pc_methodPrefix+"CreateMiniWebServer(void *chain,int MaxSockets,void (*OnReceive) (void *ReaderObject, struct packetheader *header, char* buffer, int *BeginPointer, int BufferSize, int done, void* user),void *user);"+cl);
			cs.Append("void "+pc_methodPrefix+"DestroyMiniWebServer(void *WebServerModule);"+cl);
			cs.Append("void "+pc_methodPrefix+"StartMiniWebServerModule(void *WebServerModule);"+cl);
			cs.Append("void "+pc_methodPrefix+"StopMiniWebServerModule(void *WebServerModule);"+cl);
			cs.Append(cl);
			cs.Append("void "+pc_methodPrefix+"MiniWebServer_SetReserved(void *MWS, void *object);"+cl);
			cs.Append("void *"+pc_methodPrefix+"MiniWebServer_GetReserved(void *MWS);"+cl);
			cs.Append("void *"+pc_methodPrefix+"MiniWebServer_GetMiniWebServerFromReader(void *Reader);"+cl);
			cs.Append(cl);
			cs.Append("int "+pc_methodPrefix+"GetMiniWebServerPortNumber(void *WebServerModule);"+cl);
			cs.Append("void "+pc_methodPrefix+"MiniWebServerSend(void *ReaderObject, struct packetheader *packet);"+cl);
			cs.Append("void "+pc_methodPrefix+"MiniWebServerCloseSession(void *ReaderObject);"+cl);
			cs.Append(cl);
			cs.Append("char* "+pc_methodPrefix+"GetReceivingInterface(void* ReaderObject);"+cl);
			cs.Append("void "+pc_methodPrefix+"CloseRequest(void* ReaderObject);	"+cl);

			cs.Append(cl);
			cs.Append("#endif"+cl);

			writer = File.CreateText(outputDirectory.FullName + "\\"+pc_methodPrefix+"MiniWebServer.h");
			writer.Write(cs.ToString());
			writer.Close();


			/* Build UPnPMiniWebServer.c */
			cs = new CodeProcessor(new StringBuilder(),this.Language == LANGUAGES.CPP);
			if (this.Language == LANGUAGES.CPP) 
			{
				AddLicense(cs,pc_methodPrefix+"MiniWebServer.cpp");
			} 
			else 
			{
				AddLicense(cs,pc_methodPrefix+"MiniWebServer.c");
			}
			cs.Append(cl);

			if (this.Platform==PLATFORMS.WINDOWS)
			{
				cs.Append("#ifndef MICROSTACK_NO_STDAFX"+cl);
				cs.Append("#include \"stdafx.h\""+cl);
				cs.Append("#endif"+cl);
				cs.Append("#define _CRTDBG_MAP_ALLOC"+cl);
				cs.Append("#include <math.h>"+cl);
				cs.Append("#include <winerror.h>"+cl);					
				cs.Append("#include <stdlib.h>"+cl);
				cs.Append("#include <stdio.h>"+cl);
				cs.Append("#include <stddef.h>"+cl);
				cs.Append("#include <string.h>"+cl);
				if (this.WinSock == 1) 
				{
					cs.Append("#include <winsock.h>"+cl);
					cs.Append("#include <wininet.h>"+cl);
				}
				if (this.WinSock == 2) 
				{
					cs.Append("#include <winsock2.h>"+cl);
					cs.Append("#include <ws2tcpip.h>"+cl);
				}
				if (this.SubTarget != SUBTARGETS.PPC2003) 
				{
//					cs.Append("#include <errno.h>"+cl);
				}
				cs.Append("#include <windows.h>"+cl);
				cs.Append("#include <winioctl.h>"+cl);
				cs.Append("#include <winbase.h>"+cl);
				cs.Append("#include <crtdbg.h>"+cl);
			}
			else
			{
				if (this.SubTarget == SUBTARGETS.NUCLEUS) 
				{
					cs.Append("#include <stdio.h>"+cl);
					cs.Append("#include <stdlib.h>"+cl);
					cs.Append("#include \"net/inc/externs.h\""+cl);
					cs.Append("#include \"net/inc/ip.h\""+cl);
					cs.Append("#include \"net/inc/socketd.h\""+cl);
					cs.Append("#include <errno.h>"+cl);
				}
				else 
				{
					cs.Append("#include <stdio.h>"+cl);
					cs.Append("#include <stdlib.h>"+cl);
					cs.Append("#include <sys/types.h>"+cl);
					cs.Append("#include <sys/socket.h>"+cl);
					cs.Append("#include <netinet/in.h>"+cl);
					cs.Append("#include <arpa/inet.h>"+cl);
					cs.Append("#include <sys/time.h>"+cl);
					cs.Append("#include <netdb.h>"+cl);
					cs.Append("#include <string.h>"+cl);
					cs.Append("#include <sys/ioctl.h>"+cl);
					cs.Append("#include <net/if.h>"+cl);
					cs.Append("#include <sys/utsname.h>"+cl);
					cs.Append("#include <sys/socket.h>"+cl);
					cs.Append("#include <netinet/in.h>"+cl);
					cs.Append("#include <unistd.h>"+cl);
					cs.Append("#include <fcntl.h>"+cl);
					//cs.Append("#include <errno.h>"+cl);
					cs.Append("#include <malloc.h>"+cl);
					cs.Append("#include <semaphore.h>"+cl);
				}
			}
			cs.Append(cl);
			cs.Append("#include \""+pc_methodPrefix+"MiniWebServer.h\""+cl);
			cs.Append("#include \""+pc_methodPrefix+"Parsers.h\""+cl);

			if (this.Platform == PLATFORMS.WINDOWS)
			{
				cs.Append("#define strncasecmp(x,y,z) _strnicmp(x,y,z)"+cl);
				cs.Append("#define gettimeofday(x,y) (x)->tv_sec = GetTickCount()/1000"+cl);
			}

			if (this.Platform != PLATFORMS.POSIX)
			{
				cs.Append("#define sem_t HANDLE"+cl);
				cs.Append("#define sem_init(x,y,z) *x=CreateSemaphore(NULL,z,FD_SETSIZE,NULL)"+cl);
				cs.Append("#define sem_destroy(x) (CloseHandle(*x)==0?1:0)"+cl);
				cs.Append("#define sem_wait(x) WaitForSingleObject(*x,INFINITE)"+cl);
				cs.Append("#define sem_trywait(x) ((WaitForSingleObject(*x,0)==WAIT_OBJECT_0)?0:1)"+cl);
				cs.Append("#define sem_post(x) ReleaseSemaphore(*x,1,NULL)"+cl);
			}

			cs.Append("#define DEBUGSTATEMENT(x)"+cl);
			cs.Append(cl);

			cs.Append("struct MiniWebServerObject"+cl);
			cs.Append("{"+cl);
			cs.Append("	void (*PreSelect)(void* object,fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime);"+cl);
			cs.Append("	void (*PostSelect)(void* object,int slct, fd_set *readset, fd_set *writeset, fd_set *errorset);"+cl);
			cs.Append("	void (*Destroy)(void* object);"+cl);
			cs.Append(cl);
			cs.Append("	struct "+this.pc_methodPrefix+"MWSHTTPReaderObject *Readers;"+cl);
			cs.Append("	"+this.pc_SockType+" ListenSocket;"+cl);
			cs.Append("	int MaxConnections;"+cl);
			cs.Append("	unsigned short PortNumber;"+cl);
			cs.Append("	int Terminate;"+cl);
			cs.Append(cl);
			cs.Append("	void *TimerObject;"+cl);
			cs.Append("};"+cl);

			cs.Append("struct "+this.pc_methodPrefix+"MWSHTTPReaderObject"+cl);
			cs.Append("{"+cl);
			cs.Append("	struct packetheader* PacketHeader;"+cl);
			cs.Append("	char Header[2048];"+cl);
			cs.Append("	char* Body;"+cl);
			cs.Append("	int BodySize;"+cl);
			cs.Append("	int HeaderIndex;"+cl);
			cs.Append("	int LocalIPAddress;"+cl);
			cs.Append("	"+cl);
			cs.Append("	int Body_BeginPointer;"+cl);
			cs.Append("	int Body_EndPointer;"+cl);
			cs.Append("	int Body_MallocSize;"+cl);
			cs.Append("	int Body_Read;"+cl);
			cs.Append("	"+cl);
			cs.Append("	"+this.pc_SockType+" ClientSocket;"+cl);
			cs.Append("	int FinRead;"+cl);
			cs.Append("	struct MiniWebServerObject *Parent;"+cl);
			cs.Append("	void* user;"+cl);
			cs.Append("	void (*FunctionCallback) (void *ReaderObject, struct packetheader *header, char* buffer, int *BeginPointer, int BufferSize, int done, void* user);"+cl);
			cs.Append("};"+cl);



			cs.Append("void "+this.pc_methodPrefix+"MiniWebServerProcessSocket(struct "+this.pc_methodPrefix+"MWSHTTPReaderObject *Reader)"+cl);
			cs.Append("{"+cl);
			cs.Append("	int bytesReceived;"+cl);
			cs.Append("	int i;"+cl);
			cs.Append("	struct packetheader_field_node *node;"+cl);
			cs.Append("	char* CharStar;"+cl);
			cs.Append("	"+cl);
			cs.Append("	if (Reader->BodySize==0)"+cl);
			cs.Append("	{"+cl);
			cs.Append("		/* Still Reading Headers */"+cl);
			cs.Append("		bytesReceived = recv(Reader->ClientSocket,Reader->Header+Reader->HeaderIndex,2048-Reader->HeaderIndex,0);"+cl);
			cs.Append("		if (bytesReceived==0)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			if (Reader->PacketHeader!=NULL) {"+this.pc_methodPrefix+"DestructPacket(Reader->PacketHeader);}"+cl);
			cs.Append("			if (Reader->Body_MallocSize!=0) {free(Reader->Body);}"+cl);
			cs.Append("			Reader->Body = NULL;"+cl);
			cs.Append("			Reader->Body_MallocSize = 0;"+cl);
			cs.Append("			Reader->PacketHeader = NULL;"+cl);
			cs.Append("			"+this.pc_SockClose+"(Reader->ClientSocket);"+cl);
			cs.Append("			Reader->ClientSocket = ~0;"+cl);
			cs.Append("			return;"+cl);
			cs.Append("		}"+cl);
			cs.Append("		Reader->HeaderIndex += bytesReceived;"+cl);
			cs.Append("		if (Reader->HeaderIndex>4)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			/* Must have read at least 4 bytes to perform check */"+cl);
			cs.Append("			for(i=0;i<(Reader->HeaderIndex - 3);i++)"+cl);
			cs.Append("			{"+cl);
			cs.Append("				if (Reader->Header[i] == '\\r' && Reader->Header[i+1] == '\\n' && Reader->Header[i+2] == '\\r' && Reader->Header[i+3] == '\\n')"+cl);
			cs.Append("				{"+cl);
			cs.Append("					/* Finished Header */"+cl);
			cs.Append("					Reader->PacketHeader = "+this.pc_methodPrefix+"ParsePacketHeader(Reader->Header,0,i+4);"+cl);
			cs.Append("					Reader->PacketHeader->ReceivingAddress = Reader->LocalIPAddress;"+cl);
			cs.Append("					Reader->BodySize = -1;"+cl);
			cs.Append("					Reader->Body_Read = 0;"+cl);
			cs.Append("					node = Reader->PacketHeader->FirstField;"+cl);
			cs.Append("					while(node!=NULL)"+cl);
			cs.Append("					{"+cl);
			cs.Append("						if (strncasecmp(node->Field,\"CONTENT-LENGTH\",14)==0)"+cl);
			cs.Append("						{"+cl);
			cs.Append("							CharStar = (char*)malloc(1+node->FieldDataLength);"+cl);
			cs.Append("							memcpy(CharStar,node->FieldData,node->FieldDataLength);"+cl);
			cs.Append("							CharStar[node->FieldDataLength] = '\\0';"+cl);
			cs.Append("							Reader->BodySize = atoi(CharStar);"+cl);
			cs.Append("							free(CharStar);"+cl);
			cs.Append("							break;"+cl);
			cs.Append("						}"+cl);
			cs.Append("						node = node->NextField;"+cl);
			cs.Append("					}"+cl);
			cs.Append("					if (Reader->BodySize!=-1)"+cl);
			cs.Append("					{"+cl);
			cs.Append("						if (Reader->BodySize!=0)"+cl);
			cs.Append("						{"+cl);
			cs.Append("							Reader->Body = (char*)malloc(Reader->BodySize);"+cl);
			cs.Append("							Reader->Body_MallocSize = Reader->BodySize;"+cl);
			cs.Append("						}"+cl);
			cs.Append("						else"+cl);
			cs.Append("						{"+cl);
			cs.Append("							Reader->Body = NULL;"+cl);
			cs.Append("							Reader->Body_MallocSize = 0;"+cl);
			cs.Append("						}"+cl);
			cs.Append("					}"+cl);
			cs.Append("					else"+cl);
			cs.Append("					{"+cl);
			cs.Append("						Reader->Body = (char*)malloc(4096);"+cl);
			cs.Append("						Reader->Body_MallocSize = 4096;"+cl);
			cs.Append("					}"+cl);
			cs.Append(cl);
			cs.Append("					if (Reader->HeaderIndex>i+4 && Reader->BodySize!=0)"+cl);
			cs.Append("					{"+cl);
			cs.Append("						/* Part of the body is in here */"+cl);
			cs.Append("						memcpy(Reader->Body,Reader->Header+i+4,Reader->HeaderIndex-(&Reader->Header[i+4]-Reader->Header));"+cl);
			cs.Append("						Reader->Body_BeginPointer = 0;"+cl);
			cs.Append("						Reader->Body_EndPointer = Reader->HeaderIndex-(int)(&Reader->Header[i+4]-Reader->Header);"+cl);
			cs.Append("						Reader->Body_Read = Reader->Body_EndPointer;"+cl);
			cs.Append("						"+cl);
			cs.Append("						if (Reader->BodySize==-1 || Reader->Body_Read>=Reader->BodySize)"+cl);
			cs.Append("						{"+cl);
			cs.Append("							DEBUGSTATEMENT(printf(\"Close\\r\\n\"));"+cl);
			cs.Append("							Reader->FunctionCallback(Reader,Reader->PacketHeader,Reader->Body,&Reader->Body_BeginPointer,Reader->Body_EndPointer - Reader->Body_BeginPointer,-1,Reader->user);"+cl);
			cs.Append("							"+cl);
			cs.Append("							while(Reader->Body_BeginPointer!=Reader->Body_EndPointer && Reader->Body_BeginPointer!=0)"+cl);
			cs.Append("							{"+cl);
			cs.Append("								memcpy(Reader->Body,Reader->Body+Reader->Body_BeginPointer,Reader->Body_EndPointer-Reader->Body_BeginPointer);"+cl);
			cs.Append("								Reader->Body_EndPointer = Reader->Body_EndPointer-Reader->Body_BeginPointer;"+cl);
			cs.Append("								Reader->Body_BeginPointer = 0;"+cl);
			cs.Append("								Reader->FunctionCallback(Reader,Reader->PacketHeader,Reader->Body,&Reader->Body_BeginPointer,Reader->Body_EndPointer,-1,Reader->user);"+cl);
			cs.Append("							}"+cl);
			cs.Append("							"+cl);
			cs.Append("							if (Reader->PacketHeader!=NULL) {"+this.pc_methodPrefix+"DestructPacket(Reader->PacketHeader);}"+cl);
			cs.Append("							if (Reader->Body_MallocSize!=0) {free(Reader->Body);}"+cl);
			cs.Append("							Reader->Body = NULL;"+cl);
			cs.Append("							Reader->Body_MallocSize = 0;"+cl);
			cs.Append("							Reader->PacketHeader = NULL;"+cl);
			cs.Append("							"+this.pc_SockClose+"(Reader->ClientSocket);"+cl);
			cs.Append("							Reader->ClientSocket = ~0;"+cl);
			cs.Append("						}"+cl);
			cs.Append("						else"+cl);
			cs.Append("						{"+cl);
			cs.Append("							Reader->FunctionCallback(Reader,Reader->PacketHeader,Reader->Body,&Reader->Body_BeginPointer,Reader->Body_EndPointer - Reader->Body_BeginPointer,0,Reader->user);"+cl);
			cs.Append("							while(Reader->Body_BeginPointer!=Reader->Body_EndPointer && Reader->Body_BeginPointer!=0)"+cl);
			cs.Append("							{"+cl);
			cs.Append("								memcpy(Reader->Body,Reader->Body+Reader->Body_BeginPointer,Reader->Body_EndPointer-Reader->Body_BeginPointer);"+cl);
			cs.Append("								Reader->Body_EndPointer = Reader->Body_EndPointer-Reader->Body_BeginPointer;"+cl);
			cs.Append("								Reader->Body_BeginPointer = 0;"+cl);
			cs.Append("								Reader->FunctionCallback(Reader,Reader->PacketHeader,Reader->Body,&Reader->Body_BeginPointer,Reader->Body_EndPointer,0,Reader->user);"+cl);
			cs.Append("							}"+cl);
			cs.Append("						}"+cl);
			cs.Append("					}"+cl);
			cs.Append("					else"+cl);
			cs.Append("					{"+cl);
			cs.Append("						/* There is no body, but the packet is here */"+cl);
			cs.Append("						Reader->Body_BeginPointer = 0;"+cl);
			cs.Append("						Reader->Body_EndPointer = 0;"+cl);
			cs.Append("						"+cl);
			cs.Append("						if (Reader->BodySize<=0)"+cl);
			cs.Append("						{"+cl);
			cs.Append("							Reader->FunctionCallback(Reader,Reader->PacketHeader,NULL,&Reader->Body_BeginPointer,0,-1,Reader->user);"+cl);
			cs.Append("							if (Reader->PacketHeader!=NULL) {"+this.pc_methodPrefix+"DestructPacket(Reader->PacketHeader);}"+cl);
			cs.Append("							if (Reader->Body_MallocSize!=0) {free(Reader->Body);}"+cl);
			cs.Append("							Reader->Body = NULL;"+cl);
			cs.Append("							Reader->Body_MallocSize = 0;"+cl);
			cs.Append("							Reader->PacketHeader = NULL;"+cl);
			cs.Append("							"+this.pc_SockClose+"(Reader->ClientSocket);"+cl);
			cs.Append("							Reader->ClientSocket = ~0;"+cl);
			cs.Append("						}"+cl);
			cs.Append("						else"+cl);
			cs.Append("						{"+cl);
			cs.Append("							Reader->FunctionCallback(Reader,Reader->PacketHeader,NULL,&Reader->Body_BeginPointer,0,0,Reader->user);"+cl);
			cs.Append("						}"+cl);
			cs.Append("					}"+cl);
			cs.Append("					break;"+cl);
			cs.Append("				}"+cl);
			cs.Append("			}"+cl);
			cs.Append("		}"+cl);
			cs.Append("	}"+cl);
			cs.Append("	else"+cl);
			cs.Append("	{"+cl);
			cs.Append("		/* Reading Body Only */"+cl);
			cs.Append("		if (Reader->Body_BeginPointer == Reader->Body_EndPointer)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			Reader->Body_BeginPointer = 0;"+cl);
			cs.Append("			Reader->Body_EndPointer = 0;"+cl);
			cs.Append("		}"+cl);
			cs.Append("		else"+cl);
			cs.Append("		{"+cl);
			cs.Append("			if (Reader->Body_BeginPointer!=0)"+cl);
			cs.Append("			{"+cl);
			cs.Append("				Reader->Body_EndPointer = Reader->Body_BeginPointer;"+cl);
			cs.Append("			}"+cl);
			cs.Append("		}"+cl);
			cs.Append("		"+cl);
			cs.Append("		"+cl);
			cs.Append("		if (Reader->Body_EndPointer == Reader->Body_MallocSize)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			Reader->Body_MallocSize += 4096;"+cl);
			cs.Append("			Reader->Body = (char*)realloc(Reader->Body,Reader->Body_MallocSize);"+cl);
			cs.Append("		}"+cl);
			cs.Append("		"+cl);
			cs.Append("		bytesReceived = recv(Reader->ClientSocket,Reader->Body+Reader->Body_EndPointer,Reader->Body_MallocSize-Reader->Body_EndPointer,0);"+cl);
			cs.Append("		Reader->Body_EndPointer += bytesReceived;"+cl);
			cs.Append("		Reader->Body_Read += bytesReceived;"+cl);
			cs.Append("		"+cl);
			cs.Append("		Reader->FunctionCallback(Reader, Reader->PacketHeader, Reader->Body+Reader->Body_BeginPointer, &Reader->Body_BeginPointer, Reader->Body_EndPointer - Reader->Body_BeginPointer, 0, Reader->user);"+cl);
			cs.Append("		while(Reader->Body_BeginPointer!=Reader->Body_EndPointer && Reader->Body_BeginPointer!=0)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			memcpy(Reader->Body,Reader->Body+Reader->Body_BeginPointer,Reader->Body_EndPointer-Reader->Body_BeginPointer);"+cl);
			cs.Append("			Reader->Body_EndPointer = Reader->Body_EndPointer-Reader->Body_BeginPointer;"+cl);
			cs.Append("			Reader->Body_BeginPointer = 0;"+cl);
			cs.Append("			Reader->FunctionCallback(Reader,Reader->PacketHeader,Reader->Body,&Reader->Body_BeginPointer,Reader->Body_EndPointer,0,Reader->user);				"+cl);
			cs.Append("		}"+cl);
			cs.Append("		"+cl);
			cs.Append("		if ((Reader->BodySize!=-1 && Reader->Body_Read>=Reader->BodySize)||(bytesReceived==0))"+cl);
			cs.Append("		{"+cl);
			cs.Append("			if (Reader->Body_BeginPointer == Reader->Body_EndPointer)"+cl);
			cs.Append("			{"+cl);
			cs.Append("				Reader->Body_BeginPointer = 0;"+cl);
			cs.Append("				Reader->Body_EndPointer = 0;"+cl);
			cs.Append("			}"+cl);
			cs.Append("			Reader->FunctionCallback(Reader, Reader->PacketHeader, Reader->Body, &Reader->Body_BeginPointer, Reader->Body_EndPointer, -1,Reader->user);"+cl);
			cs.Append("			if (Reader->PacketHeader!=NULL) {"+this.pc_methodPrefix+"DestructPacket(Reader->PacketHeader);}"+cl);
			cs.Append("			if (Reader->Body_MallocSize!=0) {free(Reader->Body);}"+cl);
			cs.Append("			Reader->Body = NULL;"+cl);
			cs.Append("			Reader->Body_MallocSize = 0;"+cl);
			cs.Append("			Reader->PacketHeader = NULL;"+cl);
			cs.Append("			"+this.pc_SockClose+"(Reader->ClientSocket);"+cl);
			cs.Append("			Reader->ClientSocket = ~0;"+cl);
			cs.Append("		}"+cl);
			cs.Append("		"+cl);
			cs.Append("		if (Reader->Body_BeginPointer==Reader->Body_EndPointer)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			Reader->Body_BeginPointer = 0;"+cl);
			cs.Append("			Reader->Body_EndPointer = 0;"+cl);
			cs.Append("		}"+cl);
			cs.Append("	}"+cl);
			cs.Append("}"+cl);

			cs.Append("int "+this.pc_methodPrefix+"GetMiniWebServerPortNumber(void *WebServerModule)"+cl);
			cs.Append("{"+cl);
			cs.Append("	struct MiniWebServerObject *module = (struct MiniWebServerObject*)WebServerModule;"+cl);
			cs.Append("	return(module->PortNumber);"+cl);
			cs.Append("}"+cl);

			
			cs.Append("void "+this.pc_methodPrefix+"MiniWebServerModule_Destroy(void* object)"+cl);
			cs.Append("{"+cl);
			cs.Append("	struct MiniWebServerObject *mws = ((struct MiniWebServerObject*)object);"+cl);
			cs.Append("	int i;"+cl);
			cs.Append("	for(i=0;i<mws->MaxConnections;++i)"+cl);
			cs.Append("	{"+cl);
			cs.Append("		if (mws->Readers[i].Body!=NULL && mws->Readers[i].Body_MallocSize!=0)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			free(mws->Readers[i].Body);"+cl);
			cs.Append("		}"+cl);
			cs.Append("		if (mws->Readers[i].PacketHeader!=NULL)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			"+this.pc_methodPrefix+"DestructPacket(mws->Readers[i].PacketHeader);"+cl);
			cs.Append("		}"+cl);
			cs.Append("	}"+cl);
			cs.Append("	free(((struct MiniWebServerObject*)object)->Readers);"+cl);
			cs.Append("}"+cl);

			string stype = this.pc_SockType;
			if (this.Platform==PLATFORMS.WINDOWS && this.WinSock==2)
			{
				stype = "HANDLE";
			}

			cs.Append("void "+this.pc_methodPrefix+"MiniWebServerModule_PreSelect(void *WebServerModule,fd_set *readset, fd_set *writeset, fd_set *errorset,int *blocktime)"+cl);
			cs.Append("{"+cl);
			cs.Append("	int i;"+cl);
			cs.Append("	struct MiniWebServerObject *module = (struct MiniWebServerObject*)WebServerModule;"+cl);
			cs.Append("	int NumFree = module->MaxConnections;"+cl);
			cs.Append(cl);
			cs.Append("	if (module->PortNumber==0)"+cl);
			cs.Append("	{"+cl);
			cs.Append("		module->PortNumber = "+this.pc_methodPrefix+"GetStreamSocket(htonl(INADDR_ANY),0,("+stype+"*)&(module->ListenSocket));"+cl);
			cs.Append("		listen(module->ListenSocket,4);"+cl);
			cs.Append("	}"+cl);
			cs.Append(cl);
			cs.Append("	/* Pre Select Connected Sockets*/"+cl);
			cs.Append("	for(i=0;i<module->MaxConnections;++i)"+cl);
			cs.Append("	{"+cl);
			cs.Append("		if (module->Readers[i].ClientSocket!=~0)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			/* Already Connected, just needs reading */"+cl);
			cs.Append("			FD_SET(module->Readers[i].ClientSocket,readset);"+cl);
			cs.Append("			FD_SET(module->Readers[i].ClientSocket,errorset);"+cl);
			cs.Append("			--NumFree;"+cl);
			cs.Append("		}"+cl);
			cs.Append("	}"+cl);
			cs.Append(cl);
			cs.Append("	if (NumFree!=0)"+cl);
			cs.Append("	{"+cl);
			cs.Append("		/* Pre Select Listen Socket */"+cl);
			cs.Append("		FD_SET(module->ListenSocket,readset);"+cl);
			cs.Append("	}"+cl);
			cs.Append("	else"+cl);
			cs.Append("	{"+cl);
			cs.Append("		if (*blocktime>1){*blocktime=1;}"+cl);
			cs.Append("	}"+cl);
			cs.Append("}"+cl);


			cs.Append("void "+this.pc_methodPrefix+"MWS_TimerSink(void *WebServerModule)"+cl);
			cs.Append("{"+cl);
			cs.Append("	struct "+this.pc_methodPrefix+"MWSHTTPReaderObject *module = (struct "+this.pc_methodPrefix+"MWSHTTPReaderObject*)WebServerModule;"+cl);
			cs.Append("	if (module->ClientSocket!=0)"+cl);
			cs.Append("	{"+cl);
			cs.Append("		"+this.pc_SockClose+"(module->ClientSocket);"+cl);
			cs.Append("		module->ClientSocket = ~0;"+cl);
			cs.Append("	}"+cl);			
			cs.Append("}"+cl);


			cs.Append("void "+this.pc_methodPrefix+"MiniWebServerModule_PostSelect(void *WebServerModule, int slct, fd_set *readset, fd_set *writeset, fd_set *errorset)"+cl);
			cs.Append("{"+cl);
			if (this.Platform==PLATFORMS.WINDOWS)
			{
				cs.Append("	unsigned long flags=0;"+cl);
			}
			cs.Append("	int i;"+cl);
			cs.Append("	struct MiniWebServerObject *module = (struct MiniWebServerObject*)WebServerModule;"+cl);
			cs.Append("	struct sockaddr_in addr;"+cl);
			cs.Append("	int addrlen = sizeof(struct sockaddr_in);"+cl);
			cs.Append("	"+cl);
			cs.Append(cl);
			cs.Append("	/* Select Connected Sockets*/"+cl);
			cs.Append("	for(i=0;i<module->MaxConnections;++i)"+cl);
			cs.Append("	{"+cl);
			cs.Append("		if (module->Readers[i].ClientSocket!=~0)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			if (FD_ISSET(module->Readers[i].ClientSocket,errorset)!=0)"+cl);
			cs.Append("			{"+cl);
			cs.Append("				module->Readers[i].ClientSocket = ~0;"+cl);
			cs.Append("				module->Readers[i].BodySize = 0;"+cl);
			cs.Append("				//ToDo: cleanup"+cl);
			cs.Append("			}"+cl);
			cs.Append("			if (FD_ISSET(module->Readers[i].ClientSocket,readset)!=0)"+cl);
			cs.Append("			{"+cl);
			cs.Append("				"+this.pc_methodPrefix+"MiniWebServerProcessSocket(&(module->Readers[i]));"+cl);
			cs.Append("			}"+cl);
			cs.Append("			if (module->Readers[i].ClientSocket==~0 || module->Readers[i].Body!=NULL)"+cl);
			cs.Append("			{"+cl);
			cs.Append("				"+this.pc_methodPrefix+"LifeTime_Remove(module->TimerObject,&(module->Readers[i]));"+cl);
			cs.Append("			}"+cl);
			cs.Append("		}"+cl);
			cs.Append("	}"+cl);
			cs.Append(cl);
			cs.Append("	/* Select Listen Socket */"+cl);
			cs.Append("	if (FD_ISSET(module->ListenSocket,readset)!=0)"+cl);
			cs.Append("	{"+cl);
			cs.Append("		for(i=0;i<module->MaxConnections;++i)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			if (module->Readers[i].ClientSocket==~0)"+cl);
			cs.Append("			{"+cl);
			cs.Append("				module->Readers[i].ClientSocket = accept(module->ListenSocket,(struct sockaddr*)&addr,&addrlen);"+cl);
			if (this.Platform==PLATFORMS.WINDOWS)
			{
				cs.Append("				ioctlsocket(module->Readers[i].ClientSocket,FIONBIO,&flags);"+cl);
			}
			cs.Append("				"+this.pc_methodPrefix+"LifeTime_Add(module->TimerObject,&(module->Readers[i]),3,&"+this.pc_methodPrefix+"MWS_TimerSink,NULL);"+cl);
			cs.Append("				module->Readers[i].HeaderIndex = 0;"+cl);
			cs.Append("				module->Readers[i].Body_BeginPointer = 0;"+cl);
			cs.Append("				module->Readers[i].Body_EndPointer = 0;"+cl);
			cs.Append("				module->Readers[i].Body_MallocSize = 0;"+cl);
			cs.Append("				module->Readers[i].Body_Read = 0;"+cl);
			cs.Append("				break;"+cl);
			cs.Append("			}"+cl);
			cs.Append("		}"+cl);
			cs.Append("	}"+cl);
			cs.Append("}"+cl);


			cs.Append("void "+this.pc_methodPrefix+"MiniWebServerCloseSession(void *ReaderModule)"+cl);
			cs.Append("{"+cl);
			cs.Append("	struct "+this.pc_methodPrefix+"MWSHTTPReaderObject *module = (struct "+this.pc_methodPrefix+"MWSHTTPReaderObject*)ReaderModule;"+cl);
			cs.Append("	"+this.pc_SockType+" TempSocket = module->ClientSocket;"+cl);
			cs.Append("	module->ClientSocket = ~0;"+cl);
			cs.Append("	module->BodySize = 0;"+cl);
			cs.Append("	"+this.pc_SockClose+"(TempSocket);"+cl);
			cs.Append("}"+cl);


			cs.Append("void "+this.pc_methodPrefix+"MiniWebServerSend(void *ReaderModule, struct packetheader *packet)"+cl);
			cs.Append("{"+cl);
			cs.Append("	struct "+this.pc_methodPrefix+"MWSHTTPReaderObject *module = (struct "+this.pc_methodPrefix+"MWSHTTPReaderObject*)ReaderModule;"+cl);
			cs.Append("	char* buffer;"+cl);
			cs.Append("	int bufferlength = "+this.pc_methodPrefix+"GetRawPacket(packet,&buffer);"+cl);
			cs.Append(cl);
			cs.Append("	send(module->ClientSocket,buffer,bufferlength,0);"+cl);
			cs.Append("	"+cl);
			cs.Append("	free(buffer);"+cl);
			cs.Append("}"+cl);



			cs.Append("void* "+this.pc_methodPrefix+"CreateMiniWebServer(void *chain,int MaxConnections,void (*OnReceivePtr) (void *ReaderObject, struct packetheader *header, char* buffer, int *BeginPointer, int BufferSize, int done, void* user),void* user)"+cl);
			cs.Append("{"+cl);
			cs.Append("	struct MiniWebServerObject *RetVal = (struct MiniWebServerObject*)malloc(sizeof(struct MiniWebServerObject));"+cl);
			cs.Append("	int i;"+cl);
			if (this.Platform==PLATFORMS.WINDOWS)
			{
				cs.Append("	WORD wVersionRequested;"+cl);
				cs.Append("	WSADATA wsaData;"+cl);
				cs.Append("	wVersionRequested = MAKEWORD( 1, 1 );"+cl);
				cs.Append("	if (WSAStartup( wVersionRequested, &wsaData ) != 0) {exit(1);}"+cl);
			}
			cs.Append(cl);
			cs.Append("	RetVal->MaxConnections = MaxConnections;"+cl);
			cs.Append("	RetVal->Readers = (struct "+this.pc_methodPrefix+"MWSHTTPReaderObject*)malloc(MaxConnections*sizeof(struct "+this.pc_methodPrefix+"MWSHTTPReaderObject));"+cl);
			cs.Append("	RetVal->Terminate = 0;"+cl);
			cs.Append("	RetVal->PreSelect = &"+this.pc_methodPrefix+"MiniWebServerModule_PreSelect;"+cl);
			cs.Append("	RetVal->PostSelect = &"+this.pc_methodPrefix+"MiniWebServerModule_PostSelect;"+cl);
			cs.Append("	RetVal->Destroy = &"+this.pc_methodPrefix+"MiniWebServerModule_Destroy;"+cl);

			cs.Append(cl);
			cs.Append("	memset(RetVal->Readers,0,MaxConnections*sizeof(struct "+this.pc_methodPrefix+"MWSHTTPReaderObject));"+cl);
			cs.Append("	for(i=0;i<MaxConnections;++i)"+cl);
			cs.Append("	{"+cl);
			cs.Append("		RetVal->Readers[i].ClientSocket = ~0;"+cl);
			cs.Append("		RetVal->Readers[i].FunctionCallback = OnReceivePtr;"+cl);
			cs.Append("		RetVal->Readers[i].Parent = RetVal;"+cl);
			cs.Append("		RetVal->Readers[i].user = user;"+cl);
			cs.Append("	}"+cl);
			cs.Append("	"+cl);
			cs.Append("	RetVal->PortNumber = 0;"+cl);
//			cs.Append("	RetVal->PortNumber = "+this.pc_methodPrefix+"GetStreamSocket(htonl(INADDR_ANY),&(RetVal->ListenSocket));"+cl);
//			cs.Append("	listen(RetVal->ListenSocket,4);"+cl);
			cs.Append(cl);
			cs.Append("	RetVal->TimerObject = "+this.pc_methodPrefix+"CreateLifeTime(chain);"+cl);
			cs.Append("	"+this.pc_methodPrefix+"AddToChain(chain,RetVal);"+cl);
			cs.Append("	return((void*)RetVal);"+cl);
			cs.Append("}"+cl);



			writer = File.CreateText(outputDirectory.FullName + "\\"+pc_methodPrefix+"MiniWebServer.c");
			writer.Write(cs.ToString());
			writer.Close();
			return(true);
		}

		public bool Build_UPnPHTTPClient(DirectoryInfo outputDirectory)
		{
			StreamWriter writer;

			if (this.Platform == PLATFORMS.POSIX) 
			{
				pc_SockType = "int";
//				pc_TimeType = "struct timeval";
				pc_SockClose = "close";
//				pc_stricmp = "strncasecmp";
			}

			if (this.Platform == PLATFORMS.WINDOWS) 
			{
				pc_SockType = "SOCKET";
//				pc_TimeType = "unsigned int";
				pc_SockClose = "closesocket";
//				pc_stricmp = "_strnicmp";
			}

			if (this.Language == LANGUAGES.C)
			{
				pc_methodPrefix = CallPrefix;
				pc_methodLibPrefix = CallLibPrefix;
				pc_methodPrefixDef = CallingConvention + CallPrefix;
			}


			CodeProcessor cs = new CodeProcessor(new StringBuilder(),this.Language == LANGUAGES.CPP);

			/* Build UPnPHTTPClient.h */
			AddLicense(cs,pc_methodPrefix+"HTTPClient.h");

			cs.Append(cl);
			cs.Append("#ifndef __"+pc_methodPrefix+"HTTPClient__"+cl);
			cs.Append("#define __"+pc_methodPrefix+"HTTPClient__"+cl);
			cs.Append(cl);
			cs.Append("#define HTTP_SESSION_INTERRUPT_CHAIN 1"+cl);
			cs.Append("#define HTTP_SESSION_INTERRUPT_PEERRESET 2"+cl);
			cs.Append("#define HTTP_INTERRUPT_CHAIN 3"+cl);
			cs.Append("#define HTTP_DELETEREQUEST_INTERRUPT 4"+cl);

			cs.Append("#define "+pc_methodPrefix+"AddRequest_Direct(ClientModule, buffer, bufferlength,Destination, CallbackPtr, user, user2) "+pc_methodPrefix+"AddRequest_DirectEx(ClientModule, buffer, bufferlength,NULL,0,Destination, CallbackPtr, user, user2)"+cl);
			cs.Append(cl);
			cs.Comment("Forward Declaration");
			cs.Append("struct packetheader;"+cl+cl);

			cs.Append("void* "+pc_methodPrefix+"CreateHTTPClientModule(void *Chain, int MaxSockets);"+cl);
			cs.Append("void  "+pc_methodPrefix+"DestroyHTTPClientModule(void *ClientModule);"+cl);
			cs.Append(cl);
			cs.Append("char* "+pc_methodPrefix+"GetReceivingInterface(void* ReaderObject);"+cl);
			cs.Append("void  "+pc_methodPrefix+"AddRequest(void *ClientModule, struct packetheader *packet,struct sockaddr_in *Destination, void (*CallbackPtr)(void *reader, struct packetheader *header, int IsInterrupt,char* buffer, int *p_BeginPointer, int EndPointer, int done, void* user, void* user2), void* user, void* user2);"+cl);
			cs.Append("void  "+pc_methodPrefix+"AddRequest_DirectEx(void *ClientModule, char *buffer, int bufferlength,char *buffer2, int buffer2length,struct sockaddr_in *Destination, void (*CallbackPtr)(void *reader, struct packetheader *header, int IsInterrupt, char* buffer, int *p_BeginPointer, int EndPointer, int done, void* user, void* user2), void* user, void* user2);"+cl);
			cs.Append("void  "+pc_methodPrefix+"CloseRequest(void* ReaderObject);"+cl);
			cs.Append("void  "+pc_methodPrefix+"DeleteRequests(void *ClientModule, void *user1);"+cl);

			cs.Append(cl);
			cs.Append("#endif"+cl);

			writer = File.CreateText(outputDirectory.FullName + "\\"+pc_methodPrefix+"HTTPClient.h");
			writer.Write(cs.ToString());
			writer.Close();

			/* Build UPnPHTTPClient.c */
			cs = new CodeProcessor(new StringBuilder(),this.Language == LANGUAGES.CPP);
			if (this.Language == LANGUAGES.CPP) 
			{
				AddLicense(cs,pc_methodPrefix+"HTTPClient.cpp");
			} 
			else 
			{
				AddLicense(cs,pc_methodPrefix+"HTTPClient.c");
			}
			cs.Append(cl);

			if (this.Platform==PLATFORMS.WINDOWS)
			{
				cs.Append("#ifndef MICROSTACK_NO_STDAFX"+cl);
				cs.Append("#include \"stdafx.h\""+cl);
				cs.Append("#endif"+cl);
				cs.Append("#define _CRTDBG_MAP_ALLOC"+cl);
				cs.Append("#include <math.h>"+cl);
				cs.Append("#include <winerror.h>"+cl);					
				cs.Append("#include <stdlib.h>"+cl);
				cs.Append("#include <stdio.h>"+cl);
				cs.Append("#include <stddef.h>"+cl);
				cs.Append("#include <string.h>"+cl);
				if (this.WinSock == 1) 
				{
					cs.Append("#include <winsock.h>"+cl);
					cs.Append("#include <wininet.h>"+cl);
				}
				if (this.WinSock == 2) 
				{
					cs.Append("#include <winsock2.h>"+cl);
					cs.Append("#include <ws2tcpip.h>"+cl);
				}
				if (this.SubTarget != SUBTARGETS.PPC2003) 
				{
//					cs.Append("#include <errno.h>"+cl);
				}
				cs.Append("#include <windows.h>"+cl);
				cs.Append("#include <winioctl.h>"+cl);
				cs.Append("#include <winbase.h>"+cl);
				
				cs.Append("#include <crtdbg.h>"+cl);
			}
			else
			{
				if (this.SubTarget == SUBTARGETS.NUCLEUS) 
				{
					cs.Append("#include <stdio.h>"+cl);
					cs.Append("#include <stdlib.h>"+cl);
					cs.Append("#include \"net/inc/externs.h\""+cl);
					cs.Append("#include \"net/inc/ip.h\""+cl);
					cs.Append("#include \"net/inc/socketd.h\""+cl);
					cs.Append("#include <errno.h>"+cl);
				}
				else 
				{
					cs.Append("#include <stdio.h>"+cl);
					cs.Append("#include <stdlib.h>"+cl);
					cs.Append("#include <sys/types.h>"+cl);
					cs.Append("#include <sys/socket.h>"+cl);
					cs.Append("#include <netinet/in.h>"+cl);
					cs.Append("#include <arpa/inet.h>"+cl);
					cs.Append("#include <sys/time.h>"+cl);
					cs.Append("#include <netdb.h>"+cl);
					cs.Append("#include <string.h>"+cl);
					cs.Append("#include <sys/ioctl.h>"+cl);
					cs.Append("#include <net/if.h>"+cl);
					cs.Append("#include <sys/utsname.h>"+cl);
					cs.Append("#include <sys/socket.h>"+cl);
					cs.Append("#include <netinet/in.h>"+cl);
					cs.Append("#include <unistd.h>"+cl);
					cs.Append("#include <fcntl.h>"+cl);
					//cs.Append("#include <errno.h>"+cl);
					cs.Append("#include <semaphore.h>"+cl);
					cs.Append("#include <malloc.h>"+cl);
				}
			}
			cs.Append(cl);
			cs.Append("#include \""+pc_methodPrefix+"HTTPClient.h\""+cl);
			cs.Append("#include \""+pc_methodPrefix+"Parsers.h\""+cl);

			if (this.Platform == PLATFORMS.WINDOWS)
			{
				cs.Append("#define strncasecmp(x,y,z) _strnicmp(x,y,z)"+cl);
				cs.Append("#define gettimeofday(x,y) (x)->tv_sec = GetTickCount()/1000"+cl);
			}

			if (this.Platform != PLATFORMS.POSIX)
			{
				cs.Append("#define sem_t HANDLE"+cl);
				cs.Append("#define sem_init(x,y,z) *x=CreateSemaphore(NULL,z,FD_SETSIZE,NULL)"+cl);
				cs.Append("#define sem_destroy(x) (CloseHandle(*x)==0?1:0)"+cl);
				cs.Append("#define sem_wait(x) WaitForSingleObject(*x,INFINITE)"+cl);
				cs.Append("#define sem_trywait(x) ((WaitForSingleObject(*x,0)==WAIT_OBJECT_0)?0:1)"+cl);
				cs.Append("#define sem_post(x) ReleaseSemaphore(*x,1,NULL)"+cl);
			}

			cs.Append("#define DEBUGSTATEMENT(x)"+cl);
			cs.Append("#define LVL3DEBUG(x)"+cl);
			cs.Append(cl);

			cs.Append("struct RequestQueueNode"+cl);
			cs.Append("{"+cl);
			cs.Append("	char* Request;"+cl);
			cs.Append("	char* Request2;"+cl);
			cs.Append("	int RequestLength;"+cl);
			cs.Append("	int Request2Length;"+cl);
			cs.Append("	struct sockaddr_in Destination;"+cl);
			cs.Append("	void* user;"+cl);
			cs.Append("	void* user2;"+cl);
			cs.Append("	void (*FunctionCallback) (void *sender, struct packetheader *header, int IsInterrupt, char* buffer, int *p_BeginPointer, int EndPointer, int done, void* user, void* user2);"+cl);
			cs.Append("	struct RequestQueueNode* Next;"+cl);
			cs.Append("	struct RequestQueueNode* Previous;"+cl);
			cs.Append("};"+cl);
			cs.Append(cl);

			cs.Append("struct "+this.pc_methodPrefix+"HCHTTPReaderObject"+cl);
			cs.Append("{"+cl);
			cs.Append("	struct packetheader* PacketHeader;"+cl);
			cs.Append("	char Header[2048];"+cl);
			cs.Append("	char* Body;"+cl);
			cs.Append("	int BodySize;"+cl);
			cs.Append("	int HeaderIndex;"+cl);
			cs.Append("	int LocalIPAddress;"+cl);
			cs.Append(cl);
			cs.Append("	int Body_BeginPointer;"+cl);
			cs.Append("	int Body_EndPointer;"+cl);
			cs.Append("	int Body_MallocSize;"+cl);
			cs.Append("	int Body_Read;"+cl);
			cs.Append(cl);
			cs.Append("	"+pc_SockType+" ClientSocket;"+cl);
			cs.Append("	int FinRead;"+cl);
			cs.Append("	int FinConnect;"+cl);
			cs.Append("	struct HTTPClientModule *Parent;"+cl);
			cs.Append("	void* user;"+cl);
			cs.Append("	void* user2;"+cl);
			cs.Append("	void (*FunctionCallback) (struct "+this.pc_methodPrefix+"HCHTTPReaderObject *ReaderObject, struct packetheader *header, int IsInterrupt, char* buffer, int *BeginPointer, int BufferSize, int done, void* user, void* user2);"+cl);
			cs.Append("	char* send_data;"+cl);
			cs.Append("	char* send_data2;"+cl);
			cs.Append("	int send_dataLength;"+cl);
			cs.Append("	int send_data2Length;"+cl);
			cs.Append("};"+cl);
			cs.Append(cl);

			cs.Append("struct HTTPClientModule"+cl);
			cs.Append("{"+cl);
			cs.Append("void (*PreSelect)(void* object,fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime);"+cl);
			cs.Append("void (*PostSelect)(void* object,int slct, fd_set *readset, fd_set *writeset, fd_set *errorset);"+cl);
			cs.Append("void (*Destroy)(void* object);"+cl);
			cs.Append("void *Chain;"+cl);

			cs.Append("	sem_t QueueLock;"+cl);
			cs.Append("	sem_t Monitor;"+cl);
			cs.Append("	int RequestQueueCount;"+cl);
			cs.Append("	struct RequestQueueNode* First;"+cl);
			cs.Append("	int NumSlots;"+cl);
			cs.Append("	struct "+this.pc_methodPrefix+"HCHTTPReaderObject *Readers;"+cl);
			cs.Append("	int Terminate;"+cl);
			cs.Append("	void *Reserved;"+cl);
			cs.Append(cl);
			cs.Append("	void *SocketTimer;"+cl);
			cs.Append(cl);
			cs.Append("	LVL3DEBUG(int ADD_COUNTER;)"+cl);
			cs.Append("	LVL3DEBUG(int FAILED_COUNTER;)"+cl);
			cs.Append("	LVL3DEBUG(int GRACEFUL_COUNTER;)"+cl);
			cs.Append("	LVL3DEBUG(int FORCE_COUNTER;)"+cl);
			cs.Append("	LVL3DEBUG(int START_COUNTER;)"+cl);
			cs.Append("	LVL3DEBUG(int CLOSE_COUNTER;)"+cl);
			cs.Append("	LVL3DEBUG(int SEND_FAIL;)"+cl);
			cs.Append("	LVL3DEBUG(int SEND_FAIL2;)"+cl);
			cs.Append("	LVL3DEBUG(int CONNECT_COUNTER;)"+cl);
			cs.Append(cl);
			cs.Append("};"+cl);
			cs.Append(cl);

//			cs.Append("extern int errno;"+cl);
			cs.Append(cl);


			string stype = this.pc_SockType;
			if (this.Platform==PLATFORMS.WINDOWS && this.WinSock==2)
			{
				stype = "HANDLE";
			}

			cs.Append("char* "+pc_methodPrefixDef+"GetReceivingInterface(void* ReaderObject)"+cl);
			cs.Append("{"+cl);
			cs.Append("	char* RetVal = (char*)malloc(16);"+cl);
			cs.Append("	int addr = ((struct "+this.pc_methodPrefix+"HCHTTPReaderObject*)ReaderObject)->LocalIPAddress;"+cl);
			cs.Append("	sprintf(RetVal,\"%d.%d.%d.%d\",(addr&0xFF),((addr>>8)&0xFF),((addr>>16)&0xFF),((addr>>24)&0xFF));"+cl);
			cs.Append("	return(RetVal);"+cl);
			cs.Append("}"+cl);
			cs.Append("void "+pc_methodPrefixDef+"DestroyHTTPClientModule(void *ClientModule)"+cl);
			cs.Append("{"+cl);
			cs.Append("	struct HTTPClientModule* module = (struct HTTPClientModule*)ClientModule;"+cl);
			cs.Append("	int i;"+cl);
			cs.Append("	struct RequestQueueNode *rqn,*rqn2;"+cl);
			cs.Append(cl);
			cs.Append("	LVL3DEBUG(printf(\"\\r\\n\\r\\nAdd:%d Failed:%d GC:%d Forced:%d Started:%d UserClose:%d SF:%d SF2:%d\",module->ADD_COUNTER,module->FAILED_COUNTER,module->GRACEFUL_COUNTER,module->FORCE_COUNTER,module->START_COUNTER,module->CLOSE_COUNTER,module->SEND_FAIL,module->SEND_FAIL2);)"+cl);
			cs.Append("	LVL3DEBUG(printf(\"\\r\\nConnected: %d \\r\\n\",module->CONNECT_COUNTER);)"+cl);
			cs.Append("	for(i=0;i<module->NumSlots;++i)"+cl);
			cs.Append("	{"+cl);
			cs.Append("		LVL3DEBUG(printf(\"Slot: %d	Socket: %d\\r\\n\",i,module->Readers[i].ClientSocket);)"+cl);
			cs.Append(cl);
			cs.Append("		if (module->Readers[i].Body_MallocSize!=0)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			free(module->Readers[i].Body);"+cl);
			cs.Append("		}"+cl);
			cs.Append("		if (module->Readers[i].send_data!=NULL)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			free(module->Readers[i].send_data);"+cl);
			cs.Append("		}"+cl);
			cs.Append("		if (module->Readers[i].send_data2!=NULL)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			free(module->Readers[i].send_data2);"+cl);
			cs.Append("		}"+cl);
			cs.Append("		if (module->Readers[i].ClientSocket!=~0)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			"+this.pc_SockClose+"(module->Readers[i].ClientSocket);"+cl);
			cs.Append("			if (module->Readers[i].FunctionCallback!=NULL && (module->Readers[i].FinConnect==0 || (module->Readers[i].FinConnect==1 && module->Readers[i].FinRead==0)))"+cl);
			cs.Append("			{"+cl);
			cs.Append("				module->Readers[i].FunctionCallback(&(module->Readers[i]),NULL,HTTP_SESSION_INTERRUPT_CHAIN,NULL,NULL,0,-1,module->Readers[i].user,module->Readers[i].user2);"+cl);
			cs.Append("			}"+cl);
			cs.Append("		}"+cl);
			cs.Append("		if (module->Readers[i].PacketHeader!=NULL)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			"+this.pc_methodPrefix+"DestructPacket(module->Readers[i].PacketHeader);"+cl);
			cs.Append("		}"+cl);
			cs.Append("	}"+cl);
			cs.Append(cl);
			cs.Append("	rqn = module->First;"+cl);
			cs.Append("	while(rqn!=NULL)"+cl);
			cs.Append("	{"+cl);
			cs.Append("		if (rqn->Request!=NULL) {free(rqn->Request);}"+cl);
			cs.Append("		if (rqn->Request2!=NULL) {free(rqn->Request2);}"+cl);
			cs.Append("		if (rqn->FunctionCallback!=NULL) {rqn->FunctionCallback(module,NULL,HTTP_INTERRUPT_CHAIN,NULL,NULL,0,-1,rqn->user,rqn->user2);}"+cl);
			cs.Append("		rqn2 = rqn->Next;"+cl);
			cs.Append("		free(rqn);"+cl);
			cs.Append("		rqn = rqn2;"+cl);
			cs.Append("	}"+cl);
			cs.Append(cl);
			cs.Append("	sem_destroy(&(module->Monitor));"+cl);
			cs.Append("	sem_destroy(&(module->QueueLock));"+cl);
			cs.Append("	free(module->Readers);"+cl);
			cs.Append("}"+cl);
			cs.Append(cl);


			cs.Append("void "+pc_methodPrefixDef+"StartRequest(void *ClientModule, struct RequestQueueNode *Request)"+cl);
			cs.Append("{"+cl);
			cs.Append("	int i=0;"+cl);
			if (this.Platform == PLATFORMS.WINDOWS) 
			{
				cs.Append("	unsigned long flags;"+cl);
			} 
			else 
			{
				cs.Append("	unsigned int flags;"+cl);
			}
			cs.Append("	struct HTTPClientModule *module = (struct HTTPClientModule*)ClientModule;"+cl);
			cs.Append(cl);
			cs.Append("	for(i=0;i<module->NumSlots;++i)"+cl);
			cs.Append("	{"+cl);
			cs.Append("		if (module->Readers[i].ClientSocket==~0)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			LVL3DEBUG(++module->START_COUNTER;)"+cl);
			cs.Append("			if (module->Readers[i].PacketHeader!=NULL)"+cl);
			cs.Append("			{"+cl);
			cs.Append("				"+pc_methodPrefixDef+"DestructPacket(module->Readers[i].PacketHeader);"+cl);
			cs.Append("				module->Readers[i].PacketHeader = NULL;"+cl);
			cs.Append("			}"+cl);
			cs.Append("			module->Readers[i].FunctionCallback = (void*)Request->FunctionCallback;"+cl);
			cs.Append("			module->Readers[i].send_data = Request->Request;"+cl);
			cs.Append("			module->Readers[i].send_dataLength = Request->RequestLength;"+cl);
			cs.Append("			module->Readers[i].send_data2 = Request->Request2;"+cl);
			cs.Append("			module->Readers[i].send_data2Length = Request->Request2Length;"+cl);
			cs.Append("			module->Readers[i].FinRead = 0;"+cl);
			cs.Append("			module->Readers[i].FinConnect = 0;"+cl);
			cs.Append("			module->Readers[i].Body = NULL;"+cl);
			cs.Append("			module->Readers[i].BodySize = 0;"+cl);
			cs.Append("			module->Readers[i].HeaderIndex = 0;"+cl);
			cs.Append("			module->Readers[i].Body_Read = 0;"+cl);
			cs.Append("			module->Readers[i].Body_BeginPointer = 0;"+cl);
			cs.Append("			module->Readers[i].Body_EndPointer = 0;"+cl);
			cs.Append("			module->Readers[i].user = Request->user;"+cl);
			cs.Append("			module->Readers[i].user2 = Request->user2;"+cl);
			cs.Append("			"+pc_methodPrefix+"GetStreamSocket(htonl(INADDR_ANY),0,("+stype+"*)&(module->Readers[i].ClientSocket));"+cl);
			cs.Append(cl);

			if (this.Platform == PLATFORMS.POSIX)
			{
				cs.Comment("Platform Dependent [POSIX]");
				cs.Append("			flags = fcntl(module->Readers[i].ClientSocket,F_GETFL,0);"+cl);
				cs.Append("			fcntl(module->Readers[i].ClientSocket,F_SETFL,O_NONBLOCK|flags);"+cl);
			}
			else
			{
				cs.Comment("Platform Dependent [Windows]");
				cs.Append("			flags = 1;"+cl);
				cs.Append("			ioctlsocket(module->Readers[i].ClientSocket,FIONBIO,&flags);"+cl);
			}
			cs.Append(cl);
			cs.Append("			connect(module->Readers[i].ClientSocket,(struct sockaddr*)&(Request->Destination),sizeof(Request->Destination));	"+cl);
			cs.Append("			break;"+cl);
			cs.Append("		}"+cl);
			cs.Append("	}"+cl);
			cs.Append("}"+cl);
			cs.Append(cl);

			cs.Append("void "+pc_methodPrefixDef+"ForceClose(void *Reader)"+cl);
			cs.Append("{"+cl);
			cs.Append("	struct "+this.pc_methodPrefix+"HCHTTPReaderObject *r = (struct "+this.pc_methodPrefix+"HCHTTPReaderObject*)Reader;"+cl);
			cs.Append(""+cl);
			cs.Append("	if (r->ClientSocket!=~0)"+cl);
			cs.Append("	{"+cl);
			cs.Append("		LVL3DEBUG(++r->Parent->FORCE_COUNTER;)"+cl);
			cs.Append("		"+this.pc_SockClose+"(r->ClientSocket);"+cl);
			cs.Append("		r->ClientSocket = ~0;"+cl);
			cs.Append("	}"+cl);
			cs.Append("}"+cl);
			

			cs.Append("void "+pc_methodPrefixDef+"ProcessSocket(struct "+this.pc_methodPrefix+"HCHTTPReaderObject *Reader)"+cl);
			cs.Append("{"+cl);
			cs.Append("	int bytesReceived;"+cl);
			cs.Append("	int i;"+cl);
			cs.Append("	struct packetheader_field_node *node;"+cl);
			cs.Append("	char* CharStar;"+cl);
			cs.Append(cl);
			cs.Append("	if (Reader->BodySize==0)"+cl);
			cs.Append("	{"+cl);
			cs.Comment("Still Reading Headers");
			cs.Append("		bytesReceived = recv(Reader->ClientSocket,Reader->Header+Reader->HeaderIndex,2048-Reader->HeaderIndex,0);"+cl);
			cs.Append("		if (bytesReceived==0)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			LVL3DEBUG(++Reader->Parent->GRACEFUL_COUNTER;)"+cl);
			cs.Append("			if (Reader->PacketHeader!=NULL) {"+this.pc_methodPrefix+"DestructPacket(Reader->PacketHeader);}"+cl);
			cs.Append("			if (Reader->Body_MallocSize!=0) {free(Reader->Body);}"+cl);
			cs.Append("			Reader->Body = NULL;"+cl);
			cs.Append("			Reader->Body_MallocSize = 0;"+cl);
			cs.Append("			Reader->PacketHeader = NULL;"+cl);
			cs.Append("			"+this.pc_SockClose+"(Reader->ClientSocket);"+cl);
			cs.Append("			Reader->ClientSocket = ~0;"+cl);
			cs.Append("			"+this.pc_methodPrefix+"LifeTime_Remove(Reader->Parent->SocketTimer,Reader);"+cl);
			cs.Append("			if (Reader->FinRead==0)"+cl);
			cs.Append("			{"+cl);
			cs.Append("				// Prematurely Closed: Error"+cl);
			cs.Append("				Reader->FunctionCallback(Reader,NULL,HTTP_SESSION_INTERRUPT_PEERRESET,NULL,NULL,0,-1,Reader->user,Reader->user2);"+cl);
			cs.Append("			}"+cl);
			cs.Append("			return;"+cl);
			cs.Append("		}"+cl);
			cs.Append("		else"+cl);
			cs.Append("		{"+cl);
			cs.Append("			if (Reader->FinRead!=0)"+cl);
			cs.Append("			{"+cl);
			cs.Append("				return;"+cl);
			cs.Append("			}"+cl);
			cs.Append("		}"+cl);
			cs.Append("		Reader->HeaderIndex += bytesReceived;"+cl);
			cs.Append("		if (Reader->HeaderIndex>4)"+cl);
			cs.Append("		{"+cl);
			cs.Comment("Must have read at least 4 bytes to perform check");
			cs.Append("			for(i=0;i<(Reader->HeaderIndex - 3);i++)"+cl);
			cs.Append("			{"+cl);
			cs.Append("				if (Reader->Header[i] == '\\r' && Reader->Header[i+1] == '\\n' && Reader->Header[i+2] == '\\r' && Reader->Header[i+3] == '\\n')"+cl);
			cs.Append("				{"+cl);
			cs.Comment("Finished Header");
			cs.Append("					Reader->PacketHeader = "+this.pc_methodPrefix+"ParsePacketHeader(Reader->Header,0,i+4);"+cl);
			cs.Append("					if (Reader->PacketHeader==NULL)"+cl);
			cs.Append("					{"+cl);
			cs.Append("						//Invalid Packet"+cl);
			cs.Append("						Reader->FunctionCallback(Reader,Reader->PacketHeader,0,NULL,NULL,0,-1,Reader->user,Reader->user2);"+cl);
			cs.Append("						Reader->BodySize = 0;"+cl);
			cs.Append("						if (Reader->Body!=NULL)"+cl);
			cs.Append("						{"+cl);
			cs.Append("							free(Reader->Body);"+cl);
			cs.Append("							Reader->Body = NULL;"+cl);
			cs.Append("							Reader->Body_MallocSize = 0;"+cl);
			cs.Append("						}"+cl);
			cs.Append("						Reader->FinRead=1;"+cl);
			cs.Append("						"+this.pc_methodPrefix+"LifeTime_Add(Reader->Parent->SocketTimer,Reader,1,&"+this.pc_methodPrefix+"ForceClose,NULL);"+cl);
			cs.Append("						break;"+cl);
			cs.Append("					}"+cl);
			cs.Append("					Reader->PacketHeader->ReceivingAddress = Reader->LocalIPAddress;"+cl);
			cs.Append("					Reader->BodySize = -1;"+cl);
			cs.Append("					Reader->Body_Read = 0;"+cl);
			cs.Append("					node = Reader->PacketHeader->FirstField;"+cl);
			cs.Append("					while(node!=NULL)"+cl);
			cs.Append("					{"+cl);
			cs.Append("						if (strncasecmp(node->Field,\"CONTENT-LENGTH\",14)==0)"+cl);
			cs.Append("						{"+cl);
			cs.Append("							CharStar = (char*)malloc(1+node->FieldDataLength);"+cl);
			cs.Append("							memcpy(CharStar,node->FieldData,node->FieldDataLength);"+cl);
			cs.Append("							CharStar[node->FieldDataLength] = '\\0';"+cl);
			cs.Append("							Reader->BodySize = atoi(CharStar);"+cl);
			cs.Append("							free(CharStar);"+cl);
			cs.Append("							break;"+cl);
			cs.Append("						}"+cl);
			cs.Append("						node = node->NextField;"+cl);
			cs.Append("					}"+cl);
			cs.Append("					if (Reader->BodySize!=-1)"+cl);
			cs.Append("					{"+cl);
			cs.Append("						if (Reader->BodySize!=0)"+cl);
			cs.Append("						{"+cl);
			cs.Append("							Reader->Body = (char*)malloc(Reader->BodySize);"+cl);
			cs.Append("							Reader->Body_MallocSize = Reader->BodySize;"+cl);
			cs.Append("						}"+cl);
			cs.Append("						else"+cl);
			cs.Append("						{"+cl);
			cs.Append("							Reader->Body = NULL;"+cl);
			cs.Append("							Reader->Body_MallocSize = 0;"+cl);
			cs.Append("						}"+cl);
			cs.Append("					}"+cl);
			cs.Append("					else"+cl);
			cs.Append("					{"+cl);
			cs.Append("						Reader->Body = (char*)malloc(4096);"+cl);
			cs.Append("						Reader->Body_MallocSize = 4096;"+cl);
			cs.Append("					}"+cl);
			cs.Append(cl);
			cs.Append("					if (Reader->HeaderIndex>i+4 && Reader->BodySize!=0)"+cl);
			cs.Append("					{"+cl);
			cs.Comment("Part of the body is in here");
			cs.Append("						memcpy(Reader->Body,Reader->Header+i+4,Reader->HeaderIndex-(&Reader->Header[i+4]-Reader->Header));"+cl);
			cs.Append("						Reader->Body_BeginPointer = 0;"+cl);
			cs.Append("						Reader->Body_EndPointer = Reader->HeaderIndex-(int)(&Reader->Header[i+4]-Reader->Header);"+cl);
			cs.Append("						Reader->Body_Read = Reader->Body_EndPointer;"+cl);
			cs.Append(cl);
			cs.Append("						if (Reader->BodySize!=-1 && Reader->Body_Read>=Reader->BodySize)"+cl);
			cs.Append("						{"+cl);
			cs.Append("							DEBUGSTATEMENT(printf(\"Close\\r\\n\"));"+cl);
			cs.Append("							Reader->FunctionCallback(Reader,Reader->PacketHeader,0,Reader->Body,&Reader->Body_BeginPointer,Reader->Body_EndPointer - Reader->Body_BeginPointer,-1,Reader->user,Reader->user2);"+cl);
			cs.Append("							"+cl);
			cs.Append("							while(Reader->Body_BeginPointer!=Reader->Body_EndPointer && Reader->Body_BeginPointer!=0)"+cl);
			cs.Append("							{"+cl);
			cs.Append("								memcpy(Reader->Body,Reader->Body+Reader->Body_BeginPointer,Reader->Body_EndPointer-Reader->Body_BeginPointer);"+cl);
			cs.Append("								Reader->Body_EndPointer = Reader->Body_EndPointer-Reader->Body_BeginPointer;"+cl);
			cs.Append("								Reader->Body_BeginPointer = 0;"+cl);
			cs.Append("								Reader->FunctionCallback(Reader,Reader->PacketHeader,0,Reader->Body,&Reader->Body_BeginPointer,Reader->Body_EndPointer,-1,Reader->user,Reader->user2);"+cl);
			cs.Append("							}"+cl);
			cs.Append(cl);
//			cs.Append("							"+pc_SockClose+"(Reader->ClientSocket);"+cl);
//			cs.Append("							Reader->ClientSocket = ~0;"+cl);
			cs.Append("							Reader->BodySize = 0;"+cl);
			cs.Append("							if (Reader->Body!=NULL)"+cl);
			cs.Append("							{"+cl);
			cs.Append("								free(Reader->Body);"+cl);
			cs.Append("								Reader->Body = NULL;"+cl);
			cs.Append("								Reader->Body_MallocSize = 0;"+cl);
			cs.Append("							}"+cl);
			cs.Append("							Reader->FinRead=1;"+cl);
			cs.Append("							"+this.pc_methodPrefix+"LifeTime_Add(Reader->Parent->SocketTimer,Reader,1,&"+this.pc_methodPrefix+"ForceClose,NULL);"+cl);
			cs.Append("						}"+cl);
			cs.Append("						else"+cl);
			cs.Append("						{"+cl);
			cs.Append("							Reader->FunctionCallback(Reader,Reader->PacketHeader,0,Reader->Body,&Reader->Body_BeginPointer,Reader->Body_EndPointer - Reader->Body_BeginPointer,0,Reader->user,Reader->user2);"+cl);
			cs.Append("							while(Reader->Body_BeginPointer!=Reader->Body_EndPointer && Reader->Body_BeginPointer!=0)"+cl);
			cs.Append("							{"+cl);
			cs.Append("								memcpy(Reader->Body,Reader->Body+Reader->Body_BeginPointer,Reader->Body_EndPointer-Reader->Body_BeginPointer);"+cl);
			cs.Append("								Reader->Body_EndPointer = Reader->Body_EndPointer-Reader->Body_BeginPointer;"+cl);
			cs.Append("								Reader->Body_BeginPointer = 0;"+cl);
			cs.Append("								Reader->FunctionCallback(Reader,Reader->PacketHeader,0,Reader->Body,&Reader->Body_BeginPointer,Reader->Body_EndPointer,0,Reader->user,Reader->user2);"+cl);
			cs.Append("							}"+cl);
			cs.Append("						}"+cl);
			cs.Append("					}"+cl);
			cs.Append("					else"+cl);
			cs.Append("					{"+cl);
			cs.Comment("There is no body, but the packet is here");
			cs.Append("						Reader->Body_BeginPointer = 0;"+cl);
			cs.Append("						Reader->Body_EndPointer = 0;"+cl);
			cs.Append(cl);
			cs.Append("						if (Reader->BodySize==0)"+cl);
			cs.Append("						{"+cl);
			cs.Append("							Reader->FunctionCallback(Reader,Reader->PacketHeader,0,NULL,&Reader->Body_BeginPointer,0,-1,Reader->user,Reader->user2);"+cl);
//			cs.Append("							"+pc_SockClose+"(Reader->ClientSocket);"+cl);
//			cs.Append("							Reader->ClientSocket = ~0;"+cl);
			cs.Append("							Reader->BodySize = 0;"+cl);
			cs.Append("							Reader->FinRead=1;"+cl);
			cs.Append("							"+this.pc_methodPrefix+"LifeTime_Add(Reader->Parent->SocketTimer,Reader,1,&"+this.pc_methodPrefix+"ForceClose,NULL);"+cl);
			cs.Append("						}"+cl);
			cs.Append("						else"+cl);
			cs.Append("						{"+cl);
			cs.Append("							Reader->FunctionCallback(Reader,Reader->PacketHeader,0,NULL,&Reader->Body_BeginPointer,0,0,Reader->user,Reader->user2);"+cl);
			cs.Append("						}"+cl);								
			cs.Append("					}"+cl);
			cs.Append("					break;"+cl);
			cs.Append("				}"+cl);
			cs.Append("			}"+cl);
			cs.Append("		}"+cl);
			cs.Append("	}"+cl);
			cs.Append("	else"+cl);
			cs.Append("	{"+cl);
			cs.Comment("Reading Body Only");
			cs.Append("		if (Reader->Body_BeginPointer == Reader->Body_EndPointer)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			Reader->Body_BeginPointer = 0;"+cl);
			cs.Append("			Reader->Body_EndPointer = 0;"+cl);
			cs.Append("		}"+cl);
			cs.Append("		else"+cl);
			cs.Append("		{"+cl);
			cs.Append("			if (Reader->Body_BeginPointer!=0)"+cl);
			cs.Append("			{"+cl);
			cs.Append("				Reader->Body_EndPointer = Reader->Body_BeginPointer;"+cl);
			cs.Append("			}"+cl);
			cs.Append("		}"+cl);
			cs.Append(cl);
			cs.Append(cl);
			cs.Append("		if (Reader->Body_EndPointer == Reader->Body_MallocSize)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			Reader->Body_MallocSize += 4096;"+cl);
			cs.Append("			Reader->Body = (char*)realloc(Reader->Body,Reader->Body_MallocSize);"+cl);
			cs.Append("		}"+cl);
			cs.Append(cl);
			cs.Append("		bytesReceived = recv(Reader->ClientSocket,Reader->Body+Reader->Body_EndPointer,Reader->Body_MallocSize-Reader->Body_EndPointer,0);"+cl);
			cs.Append("		if (bytesReceived==0)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			LVL3DEBUG(++Reader->Parent->GRACEFUL_COUNTER;)"+cl);
			cs.Append("		}"+cl);
			cs.Append("		Reader->Body_EndPointer += bytesReceived;"+cl);
			cs.Append("		Reader->Body_Read += bytesReceived;"+cl);
			cs.Append("		"+cl);
			cs.Append("		Reader->FunctionCallback(Reader, Reader->PacketHeader, 0,Reader->Body+Reader->Body_BeginPointer, &Reader->Body_BeginPointer, Reader->Body_EndPointer - Reader->Body_BeginPointer, 0, Reader->user, Reader->user2);"+cl);
			cs.Append("		while(Reader->Body_BeginPointer!=Reader->Body_EndPointer && Reader->Body_BeginPointer!=0)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			memcpy(Reader->Body,Reader->Body+Reader->Body_BeginPointer,Reader->Body_EndPointer-Reader->Body_BeginPointer);"+cl);
			cs.Append("			Reader->Body_EndPointer = Reader->Body_EndPointer-Reader->Body_BeginPointer;"+cl);
			cs.Append("			Reader->Body_BeginPointer = 0;"+cl);
			cs.Append("			Reader->FunctionCallback(Reader,Reader->PacketHeader,0,Reader->Body,&Reader->Body_BeginPointer,Reader->Body_EndPointer,0,Reader->user,Reader->user2);"+cl);
			cs.Append("		}"+cl);
			cs.Append(cl);
			cs.Append("		if ((Reader->BodySize!=-1 && Reader->Body_Read>=Reader->BodySize)||(bytesReceived==0))"+cl);
			cs.Append("		{"+cl);
			cs.Append("			if (Reader->Body_BeginPointer == Reader->Body_EndPointer)"+cl);
			cs.Append("			{"+cl);
			cs.Append("				Reader->Body_BeginPointer = 0;"+cl);
			cs.Append("				Reader->Body_EndPointer = 0;"+cl);
			cs.Append("			}"+cl);
			cs.Append("			Reader->FunctionCallback(Reader, Reader->PacketHeader, 0,Reader->Body, &Reader->Body_BeginPointer, Reader->Body_EndPointer, -1,Reader->user,Reader->user2);"+cl);
//			cs.Append("			"+this.pc_SockClose+"(Reader->ClientSocket);"+cl);
			cs.Append("			if (Reader->Body!=NULL)"+cl);
			cs.Append("			{"+cl);
			cs.Append("				free(Reader->Body);"+cl);
			cs.Append("				Reader->Body = NULL;"+cl);
			cs.Append("				Reader->Body_MallocSize = 0;"+cl);
			cs.Append("			}"+cl);
//			cs.Append("			Reader->ClientSocket = ~0;"+cl);
			cs.Append("			Reader->BodySize = 0;"+cl);
			cs.Append("			Reader->FinRead=1;"+cl);
			cs.Append("			"+this.pc_methodPrefix+"LifeTime_Add(Reader->Parent->SocketTimer,Reader,1,&"+this.pc_methodPrefix+"ForceClose,NULL);"+cl);
			cs.Append("		}"+cl);
			cs.Append(cl);
			cs.Append("		if (Reader->Body_BeginPointer==Reader->Body_EndPointer)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			Reader->Body_BeginPointer = 0;"+cl);
			cs.Append("			Reader->Body_EndPointer = 0;"+cl);
			cs.Append("		}"+cl);
			cs.Append("	}"+cl);
			cs.Append("}"+cl);
			cs.Append(cl);
			cs.Append("void "+pc_methodPrefixDef+"CloseRequest(void *ReaderObject)"+cl);
			cs.Append("{"+cl);
			cs.Append("	if (((struct "+this.pc_methodPrefix+"HCHTTPReaderObject*)ReaderObject)->ClientSocket!=~0)"+cl);
			cs.Append("	{"+cl);
			cs.Append("		LVL3DEBUG(++(((struct "+this.pc_methodPrefix+"HCHTTPReaderObject*)ReaderObject)->Parent->CLOSE_COUNTER);)"+cl);
			cs.Append("		if (((struct "+this.pc_methodPrefix+"HCHTTPReaderObject*)ReaderObject)->Body!=NULL)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			free(((struct "+this.pc_methodPrefix+"HCHTTPReaderObject*)ReaderObject)->Body);"+cl);
			cs.Append("			((struct "+this.pc_methodPrefix+"HCHTTPReaderObject*)ReaderObject)->Body = NULL;"+cl);
			cs.Append("		}"+cl);
			cs.Append("		if (((struct "+this.pc_methodPrefix+"HCHTTPReaderObject*)ReaderObject)->PacketHeader!=NULL)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			"+this.pc_methodPrefix+"DestructPacket(((struct "+this.pc_methodPrefix+"HCHTTPReaderObject*)ReaderObject)->PacketHeader);"+cl);
			cs.Append("			((struct "+this.pc_methodPrefix+"HCHTTPReaderObject*)ReaderObject)->PacketHeader = NULL;"+cl);
			cs.Append("		}"+cl);
			cs.Append("		"+this.pc_SockClose+"(((struct "+this.pc_methodPrefix+"HCHTTPReaderObject*)ReaderObject)->ClientSocket);"+cl);
			cs.Append("		((struct "+this.pc_methodPrefix+"HCHTTPReaderObject*)ReaderObject)->ClientSocket = ~0;"+cl);
			cs.Append("		((struct "+this.pc_methodPrefix+"HCHTTPReaderObject*)ReaderObject)->BodySize = 0;"+cl);
			cs.Append("		((struct "+this.pc_methodPrefix+"HCHTTPReaderObject*)ReaderObject)->user = NULL;"+cl);
			cs.Append("		((struct "+this.pc_methodPrefix+"HCHTTPReaderObject*)ReaderObject)->user2 = NULL;"+cl);
			cs.Append("	}"+cl);
			cs.Append("}"+cl);
			cs.Append(cl);


			cs.Append("void "+this.pc_methodPrefix+"DeleteRequests(void *ClientModule, void *user1)"+cl);
			cs.Append("{"+cl);
			cs.Append("	struct HTTPClientModule *module = (struct HTTPClientModule*)ClientModule;"+cl);
			cs.Append("	struct RequestQueueNode *RequestNode = NULL,*TempNode = NULL;"+cl);
			cs.Append("	struct RequestQueueNode *DeleteHead=NULL,*DeleteTail=NULL;"+cl);
			cs.Append("	int i;"+cl);
			cs.Append("	sem_wait(&(module->QueueLock));"+cl);
			
			cs.Append("	for(i=0;i<module->NumSlots;++i)"+cl);
			cs.Append("	{"+cl);
			cs.Append("		if (module->Readers[i].ClientSocket!=~0 && module->Readers[i].user==user1)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			module->Readers[i].FinRead = 1;"+cl);
			cs.Append("			TempNode = (struct RequestQueueNode*)malloc(sizeof(struct RequestQueueNode));"+cl);
			cs.Append("			memset(TempNode,0,sizeof(struct RequestQueueNode));"+cl);
			cs.Append("			TempNode->FunctionCallback = module->Readers[i].FunctionCallback;"+cl);
			cs.Append("			TempNode->user = module->Readers[i].user;"+cl);
			cs.Append("			TempNode->user2 = module->Readers[i].user2;"+cl);
			cs.Append("			if (DeleteHead==NULL)"+cl);
			cs.Append("			{"+cl);
			cs.Append("				DeleteHead = TempNode;"+cl);
			cs.Append("				DeleteTail = TempNode;"+cl);
			cs.Append("			}"+cl);
			cs.Append("			else"+cl);
			cs.Append("			{"+cl);
			cs.Append("				DeleteTail->Next = TempNode;"+cl);
			cs.Append("				DeleteTail = TempNode;"+cl);
			cs.Append("			}"+cl);
			cs.Append("		}"+cl);
			cs.Append("	}"+cl);
	

			cs.Append("	if (module->First!=NULL)"+cl);
			cs.Append("	{"+cl);
			cs.Append("		TempNode = module->First;"+cl);
			cs.Append("		while(TempNode!=NULL)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			if (TempNode->user==user1)"+cl);
			cs.Append("			{"+cl);
			cs.Append("				//Match, Delete this item"+cl);
			cs.Append("				if (DeleteHead==NULL)"+cl);
			cs.Append("				{"+cl);
			cs.Append("					DeleteHead = TempNode;"+cl);
			cs.Append("				}"+cl);
			cs.Append("				else"+cl);
			cs.Append("				{"+cl);
			cs.Append("					DeleteTail->Next = TempNode;"+cl);
			cs.Append("				}"+cl);
			cs.Append("	"+cl);
			cs.Append("	"+cl);
			cs.Append("		"+cl);		
			cs.Append("				if (TempNode->Previous==NULL)"+cl);
			cs.Append("				{"+cl);
			cs.Append("					//First Item"+cl);
			cs.Append("					module->First = TempNode->Next;"+cl);
			cs.Append("					if (module->First!=NULL)"+cl);
			cs.Append("					{"+cl);
			cs.Append("						module->First->Previous = NULL;"+cl);
			cs.Append("						if (module->First->Next!=NULL)"+cl);
			cs.Append("						{"+cl);
			cs.Append("							module->First->Next->Previous = module->First;"+cl);
			cs.Append("						}"+cl);
			cs.Append("					}"+cl);
			cs.Append("				}"+cl);
			cs.Append("				else"+cl);
			cs.Append("				{"+cl);
			cs.Append("					//Not First Item"+cl);
			cs.Append("					TempNode->Previous->Next = TempNode->Next;"+cl);
			cs.Append("					if (TempNode->Next!=NULL)"+cl);
			cs.Append("					{"+cl);
			cs.Append("						TempNode->Next->Previous = TempNode->Previous;"+cl);
			cs.Append("					}"+cl);
			cs.Append("				}"+cl);
			cs.Append("	"+cl);
			cs.Append("				RequestNode = TempNode->Next;"+cl);
			cs.Append("				--module->RequestQueueCount;"+cl);
			cs.Append("				TempNode->Next = NULL;"+cl);
			cs.Append("				TempNode->Previous = DeleteTail;"+cl);
			cs.Append("				DeleteTail = TempNode;"+cl);
			cs.Append("				TempNode = RequestNode;"+cl);
			cs.Append("			}"+cl);
			cs.Append("			else"+cl);
			cs.Append("			{"+cl);
			cs.Append("				TempNode = TempNode->Next;"+cl);
			cs.Append("			}"+cl);
			cs.Append("		}"+cl);
			cs.Append("	}"+cl);
			cs.Append("	sem_post(&(module->QueueLock));"+cl);
			cs.Append("	"+cl);
			cs.Append("	"+cl);
			cs.Append("	TempNode = DeleteHead;"+cl);
			cs.Append("	while(TempNode!=NULL)"+cl);
			cs.Append("	{"+cl);
			cs.Append("		if (TempNode->FunctionCallback!=NULL)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			TempNode->FunctionCallback(ClientModule, NULL,HTTP_DELETEREQUEST_INTERRUPT, NULL, NULL, 0, -1, TempNode->user, TempNode->user2);"+cl);
			cs.Append("		}"+cl);
			cs.Append("		if (TempNode->Request!=NULL)	{free(TempNode->Request);}"+cl);
			cs.Append("		if (TempNode->Request2!=NULL) {free(TempNode->Request2);}"+cl);
			cs.Append("		DeleteHead = TempNode->Next;	"+cl);
			cs.Append("		free(TempNode);"+cl);
			cs.Append("		TempNode = DeleteHead;"+cl);
			cs.Append("	}"+cl);
			cs.Append("}"+cl);



			cs.Append("void "+pc_methodPrefixDef+"AddRequest_DirectEx(void *ClientModule, char *buffer, int bufferlength,char *buffer2, int buffer2length, struct sockaddr_in *Destination, void (*CallbackPtr)(void *reader, struct packetheader *header, int IsInterrupt, char* buffer, int *p_BeginPointer, int EndPointer, int done, void* user, void* user2),void* user, void* user2)"+cl);
			cs.Append("{"+cl);
			cs.Append("	struct HTTPClientModule *module = (struct HTTPClientModule*)ClientModule;"+cl);
			cs.Append("	struct RequestQueueNode *RequestNode = (struct RequestQueueNode*)malloc(sizeof(struct RequestQueueNode));"+cl);
			cs.Append("	struct RequestQueueNode *TempNode;"+cl);
			cs.Append(cl);
			cs.Append("	LVL3DEBUG(++module->ADD_COUNTER;)"+cl);
			cs.Append(cl);
			cs.Append("	RequestNode->RequestLength = bufferlength;"+cl);
			cs.Append("	RequestNode->Request = buffer;"+cl);
			cs.Append("	RequestNode->Request2 = buffer2;"+cl);
			cs.Append("	RequestNode->Request2Length = buffer2length;"+cl);
			cs.Append("	RequestNode->user = user;"+cl);
			cs.Append("	RequestNode->user2 = user2;"+cl);
			cs.Append("	RequestNode->FunctionCallback = CallbackPtr;"+cl);
			cs.Append("	RequestNode->Next = NULL;"+cl);
			cs.Append("	"+cl);
			cs.Append("	if (Destination!=NULL)"+cl);
			cs.Append("	{"+cl);
			cs.Append("		memset((char *)&(RequestNode->Destination), 0, sizeof(RequestNode->Destination));"+cl);
			cs.Append("		RequestNode->Destination.sin_family = AF_INET;"+cl);
			cs.Append("		RequestNode->Destination.sin_addr.s_addr = Destination->sin_addr.s_addr;"+cl);
			cs.Append("		RequestNode->Destination.sin_port = Destination->sin_port;"+cl);
			cs.Append("	}"+cl);
			cs.Append("	sem_wait(&(module->QueueLock));"+cl);
			cs.Append("		++module->RequestQueueCount;"+cl);
			cs.Append("		if (module->First==NULL)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			module->First = RequestNode;"+cl);
			cs.Append("			RequestNode->Previous = NULL;"+cl);
			cs.Append("		}"+cl);
			cs.Append("		else"+cl);
			cs.Append("		{"+cl);
			cs.Append("			TempNode = module->First;"+cl);
			cs.Append("			while(TempNode->Next!=NULL)"+cl);
			cs.Append("			{"+cl);
			cs.Append("				TempNode = TempNode->Next;"+cl);
			cs.Append("			}"+cl);
			cs.Append("			TempNode->Next = RequestNode;"+cl);
			cs.Append("			RequestNode->Previous = TempNode;"+cl);
			cs.Append("		}"+cl);
			cs.Append("	sem_post(&(module->QueueLock));"+cl);
			cs.Append("	sem_post(&(module->Monitor));"+cl);
			cs.Append("	"+this.pc_methodPrefix+"ForceUnBlockChain(module->Chain);"+cl);
			cs.Append("}"+cl);
			cs.Append("void "+pc_methodPrefixDef+"AddRequest(void *ClientModule, struct packetheader *packet,struct sockaddr_in *Destination, void (*CallbackPtr)(void *reader, struct packetheader *header, int IsInterrupt, char* buffer, int *p_BeginPointer, int EndPointer, int done, void* user, void* user2), void* user, void* user2)"+cl);
			cs.Append("{"+cl);
			cs.Append("	int BufferLength;"+cl);
			cs.Append("	char *Buffer;"+cl);
			cs.Append(cl);
			cs.Append("	BufferLength = "+this.pc_methodPrefix+"GetRawPacket(packet,&Buffer);"+cl);
			cs.Append("	"+this.pc_methodPrefix+"DestructPacket(packet);"+cl);
			cs.Append("	"+this.pc_methodPrefix+"AddRequest_Direct(ClientModule,Buffer,BufferLength,Destination,CallbackPtr,user,user2);"+cl);
			cs.Append("}"+cl);
			cs.Append(cl);
			cs.Append("void "+pc_methodPrefixDef+"HTTPClientModule_PreSelect(void *ClientModule,fd_set *readset, fd_set *writeset, fd_set *errorset, int *blocktime)"+cl);
			cs.Append("{"+cl);
			cs.Append("	int OK,idx;"+cl);
			cs.Append("	int i=0;"+cl);
			cs.Append("	int NumFree = 0;"+cl);
			cs.Append("	struct RequestQueueNode *data;"+cl);
			cs.Append("	struct HTTPClientModule *module = (struct HTTPClientModule*)ClientModule;"+cl);
			cs.Append(cl);
			cs.Append("	NumFree = 0;"+cl);
			cs.Append("	for(i=0;i<module->NumSlots;++i)"+cl);
			cs.Append("	{"+cl);
			cs.Append("		if (module->Readers[i].ClientSocket==~0) {++NumFree;}"+cl);
			cs.Append("	}"+cl);
			cs.Append("	DEBUGSTATEMENT(printf(\"NumFree = %d\\r\\n\",NumFree));"+cl);
			cs.Append("	DEBUGSTATEMENT(printf(\"NumSlots = %d\\r\\n\",module->NumSlots));"+cl);
			cs.Append(cl);
			cs.Append("for(i=0;i<NumFree;++i)"+cl);
			cs.Append("{"+cl);
			cs.Append("	if (sem_trywait(&(module->Monitor))==0 || module->RequestQueueCount>0)"+cl);
			cs.Append("	{"+cl);
			cs.Append("		sem_wait(&(module->QueueLock));"+cl);
			cs.Append("		data = module->First;"+cl);
			cs.Append("		OK = 0;"+cl);
			cs.Append("		while(OK==0 && data !=NULL)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			OK = -1;"+cl);
			cs.Append("			for(idx=0;idx<module->NumSlots;++idx)"+cl);
			cs.Append("			{"+cl);
			cs.Append("				if (module->Readers[idx].ClientSocket!=~0 && module->Readers[idx].user==data->user)"+cl);
			cs.Append("				{"+cl);
			cs.Append("					// Try Again with another Request"+cl);
			cs.Append("					OK = 0;"+cl);
			cs.Append("					data = data->Next;"+cl);
			cs.Append("					break;"+cl);
			cs.Append("				}"+cl);
			cs.Append("			}"+cl);
			cs.Append("		}"+cl);
			cs.Append(""+cl);
			cs.Append("		if (data!=NULL)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			if (data->Previous == NULL)"+cl);
			cs.Append("			{"+cl);
			cs.Append("				//First Item"+cl);
			cs.Append("				module->First = data->Next;"+cl);
			cs.Append("				if (module->First!=NULL)"+cl);
			cs.Append("				{"+cl);
			cs.Append("					module->First->Previous = NULL;"+cl);
			cs.Append("				}"+cl);
			cs.Append("			}"+cl);
			cs.Append("			else"+cl);
			cs.Append("			{"+cl);
			cs.Append("				//Not First Item"+cl);
			cs.Append("				data->Previous->Next = data->Next;"+cl);
			cs.Append("				if (data->Next!=NULL)"+cl);
			cs.Append("				{"+cl);
			cs.Append("					data->Next->Previous = data->Previous;"+cl);
			cs.Append("				}"+cl);
			cs.Append("			}"+cl);
			cs.Append("		}"+cl);
			cs.Append("		sem_post(&(module->QueueLock));"+cl);
			cs.Append(""+cl);
			cs.Append("		if (data!=NULL)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			"+this.pc_methodPrefix+"StartRequest(module,data);"+cl);
			cs.Append("			free(data);"+cl);
			cs.Append("			--module->RequestQueueCount;"+cl);
			cs.Append("		}"+cl);
			cs.Append("	}"+cl);
			cs.Append("	else"+cl);
			cs.Append("	{"+cl);
			cs.Append("		break;"+cl);
			cs.Append("	}"+cl);
			cs.Append("}"+cl);

			cs.Append(cl);
			cs.Comment("Pre Select");
			cs.Append("	for(i=0;i<module->NumSlots;++i)"+cl);
			cs.Append("	{"+cl);
			cs.Append("		if (module->Readers[i].ClientSocket!=~0)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			if (module->Readers[i].FinConnect==0)"+cl);
			cs.Append("			{"+cl);
			cs.Comment("Not Connected Yet");
			cs.Append("				FD_SET(module->Readers[i].ClientSocket,writeset);"+cl);
			cs.Append("				FD_SET(module->Readers[i].ClientSocket,errorset);"+cl);
			cs.Append("			}"+cl);
			cs.Append("			else"+cl);
			cs.Append("			{"+cl);
			cs.Comment("Already Connected, just needs reading");
			cs.Append("				FD_SET(module->Readers[i].ClientSocket,readset);"+cl);
			cs.Append("				FD_SET(module->Readers[i].ClientSocket,errorset);"+cl);
			cs.Append("			}"+cl);
			cs.Append("		}"+cl);
			cs.Append("	}"+cl);
			cs.Append("}"+cl);

			cs.Append("void "+pc_methodPrefixDef+"HTTPClient_PostSelect(void *ClientModule, int slct, fd_set *readset, fd_set *writeset, fd_set *errorset)"+cl);
			cs.Append("{"+cl);
			cs.Append("	int i=0;"+cl);
			cs.Append("	int tst;"+cl);
			if (this.Platform == PLATFORMS.WINDOWS) 
			{
				cs.Append("	unsigned long flags;"+cl);
			} 
			else 
			{
				cs.Append("	unsigned int flags;"+cl);
			}
			cs.Append("	struct sockaddr_in receivingAddress;"+cl);
			cs.Append("	int receivingAddressLength = sizeof(struct sockaddr_in);"+cl);
			cs.Append("	struct HTTPClientModule *module = (struct HTTPClientModule*)ClientModule;"+cl);
			cs.Append(cl);
			cs.Append("	for(i=0;i<module->NumSlots;++i)"+cl);
			cs.Append("	{"+cl);
			cs.Append("		if (module->Readers[i].ClientSocket!=~0)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			if (module->Readers[i].FinConnect==0)"+cl);
			cs.Append("			{"+cl);
			cs.Comment("Not Connected Yet");
			cs.Append("				if (FD_ISSET(module->Readers[i].ClientSocket,writeset)!=0)"+cl);
			cs.Append("				{"+cl);
			cs.Comment("Connected");
			cs.Append("					getsockname(module->Readers[i].ClientSocket,(struct sockaddr*)&receivingAddress,&receivingAddressLength);"+cl);
			cs.Append("					module->Readers[i].LocalIPAddress = receivingAddress.sin_addr.s_addr;"+cl);
			cs.Append("					module->Readers[i].FinConnect = 1;"+cl);
			cs.Append("					module->Readers[i].BodySize = 0;"+cl);
			cs.Append("					module->Readers[i].Body_Read = 0;"+cl);
			cs.Append("					module->Readers[i].Body_BeginPointer = 0;"+cl);
			cs.Append("					module->Readers[i].Body_EndPointer = 0;"+cl);
			cs.Append("					module->Readers[i].HeaderIndex = 0;"+cl);
			cs.Append("				"+cl);
			if (this.Platform==PLATFORMS.POSIX)
			{
				cs.Comment("Platform Dependent [POSIX]");
				cs.Append("					flags = fcntl(module->Readers[i].ClientSocket,F_GETFL,0);"+cl);
				cs.Append("					fcntl(module->Readers[i].ClientSocket,F_SETFL,(~O_NONBLOCK)&flags);"+cl);
			}
			else
			{
				cs.Comment("Platform Dependent [Windows]");
				cs.Append("					flags = 0;"+cl);
				cs.Append("					ioctlsocket(module->Readers[i].ClientSocket,FIONBIO,&flags);"+cl);
			}
			cs.Append("					tst=send(module->Readers[i].ClientSocket,module->Readers[i].send_data,module->Readers[i].send_dataLength,0);"+cl);
			cs.Append("					LVL3DEBUG(if (tst!=module->Readers[i].send_dataLength))"+cl);
			cs.Append("					LVL3DEBUG({)"+cl);
			cs.Append("					LVL3DEBUG(	++module->SEND_FAIL;)"+cl);
			cs.Append("					LVL3DEBUG(})"+cl);
			cs.Append("					LVL3DEBUG(else)"+cl);
			cs.Append("					LVL3DEBUG({)"+cl);
			cs.Append("					LVL3DEBUG(	++module->CONNECT_COUNTER;)"+cl);
			cs.Append("					LVL3DEBUG(})"+cl);
			cs.Append("					free(module->Readers[i].send_data);"+cl);
			cs.Append("					module->Readers[i].send_data=NULL;"+cl);
			cs.Append("					if (module->Readers[i].send_data2!=NULL)"+cl);
			cs.Append("					{"+cl);
			cs.Append("						tst=send(module->Readers[i].ClientSocket,module->Readers[i].send_data2,module->Readers[i].send_data2Length,0);"+cl);
			cs.Append("						LVL3DEBUG(if (tst!=module->Readers[i].send_dataLength))"+cl);
			cs.Append("						LVL3DEBUG({)"+cl);
			cs.Append("						LVL3DEBUG(	++module->SEND_FAIL2;)"+cl);
			cs.Append("						LVL3DEBUG(})"+cl);
			cs.Append("						free(module->Readers[i].send_data2);"+cl);
			cs.Append("						module->Readers[i].send_data2=NULL;"+cl);
			cs.Append("					}"+cl);
			
			cs.Append("				}"+cl);
			cs.Append("				if (FD_ISSET(module->Readers[i].ClientSocket,errorset)!=0)"+cl);
			cs.Append("				{"+cl);
			cs.Comment("Connection Failed");
			cs.Append("					LVL3DEBUG(++(module->FAILED_COUNTER);)"+cl);
			cs.Append("					if (module->Readers[i].send_data!=NULL)"+cl);
			cs.Append("					{"+cl);
			cs.Append("						free(module->Readers[i].send_data);"+cl);
			cs.Append("						module->Readers[i].send_data = NULL;"+cl);
			cs.Append("					}"+cl);
			cs.Append("					if (module->Readers[i].send_data2!=NULL)"+cl);
			cs.Append("					{"+cl);
			cs.Append("						free(module->Readers[i].send_data2);"+cl);
			cs.Append("						module->Readers[i].send_data2 = NULL;"+cl);
			cs.Append("					}"+cl);
			cs.Append("					"+this.pc_SockClose+"(module->Readers[i].ClientSocket);"+cl);
			cs.Append("					module->Readers[i].Body_BeginPointer = 0;"+cl);
			cs.Append("					module->Readers[i].ClientSocket = ~0;"+cl);
			cs.Append("					module->Readers[i].BodySize = 0;"+cl);
			cs.Append("					module->Readers[i].FunctionCallback(&(module->Readers[i]), NULL,0, NULL, &module->Readers[i].Body_BeginPointer, 0, -1,module->Readers[i].user,module->Readers[i].user2);"+cl);
			cs.Append("				}"+cl);
			cs.Append("			}"+cl);
			cs.Append("			else"+cl);
			cs.Append("			{"+cl);
			cs.Comment("Already Connected, just needs reading");
			cs.Append("				if (FD_ISSET(module->Readers[i].ClientSocket,readset)!=0)"+cl);
			cs.Append("				{"+cl);
			cs.Comment("Data Available");
			cs.Append("					"+pc_methodPrefix+"ProcessSocket(&(module->Readers[i]));"+cl);
			cs.Append("				}"+cl);
			cs.Comment("Check if PeerReset");
			cs.Append("				else if (FD_ISSET(module->Readers[i].ClientSocket,errorset)!=0)"+cl);
			cs.Append("				{"+cl);
			cs.Comment("Socket Closed");
			cs.Append("					"+this.pc_SockClose+"(module->Readers[i].ClientSocket);"+cl);
			cs.Append("					module->Readers[i].ClientSocket = ~0;"+cl);
			cs.Append("					module->Readers[i].BodySize = 0;"+cl);
			cs.Append("					if (module->Readers[i].BodySize==-1)"+cl);
			cs.Append("					{"+cl);
			cs.Append("						module->Readers[i].Body_BeginPointer = 0;"+cl);
			cs.Append("						module->Readers[i].FunctionCallback(&(module->Readers[i]), module->Readers[i].PacketHeader, 0,NULL, &module->Readers[i].Body_BeginPointer, 0, -1, module->Readers[i].user,module->Readers[i].user2);"+cl);
			cs.Append("					}"+cl);
			cs.Append("				}"+cl);
			cs.Append("			}"+cl);
			cs.Append("		}"+cl);
			cs.Append("	}"+cl);
			cs.Append("}"+cl);
			cs.Append(cl);


			cs.Append("void* "+pc_methodPrefixDef+"CreateHTTPClientModule(void *Chain, int MaxSockets)"+cl);
			cs.Append("{"+cl);
			cs.Append("	struct HTTPClientModule *RetVal = (struct HTTPClientModule*)malloc(sizeof(struct HTTPClientModule));"+cl);
			cs.Append("	int i=0;"+cl);
			cs.Append("	struct timeval tv;"+cl);
			cs.Append(cl);
			cs.Append("	gettimeofday(&tv,NULL);"+cl);
			cs.Append("	srand((int)tv.tv_sec);"+cl);
			cs.Append(cl);
			cs.Append("	LVL3DEBUG(RetVal->ADD_COUNTER=0;)"+cl);
			cs.Append("	LVL3DEBUG(RetVal->FAILED_COUNTER = 0;)"+cl);
			cs.Append("	LVL3DEBUG(RetVal->GRACEFUL_COUNTER = 0;)"+cl);
			cs.Append("	LVL3DEBUG(RetVal->FORCE_COUNTER = 0;)"+cl);
			cs.Append("	LVL3DEBUG(RetVal->START_COUNTER = 0;)"+cl);
			cs.Append("	LVL3DEBUG(RetVal->CLOSE_COUNTER = 0;)"+cl);
			cs.Append("	LVL3DEBUG(RetVal->SEND_FAIL = 0;)"+cl);
			cs.Append("	LVL3DEBUG(RetVal->SEND_FAIL2 = 0;)"+cl);
			cs.Append("	LVL3DEBUG(RetVal->CONNECT_COUNTER = 0;)"+cl);
			cs.Append(cl);
			cs.Append("	RetVal->RequestQueueCount = 0;"+cl);
			cs.Append("	RetVal->Terminate = 0;"+cl);
			cs.Append("	RetVal->NumSlots = MaxSockets;"+cl);
			cs.Append("	RetVal->First = NULL;"+cl);
			cs.Append("	RetVal->Readers = (struct "+this.pc_methodPrefix+"HCHTTPReaderObject*)malloc(MaxSockets*sizeof(struct "+this.pc_methodPrefix+"HCHTTPReaderObject));"+cl);
			cs.Append(cl);
			cs.Append("	memset(RetVal->Readers,0,MaxSockets*sizeof(struct "+this.pc_methodPrefix+"HCHTTPReaderObject));"+cl);
			cs.Append("	for(i=0;i<MaxSockets;++i)"+cl);
			cs.Append("	{"+cl);
			cs.Append("		RetVal->Readers[i].ClientSocket = ~0;"+cl);
			cs.Append("		RetVal->Readers[i].Parent = RetVal;"+cl);
			cs.Append("	}"+cl);
			cs.Append(cl);
			cs.Append("	sem_init(&(RetVal->QueueLock),0,1);"+cl);
			cs.Append("	sem_init(&(RetVal->Monitor),0,0);"+cl);
			cs.Append("	RetVal->PreSelect = &"+this.pc_methodPrefix+"HTTPClientModule_PreSelect;"+cl);
			cs.Append("	RetVal->PostSelect = &"+this.pc_methodPrefix+"HTTPClient_PostSelect;"+cl);
			cs.Append("	RetVal->Destroy = &"+this.pc_methodPrefix+"DestroyHTTPClientModule;"+cl);
			cs.Append("	RetVal->Chain = Chain;"+cl);
			cs.Append(cl);
			cs.Append("	RetVal->SocketTimer = "+this.pc_methodPrefix+"CreateLifeTime(Chain);"+cl);
			cs.Append(cl);
			cs.Append("	"+this.pc_methodPrefix+"AddToChain(Chain,RetVal);"+cl);
			cs.Append("	return((void*)RetVal);"+cl);
			cs.Append("}"+cl);
			cs.Append(cl);

			writer = File.CreateText(outputDirectory.FullName + "\\"+pc_methodPrefix+"HTTPClient.c");
			writer.Write(cs.ToString());
			writer.Close();

			return(true);
		}

		public string PrintfTransform(string data)
		{
			data = data.Replace("\\","\\\\");
			data = data.Replace("\r","\\r");
			data = data.Replace("\n","\\n");
			data = data.Replace("\"","\\\"");
			return data;
		}

		public string ToCType(string t) 
		{
			switch (t) 
			{
				case "System.Char": return "char";
				case "System.String": return "char*";
				case "System.Boolean": return "int";
				case "System.Uri": return "char*";
				case "System.Byte": return "unsigned char";
				case "System.UInt16": return "unsigned short";
				case "System.UInt32": return "unsigned int";
				case "System.Int32": return "int";
				case "System.Int16": return "short";
				case "System.SByte": return "char";
				case "System.Single": return "float";
				case "System.Double": return "double";
				case "System.Byte[]": return "char*";
				default: return "char*";
			}
		}

		public string ToPrintfType(string t) 
		{
			switch (t) 
			{
				case "System.Boolean":
				case "System.Byte[]":
				case "System.String":
				case "System.Uri":
					return "%s";
				case "System.Byte":
				case "System.UInt16":
				case "System.UInt32":
					return "%u";
				case "System.Char":
				case "System.SByte":
				case "System.Int16":
				case "System.Int32":
					return "%d";
				case "System.Single":
				case "System.Double":
					return "%f";
				default:
					return "void";
			}
		}

		public string ToPrintfTypeBool(string t) 
		{
			switch (t) 
			{
				case "System.Byte[]":
				case "System.String":
				case "System.Uri":
					return "%s";
				case "System.Byte":
				case "System.UInt16":
				case "System.UInt32":
					return "%u";
				case "System.Boolean":
				case "System.Char":
				case "System.SByte":
				case "System.Int16":
				case "System.Int32":
					return "%d";
				case "System.Single":
				case "System.Double":
					return "%f";
				default:
					return "void";
			}
		}

		public string ToSampleValue(string t) 
		{
			switch (t) 
			{
				case "System.Boolean":
					return "1";
				case "System.Byte[]":
				case "System.String":
					return "\"Sample String\"";
				case "System.Uri":
					return "\"http://opentools.homeip.net\"";
				case "System.Byte":
					return "250";
				case "System.UInt16":
					return "250";
				case "System.UInt32":
					return "250";
				case "System.Char":
				case "System.SByte":
					return "250";
				case "System.Int16":
					return "25000";
				case "System.Int32":
					return "25000";
				case "System.Single":
				case "System.Double":
					return "0.01";
				default:
					return "NULL";
			}
		}

		public string ToEmptyValue(string t) 
		{
			switch (t) 
			{
				case "System.Byte[]":
					return "NULL";
				case "System.String":
				case "System.Uri":
					return "\"\"";
				case "System.Boolean":
				case "System.Byte":
				case "System.UInt16":
				case "System.UInt32":
				case "System.Char":
				case "System.SByte":
				case "System.Int16":
				case "System.Int32":
				case "System.Single":
				case "System.Double":
					return "0";
				default:
					return "NULL";
			}
		}

		public int FromHex(string hn)
		{
			return(int.Parse(hn.ToUpper(),System.Globalization.NumberStyles.HexNumber));
		}

		public string ToHex(object obj)
		{
			if (obj.GetType().FullName=="System.UInt32")
			{
				UInt32 unumber = UInt32.Parse(obj.ToString());
				return(unumber.ToString("X"));
			}
			else
			{
				Int32 number = Int32.Parse(obj.ToString());
				return(number.ToString("X"));
			}
		}

	}
}
