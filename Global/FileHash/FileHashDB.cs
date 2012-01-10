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
using System.Text;
using System.Threading;
using System.Collections;
using System.CodeDom.Compiler;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Formatters.Binary;

namespace OpenSource.FileHashDB
{
	
	/// <summary>
	/// Summary description for FileHashDB.
	/// </summary>
	public class FileHashDB
	{
		[Serializable()]
		private struct IndexStruct
		{
			public int EngineVersion;
			public int UpdateID;
			public Hashtable IndexHashTable;
		}

		private TempFileCollection tfc = null;
		private BinaryFormatter formatter = new BinaryFormatter();
		private Thread InitThread;
		public short MaxBlockSize = 512;
		public byte[] InitBuffer = new byte[10];
		public byte[] IndexBuffer;
		public byte[] DeleteBuffer;

		private int EngineVersion = 1;
		private string IndexFilePath = "";

		private byte[] ClearBlock;
		private int Indexer = 1;
		private int UpdateID = 1;
		private FileInfo DBFileInfo;

		private int InitializeCounter = 1;

		private AsyncCallback DeleteCB;
		private AsyncCallback ClearCB;

		private Hashtable IndexMap = Hashtable.Synchronized(new Hashtable());

		public delegate void OnReadyHandler(FileHashDB sender, Exception e);
		public event OnReadyHandler OnReady;

		public delegate void OnIndexRebuiltHandler(FileHashDB sender);
		public event OnIndexRebuiltHandler OnIndexRebuilt;

		public delegate void OnReadHandler(FileHashDB sender, string KEY, byte[] buffer, object Tag);
		public delegate void OnWriteHandler(FileHashDB sender, string KEY, object Tag);
		public delegate void OnReadObjectHandler(FileHashDB sender, string KEY, object obj, object Tag);
		private OnReadHandler ObjectReadCB;

		private Hashtable PendingReadTable = Hashtable.Synchronized(new Hashtable());
		internal FileStream fstream;
		private Hashtable Map = null;
		private Hashtable CBMap = new Hashtable();

		private SortedList FreeBlockList = new SortedList();
		private int EPtr = 10;
		private long handle = 0;

		internal Mutex SeekLock = new Mutex();

		~FileHashDB()
		{
			if(tfc==null)
			{
				IndexStruct iss = new IndexStruct();
				iss.EngineVersion = EngineVersion;
				iss.UpdateID = UpdateID;
				iss.IndexHashTable = Map;

				BinaryFormatter bf = new BinaryFormatter();
				FileStream fs = new FileStream(this.IndexFilePath,FileMode.OpenOrCreate);
				bf.Serialize(fs,iss);
				fs.Close();
			}
		}
		public FileHashDB(short _BlockSize, bool KEYS)
		{
			if(KEYS) Map = new Hashtable();
			ObjectReadCB = new OnReadHandler(ReadObjectSink);
			string FullFilePath = Guid.NewGuid().ToString();
			tfc = new TempFileCollection();
			tfc.AddFile(FullFilePath,false);

			MaxBlockSize = _BlockSize;
			fstream = new FileStream(FullFilePath,FileMode.Create,FileAccess.ReadWrite,FileShare.Read);
			DBFileInfo = new FileInfo(FullFilePath);
			DeleteBuffer = new byte[MaxBlockSize];
			DeleteCB = new AsyncCallback(DeleteSink);
			ClearCB = new AsyncCallback(ClearSink);
			MemoryStream ms = new MemoryStream();
			byte[] x = BitConverter.GetBytes((short)0);
			ms.Write(x,0,x.Length);
			ms.Write(x,0,x.Length);
			ClearBlock = ms.ToArray();
			ms.Close();

			IndexFilePath = DBFileInfo.FullName.Substring(0,DBFileInfo.FullName.Length-DBFileInfo.Extension.Length) + ".idx";
			tfc.AddFile(IndexFilePath,false);

			// Initialize File Header
			byte[] BlockSize = BitConverter.GetBytes((short)MaxBlockSize);
			byte[] bUpdateID = BitConverter.GetBytes(UpdateID);
			byte[] eVersion = BitConverter.GetBytes(EngineVersion);
			SeekLock.WaitOne();
			fstream.Seek(0,SeekOrigin.Begin);
			InitializeCounter = 3;
			fstream.BeginWrite(BlockSize,0,2,new AsyncCallback(InitHeaderSink),null);
			fstream.BeginWrite(bUpdateID,0,4,new AsyncCallback(InitHeaderSink),null);
			fstream.BeginWrite(eVersion,0,4,new AsyncCallback(InitHeaderSink),null);
			SeekLock.ReleaseMutex();
			if(Interlocked.Decrement(ref InitializeCounter)==0)
			{
				if(OnReady!=null) OnReady(this,null);
			}
			
		}
		public FileHashDB(string FullFilePath, short _BlockSize, OnReadyHandler rCB)
		{
			Map = new Hashtable();
			ObjectReadCB = new OnReadHandler(ReadObjectSink);
			OnReady += rCB;
			MaxBlockSize = _BlockSize;
			fstream = new FileStream(FullFilePath,FileMode.OpenOrCreate,FileAccess.ReadWrite,FileShare.Read);
			DBFileInfo = new FileInfo(FullFilePath);
			DeleteBuffer = new byte[MaxBlockSize];
			DeleteCB = new AsyncCallback(DeleteSink);
			ClearCB = new AsyncCallback(ClearSink);
			MemoryStream ms = new MemoryStream();
			byte[] x = BitConverter.GetBytes((short)0);
			ms.Write(x,0,x.Length);
			ms.Write(x,0,x.Length);
			ClearBlock = ms.ToArray();
			ms.Close();

			if(fstream.Length>0)
			{
				// Already Exists
				SeekLock.WaitOne();
				fstream.Seek(0,SeekOrigin.Begin);
				fstream.BeginRead(InitBuffer,0,10,new AsyncCallback(InitSink),null);
				SeekLock.ReleaseMutex();
			}
			else
			{
				IndexFilePath = DBFileInfo.FullName.Substring(0,DBFileInfo.FullName.Length-DBFileInfo.Extension.Length) + ".idx";

				// Initialize File Header
				byte[] BlockSize = BitConverter.GetBytes((short)MaxBlockSize);
				byte[] bUpdateID = BitConverter.GetBytes(UpdateID);
				byte[] eVersion = BitConverter.GetBytes(EngineVersion);
				SeekLock.WaitOne();
				fstream.Seek(0,SeekOrigin.Begin);
				InitializeCounter = 3;
				fstream.BeginWrite(BlockSize,0,2,new AsyncCallback(InitHeaderSink),null);
				fstream.BeginWrite(bUpdateID,0,4,new AsyncCallback(InitHeaderSink),null);
				fstream.BeginWrite(eVersion,0,4,new AsyncCallback(InitHeaderSink),null);
				SeekLock.ReleaseMutex();
				if(Interlocked.Decrement(ref InitializeCounter)==0)
				{
					if(OnReady!=null) OnReady(this,null);
				}
			}
		}

		private void InitHeaderSink(IAsyncResult result)
		{
			fstream.EndWrite(result);
			if(Interlocked.Decrement(ref InitializeCounter)==0)
			{
				if(OnReady!=null) OnReady(this, null);
			}
		}

		private void InitSink(IAsyncResult result)
		{
			fstream.EndRead(result);
			short BlockSize = BitConverter.ToInt16(InitBuffer,0);
			int eVersion = BitConverter.ToInt32(InitBuffer,6);
			UpdateID = BitConverter.ToInt32(InitBuffer,2);

			if(eVersion!=EngineVersion)
			{
				if(OnReady!=null) OnReady(this, new ApplicationException("FileHashDB v" + EngineVersion.ToString() + " trying to open file v" + eVersion.ToString()));
				return;
			}

			MaxBlockSize = BlockSize;
			++UpdateID;

			byte[] uid = BitConverter.GetBytes(UpdateID);

			SeekLock.WaitOne();
			fstream.Seek(2,SeekOrigin.Begin);
			fstream.BeginWrite(uid,0,4,new AsyncCallback(WriteInitSink),null);
			SeekLock.ReleaseMutex();
		}
		private void WriteInitSink(IAsyncResult result)
		{
			fstream.EndWrite(result);

			IndexFilePath = DBFileInfo.FullName.Substring(0,DBFileInfo.FullName.Length-DBFileInfo.Extension.Length) + ".idx";

			if(File.Exists(IndexFilePath))
			{
				InitThread = new Thread(new ThreadStart(InitThreadEntry));
				InitThread.Start();
			}
			else
			{
				OnIndexRebuilt += new OnIndexRebuiltHandler(InitIndexSink);
				RebuildIndex();
			}
		}
		private void InitThreadEntry()
		{
			FileStream istream = new FileStream(IndexFilePath,FileMode.Open);
			BinaryFormatter bf = new BinaryFormatter();
			IndexStruct iss = (IndexStruct)bf.Deserialize(istream);
			istream.Close();

			Map = iss.IndexHashTable;
			if(iss.UpdateID==UpdateID)
			{
				if(OnReady!=null) OnReady(this,null);
			}
			else
			{
				Map.Clear();
				OnIndexRebuilt += new OnIndexRebuiltHandler(InitIndexSink);
				RebuildIndex();
			}
		}

		private void InitIndexSink(FileHashDB sender)
		{
			OnIndexRebuilt -= new OnIndexRebuiltHandler(InitIndexSink);
			if(OnReady!=null) OnReady(this, null);
		}
		public void RebuildIndex()
		{
			Indexer = 1;
			IndexBuffer = new byte[(int)MaxBlockSize];
			EPtr = (int)fstream.Length;
			int CPtr = 10;
			AsyncCallback CB = new AsyncCallback(BlockSink);
			while(CPtr+MaxBlockSize<=EPtr)
			{
				SeekLock.WaitOne();
				fstream.Seek((long)CPtr,SeekOrigin.Begin);
				Interlocked.Increment(ref Indexer);
				fstream.BeginRead(IndexBuffer,0,(int)MaxBlockSize,CB,CPtr);
				SeekLock.ReleaseMutex();
				CPtr += (int)MaxBlockSize;
			}
			if(Interlocked.Decrement(ref Indexer)==0)
			{
				if(OnIndexRebuilt!=null) OnIndexRebuilt(this);
			}
		}
		private void BlockSink(IAsyncResult result)
		{
			fstream.EndRead(result);
			int offset = (int)result.AsyncState;

			short BlockSize = BitConverter.ToInt16(IndexBuffer,0);
			short KeySize = BitConverter.ToInt16(IndexBuffer,2);

			if(BlockSize==0 && KeySize==0)
			{
				// Free Block
				AddFreeBlock(offset);
			}
			else
			{
				if(KeySize!=0)
				{
					// Parent Block
					UTF8Encoding U = new UTF8Encoding();
					string key = U.GetString(IndexBuffer,4,KeySize);
					lock(Map)
					{
						this.Map[key] = offset;
					}
				}
			}
			if(Interlocked.Decrement(ref Indexer)==0)
			{
				if(OnIndexRebuilt!=null) OnIndexRebuilt(this);
			}
		}
		public void DeleteRecord(int Position)
		{
			SeekLock.WaitOne();
			fstream.Seek((long)Position,SeekOrigin.Begin);
			fstream.BeginRead(DeleteBuffer,0,MaxBlockSize,DeleteCB,Position);
			SeekLock.ReleaseMutex();
		}
		public void DeleteRecord(string KEY)
		{
			int offset = -1;
			lock(Map)
			{
				if(Map.ContainsKey(KEY))
				{
					offset = (int)Map[KEY];
					Map.Remove(KEY);
				}
				else
				{
					return;
				}
			}

			SeekLock.WaitOne();
			fstream.Seek((long)offset,SeekOrigin.Begin);
			fstream.BeginRead(DeleteBuffer,0,MaxBlockSize,DeleteCB,offset);
			SeekLock.ReleaseMutex();
		}
		private void DeleteSink(IAsyncResult result)
		{
			fstream.EndRead(result);
			int offset = (int)result.AsyncState;
			int NextBlock = BitConverter.ToInt32(DeleteBuffer,MaxBlockSize-4);
			if(NextBlock!=-1)
			{
				SeekLock.WaitOne();
				fstream.Seek((long)offset,SeekOrigin.Begin);
				fstream.BeginRead(DeleteBuffer,0,MaxBlockSize,DeleteCB,NextBlock);
				SeekLock.ReleaseMutex();
			}
			SeekLock.WaitOne();
			fstream.Seek((long)offset,SeekOrigin.Begin);
			fstream.BeginWrite(ClearBlock,0,ClearBlock.Length,ClearCB,offset);
			SeekLock.ReleaseMutex();
		}
		private void ClearSink(IAsyncResult result)
		{
			fstream.EndWrite(result);
			int offset = (int)result.AsyncState;
			AddFreeBlock(offset);
		}

		public void ReadRecord(int Position, object Tag, OnReadHandler CB)
		{
			DataBlock d = new DataBlock(this,Position,new DataBlock.OnReadHandler(ReadDataBlockSink),new object[3]{CB,"",Tag});
			PendingReadTable[d] = d;
		}

		public void ReadRecord(string KEY, object Tag, OnReadHandler CB)
		{
			object kobj = Map[KEY];
			if(kobj==null)
			{
				if(CB!=null) CB(this,KEY,new byte[0],Tag);
				return;
			}
			else
			{
				int i = (int)kobj;
				DataBlock d = new DataBlock(this,i,new DataBlock.OnReadHandler(ReadDataBlockSink),new object[3]{CB,KEY,Tag});
				PendingReadTable[d] = d;
			}
		}
		private void ReadDataBlockSink(DataBlock sender, string key, byte[] ReadBuffer, object Tag)
		{
			PendingReadTable.Remove(sender);
			object[] State = (object[])Tag;
			OnReadHandler CB = (OnReadHandler)State[0];
			string KEY = (string)State[1];
			object _Tag = State[2];
			if(CB!=null) CB(this,KEY,ReadBuffer,_Tag);
		}

		public int WriteObject(string KEY, object ObjectToWrite, object Tag, FileHashDB.OnWriteHandler CB)
		{
			MemoryStream ostream = new MemoryStream();
			formatter.Serialize(ostream,ObjectToWrite);
			byte[] buffer = ostream.ToArray();
			return(WriteRecord(KEY,buffer,0,buffer.Length,Tag,CB));
		}
		public void ReadObject(int Position, object Tag, FileHashDB.OnReadObjectHandler CB)
		{
			ReadRecord(Position,new Object[2]{Tag,CB},ObjectReadCB);
		}
		public void ReadObject(string KEY, object Tag, FileHashDB.OnReadObjectHandler CB)
		{
			ReadRecord(KEY,new object[2]{Tag,CB},ObjectReadCB);
		}
		private void ReadObjectSink(FileHashDB sender, string KEY, byte[] buffer, object Tag)
		{
			object RetObj = null;
			if(buffer.Length>0)
			{
				MemoryStream ostream = new MemoryStream(buffer);
				RetObj = formatter.Deserialize(ostream);
				object[] state = (object[])Tag;
				if(state[1]!=null)
				{
					((OnReadObjectHandler)state[1])(this,KEY,RetObj,state[0]);
				}
			}
		}

		public int WriteRecord(string KEY, byte[] buffer, int offset, int count, object Tag, FileHashDB.OnWriteHandler WriteCB)
		{
			DataBlock d = GetFreeBlock();
			d._Tag = WriteCB;
			d.OnWrite += new DataBlock.OnWriteHandler(WriteSink);
			long h = 0;

			lock(CBMap)
			{
				h = handle++;
				CBMap[h] = new object[2]{KEY,Tag};
			}

			if(Map!=null)
			{
				lock(Map)
				{
					Map[KEY] = d.OFFSET;
				}
			}
			
			d.Write(KEY,buffer,offset,count,h);
			return(d.OFFSET);
		}

		private void WriteSink(DataBlock sender, object Tag)
		{
			object[] d = null;

			lock(CBMap)
			{
				d = (object[])CBMap[Tag];
				//Done
				CBMap.Remove(Tag);
			}
			if(sender._Tag!=null)
			{
				((OnWriteHandler)sender._Tag)(this,(string)d[0],d[1]);
				sender._Tag = null;
			}
		}

		public ICollection Keys
		{
			get
			{
				return(Map.Keys);
			}
		}



		public DataBlock GetFreeBlock()
		{
			DataBlock RetVal = null;
			lock(FreeBlockList)
			{
				if(FreeBlockList.Count!=0)
				{
					RetVal = new DataBlock((int)FreeBlockList.GetKey(0),this);
					FreeBlockList.RemoveAt(0);
				}
				else
				{
					RetVal = new DataBlock(EPtr,this);
					EPtr += 512;
				}
			}
			return(RetVal);
		}
		public void AddFreeBlock(DataBlock b)
		{
			AddFreeBlock(b.OFFSET);
		}
		public void AddFreeBlock(int offset)
		{
			lock(FreeBlockList)
			{
				if(offset+(int)MaxBlockSize==EPtr)
				{
					int coffset = offset;
					bool done = false;
					while(!done)
					{
						if(coffset-MaxBlockSize>=10)
						{
							if(FreeBlockList.Contains(coffset-(int)MaxBlockSize))
							{
								FreeBlockList.Remove(coffset-(int)MaxBlockSize);
								coffset -= (int)MaxBlockSize;
							}
							else
							{
								done = true;
							}
						}
						else
						{
							done = true;
						}
					}
					EPtr = coffset;
					fstream.SetLength((long)coffset);
					return;
				}
				FreeBlockList.Add(offset,offset);
			}
		}
	}
}
