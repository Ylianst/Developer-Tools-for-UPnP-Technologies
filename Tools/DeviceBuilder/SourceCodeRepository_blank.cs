using System;
using System.IO;
using System.Windows.Forms;

namespace UPnPStackBuilder
{
	/// <summary>
	/// Summary description for SourceCodeRepository.
	/// </summary>
	public class SourceCodeRepository
	{
		#region EULA
		// -=S3P4R470R=- {EULA}
		// -=S3P4R470R=- {EULA}
		#endregion
		#region UPnPMicroStack.c
		// -=S3P4R470R=- {UPnPMicroStack_C}
		// -=S3P4R470R=- {UPnPMicroStack_C}
		#endregion
		#region UPnPMicroStack.h
		// -=S3P4R470R=- {UPnPMicroStack_H}
		// -=S3P4R470R=- {UPnPMicroStack_H}
		#endregion
		#region ILibParsers.c
		// -=S3P4R470R=- {ILibParsers_C}
		// -=S3P4R470R=- {ILibParsers_C}
		#endregion
		#region ILibParsers.h
		// -=S3P4R470R=- {ILibParsers_H}
		// -=S3P4R470R=- {ILibParsers_H}
		#endregion
		#region ILibAsyncSocket.c
		// -=S3P4R470R=- {ILibAsyncSocket_C}
		// -=S3P4R470R=- {ILibAsyncSocket_C}
		#endregion
		#region ILibAsyncSocket.h
		// -=S3P4R470R=- {ILibAsyncSocket_H}
		// -=S3P4R470R=- {ILibAsyncSocket_H}
		#endregion
		#region ILibAsyncServerSocket.c
		// -=S3P4R470R=- {ILibAsyncServerSocket_C}
		// -=S3P4R470R=- {ILibAsyncServerSocket_C}
		#endregion
		#region ILibAsyncServerSocket.h
		// -=S3P4R470R=- {ILibAsyncServerSocket_H}
		// -=S3P4R470R=- {ILibAsyncServerSocket_H}
		#endregion
		#region ILibWebClient.c
		// -=S3P4R470R=- {ILibWebClient_C}
		// -=S3P4R470R=- {ILibWebClient_C}
		#endregion
		#region ILibWebClient.h
		// -=S3P4R470R=- {ILibWebClient_H}
		// -=S3P4R470R=- {ILibWebClient_H}
		#endregion
		#region ILibWebServer.c
		// -=S3P4R470R=- {ILibWebServer_C}
		// -=S3P4R470R=- {ILibWebServer_C}
		#endregion
		#region ILibWebServer.h
		// -=S3P4R470R=- {ILibWebServer_H}
		// -=S3P4R470R=- {ILibWebServer_H}
		#endregion
		#region ILibSSDPClient.c
		// -=S3P4R470R=- {ILibSSDPClient_C}
		// -=S3P4R470R=- {ILibSSDPClient_C}
		#endregion
		#region ILibSSDPClient.h
		// -=S3P4R470R=- {ILibSSDPClient_H}
		// -=S3P4R470R=- {ILibSSDPClient_H}
		#endregion
		#region UPnPControlPointStructs.h
		// -=S3P4R470R=- {UPnPControlPointStructs_H}
		// -=S3P4R470R=- {UPnPControlPointStructs_H}
		#endregion


		public SourceCodeRepository()
		{
		}

		public static string GetMicroStack_C_Template(string PreFix)
		{
			return(GetEULA(PreFix+"MicroStack.c","") + RemoveOldEULA(UPnPMicroStack_C));
		}
		public static string GetMicroStack_H_Template(string PreFix)
		{
			return(GetEULA(PreFix+"MicroStack.h","") + RemoveOldEULA(UPnPMicroStack_H));
		}
		private static string GetEULA(string FileName, string Settings)
		{
			string RetVal = EULA;
			if(Settings=="") {Settings="*";}

			RetVal = RetVal.Replace("<REVISION>","#"+Application.ProductVersion);
			RetVal = RetVal.Replace("<DATE>",DateTime.Now.ToLongDateString());
			RetVal = RetVal.Replace("<FILE>",FileName);
			RetVal = RetVal.Replace("<SETTINGS>",Settings);

			return(RetVal);
		}
		private static string RemoveOldEULA(string InVal)
		{
			string RetVal = InVal;
			RetVal = RetVal.Substring(2+RetVal.IndexOf("*/"));
			return(RetVal);
		}

		public static void Generate_Parsers(string PreFix, DirectoryInfo outputDir)
		{
			StreamWriter writer;
			string lib;

			lib = GetEULA(PreFix+"Parsers.h","") + RemoveOldEULA(ILibParsers_H);
			lib = lib.Replace("ILib",PreFix);

			writer = File.CreateText(outputDir.FullName + "\\"+PreFix+"Parsers.h");
			writer.Write(lib);
			writer.Close();

			lib = GetEULA(PreFix+"Parsers.c","") + RemoveOldEULA(ILibParsers_C);
			lib = lib.Replace("ILib",PreFix);
			writer = File.CreateText(outputDir.FullName + "\\"+PreFix+"Parsers.c");
			writer.Write(lib);
			
			writer.Close();
		}
		public static void Generate_AsyncSocket(string PreFix, DirectoryInfo outputDir)
		{
			StreamWriter writer;
			string lib;

			lib = GetEULA(PreFix+"AsyncSocket.h","") + RemoveOldEULA(ILibAsyncSocket_H);
			lib = lib.Replace("ILib",PreFix);

			writer = File.CreateText(outputDir.FullName + "\\"+PreFix+"AsyncSocket.h");
			writer.Write(lib);
			writer.Close();

			lib = GetEULA(PreFix+"AsyncSocket.c","") + RemoveOldEULA(ILibAsyncSocket_C);
			lib = lib.Replace("ILib",PreFix);
			writer = File.CreateText(outputDir.FullName + "\\"+PreFix+"AsyncSocket.c");
			writer.Write(lib);
			
			writer.Close();
		}
		public static void Generate_AsyncServerSocket(string PreFix, DirectoryInfo outputDir)
		{
			StreamWriter writer;
			string lib;

			lib = GetEULA(PreFix+"AsyncServerSocket.h","") + RemoveOldEULA(ILibAsyncServerSocket_H);
			lib = lib.Replace("ILib",PreFix);

			writer = File.CreateText(outputDir.FullName + "\\"+PreFix+"AsyncServerSocket.h");
			writer.Write(lib);
			writer.Close();

			lib = GetEULA(PreFix+"AsyncServerSocket.c","") + RemoveOldEULA(ILibAsyncServerSocket_C);
			lib = lib.Replace("ILib",PreFix);
			writer = File.CreateText(outputDir.FullName + "\\"+PreFix+"AsyncServerSocket.c");
			writer.Write(lib);
			
			writer.Close();
		}
		public static void Generate_WebClient(string PreFix, DirectoryInfo outputDir, bool Legacy)
		{
			StreamWriter writer;
			string lib;

			lib = GetEULA(PreFix+"WebClient.h","") + RemoveOldEULA(ILibWebClient_H);
			lib = lib.Replace("ILib",PreFix);

			writer = File.CreateText(outputDir.FullName + "\\"+PreFix+"WebClient.h");
			writer.Write(lib);
			writer.Close();

			lib = GetEULA(PreFix+"WebClient.c","") + RemoveOldEULA(ILibWebClient_C);
			lib = lib.Replace("ILib",PreFix);

			if(Legacy)
			{
				// Remove HTTP/1.1 Specific Code
				string xx = "//{{{ REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT--> }}}";
				string yy = "//{{{ <--REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT }}}";
				int ix = lib.IndexOf(xx);
				int iy;
				while(ix!=-1)
				{
					iy = lib.IndexOf(yy) + yy.Length;
					lib = lib.Remove(ix,iy-ix);
					ix = lib.IndexOf(xx);
				}
			}

			writer = File.CreateText(outputDir.FullName + "\\"+PreFix+"WebClient.c");
			writer.Write(lib);
			
			writer.Close();
		}
		public static void Generate_WebServer(string PreFix, DirectoryInfo outputDir, bool Legacy)
		{
			StreamWriter writer;
			string lib;

			lib = GetEULA(PreFix+"WebServer.h","") + RemoveOldEULA(ILibWebServer_H);
			lib = lib.Replace("ILib",PreFix);

			writer = File.CreateText(outputDir.FullName + "\\"+PreFix+"WebServer.h");
			writer.Write(lib);
			writer.Close();

			lib = GetEULA(PreFix+"WebServer.c","") + "\r\n";
			if(Legacy)
			{
				lib += "#define HTTPVERSION \"1.0\"";
			}
			else
			{
				lib += "#define HTTPVERSION \"1.1\"";
			}
			lib += RemoveOldEULA(ILibWebServer_C);
			lib = lib.Replace("ILib",PreFix);
			if(Legacy)
			{
				// Remove HTTP/1.1 Specific Code
				string xx = "//{{{ REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT--> }}}";
				string yy = "//{{{ <--REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT }}}";
				int ix = lib.IndexOf(xx);
				int iy;
				while(ix!=-1)
				{
					iy = lib.IndexOf(yy) + yy.Length;
					lib = lib.Remove(ix,iy-ix);
					ix = lib.IndexOf(xx);
				}
			}
			writer = File.CreateText(outputDir.FullName + "\\"+PreFix+"WebServer.c");
			writer.Write(lib);
			
			writer.Close();
		}
		public static string RemoveAndClearTag(string BeginTag, string EndTag, string data)
		{
			// Remove HTTP/1.1 Specific Code
			int ix = data.IndexOf(BeginTag);
			int iy;
			while(ix!=-1)
			{
				iy = data.IndexOf(EndTag) + EndTag.Length;
				data = data.Remove(ix,iy-ix);
				ix = data.IndexOf(BeginTag);
			}
			return(data);
		}
		public static string RemoveTag(string BeginTag, string EndTag, string data)
		{
			data = data.Replace(BeginTag,"");
			data = data.Replace(EndTag,"");
			return(data);
		}
		public static void Generate_SSDPClient(string PreFix, DirectoryInfo outputDir)
		{
			StreamWriter writer;
			string lib;

			lib = GetEULA(PreFix+"SSDPClient.h","") + RemoveOldEULA(ILibSSDPClient_H);
			lib = lib.Replace("ILib",PreFix);

			writer = File.CreateText(outputDir.FullName + "\\"+PreFix+"SSDPClient.h");
			writer.Write(lib);
			writer.Close();

			lib = GetEULA(PreFix+"SSDPClient.c","") + RemoveOldEULA(ILibSSDPClient_C);
			lib = lib.Replace("ILib",PreFix);
			writer = File.CreateText(outputDir.FullName + "\\"+PreFix+"SSDPClient.c");
			writer.Write(lib);
			
			writer.Close();
		}
		public static void Generate_UPnPControlPointStructs(string PreFix, string ReplaceText,DirectoryInfo outputDir)
		{
			StreamWriter writer;
			string lib;

			lib = GetEULA("UPnPControlPointStructs.h","") + RemoveOldEULA(UPnPControlPointStructs_H);
			lib = lib.Replace("ILib",PreFix);
			lib = lib.Replace("<REPLACE>",ReplaceText);

			writer = File.CreateText(outputDir.FullName + "\\UPnPControlPointStructs.h");
			writer.Write(lib);		
			writer.Close();
		}
	}
}
