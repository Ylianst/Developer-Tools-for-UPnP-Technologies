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
using OpenSource.UPnP;
using OpenSource.UPnP.AV.CdsMetadata;
using OpenSource.UPnP.AV.MediaServer.CP;

namespace OpenSource.UPnP.AV.Extensions.MetaData
{
	/// <summary>
	/// Summary description for Class1.
	/// </summary>
	public sealed class Finder
	{
		private Finder()
		{
		}

		public static MediaItem PopulateMetaData(MediaResource R, FileInfo F)
		{
			MediaItem RetVal;
			MediaBuilder.item Item = null;
			DText parser = new DText();
			parser.ATTRMARK = "-";
			parser.MULTMARK = ".";

			switch(F.Extension.ToUpper())
			{
				case ".MP3":
					Item = ParseMP3_V1(F);
					if(Item==null)
					{
						parser[0] = F.Name;
						if(parser.DCOUNT()==2)
						{
							Item = new MediaBuilder.musicTrack(parser[2,1].Trim());
							Item.creator = parser[1].Trim();
							((MediaBuilder.musicTrack)Item).artist = new PersonWithRole[1]{new PersonWithRole()};
							((MediaBuilder.musicTrack)Item).artist[0].Name = Item.creator;
							((MediaBuilder.musicTrack)Item).artist[0].Role = null;	
						}
					}
					break;
			}
			
			if(Item!=null)
			{
				RetVal = MediaBuilder.CreateItem(Item);
				RetVal.AddResource(R);
				return(RetVal);
			}
			else
			{
				// Create a Generic Item
				string fname = F.Name;
				int fnameidx = fname.IndexOf(".");
				if(fnameidx!=-1) fname=fname.Substring(0,fnameidx);
				MediaBuilder.item genericItem = new MediaBuilder.item(fname);
				RetVal = MediaBuilder.CreateItem(genericItem);
				RetVal.AddResource(R);
				return(RetVal);
			}
		}

		private static MediaBuilder.item ParseMP3_V1(FileInfo f)
		{
			if (f.Exists == false) return null;

			Stream fileData;
			byte[] buffer = new byte[128];
			char[] CharBuffer = new char[128];
			string StrBuffer = "";

			fileData = f.OpenRead();
			if (fileData.Length <= 128) 
			{
				fileData.Close();
				return null;
			}

			fileData.Seek(-128,SeekOrigin.End);
			fileData.Read(buffer,0,128);
			fileData.Close();

			for (int id=0;id<128;++id)
			{
				CharBuffer[id] = Convert.ToChar(buffer[id]);
				if (CharBuffer[id]==0) {CharBuffer[id] = ' ';}
			}
			StrBuffer = new String(CharBuffer,0,128);

			if(StrBuffer.Substring(0,3) != "TAG") return null;

			// This has a valid ID3 Tag
			string SongTitle = StrBuffer.Substring(3,30).Trim();
			string AlbumName = StrBuffer.Substring(63,30).Trim();
			string ArtistName = StrBuffer.Substring(33,30).Trim();

			if(SongTitle=="") return(null);
			
			MediaBuilder.musicTrack RetVal = new MediaBuilder.musicTrack(SongTitle);
			RetVal.album = new string[1]{AlbumName};
			RetVal.artist = new PersonWithRole[1]{new PersonWithRole()};
			RetVal.artist[0].Name = ArtistName;
			RetVal.artist[0].Role = null;
			RetVal.creator = ArtistName;
			return((MediaBuilder.item)RetVal);
		}
	}
}
