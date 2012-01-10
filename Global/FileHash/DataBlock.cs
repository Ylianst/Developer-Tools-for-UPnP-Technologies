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

namespace OpenSource.FileHashDB
{
	/// <summary>
	/// Summary description for DataBlock.
	/// </summary>
	public class DataBlock
	{
		private int PendingWrites = 1;
		//private int BytesWritten = 0;

		private string KEY = "";

		internal int OFFSET = -1;
		internal int NEXTBLOCK = -1;
		internal object _Tag = null;

		private FileHashDB f;
		private byte[] buffer = new byte[512];
		private MemoryStream mstream = new MemoryStream();

		public delegate void OnReadHandler(DataBlock sender, string key, byte[] ReadBuffer, object Tag);
		public event OnReadHandler OnRead;

		public delegate void OnWriteHandler(DataBlock sender, object Tag);
		public event OnWriteHandler OnWrite;

		private AsyncCallback CB;

		internal DataBlock(int BlockOffset, FileHashDB WRITER_sender)
		{
			CB = new AsyncCallback(WriteBlockSink);
			f = WRITER_sender;
			OFFSET = BlockOffset;
		}

		public void Write(string KEY, byte[] DataBuffer, int offset, int count, object Tag)
		{			
			UTF8Encoding U = new UTF8Encoding();
			byte[] key = U.GetBytes(KEY);
			byte[] keysize = BitConverter.GetBytes((short)key.Length);
			byte[] Fill = null;

			int DataIDX = 0;
			int DataWritten = 0;
			int DataWrite = 0;

			byte[] BlockSize = null;
			byte[] NextBlock = null;

			DataBlock OtherBlock = null;

			f.SeekLock.WaitOne();
			f.fstream.Seek(OFFSET+2,SeekOrigin.Begin);
			Interlocked.Increment(ref PendingWrites);
			f.fstream.BeginWrite(keysize,0,2,CB,Tag);
			Interlocked.Increment(ref PendingWrites);
			f.fstream.BeginWrite(key,0,key.Length,CB,Tag);
			
			if(key.Length + 8 < f.MaxBlockSize)
			{
				// Can fit data here
				if(DataBuffer.Length>(f.MaxBlockSize-key.Length-8))
				{
					DataWrite = f.MaxBlockSize-key.Length-8;
				}
				else
				{
					DataWrite = DataBuffer.Length;
				}
				Interlocked.Increment(ref PendingWrites);
				f.fstream.BeginWrite(DataBuffer,0,DataWrite,CB,Tag);
				DataWritten += DataWrite;
				DataIDX += DataWrite;
				Fill = new Byte[(int)f.MaxBlockSize-(DataWrite+key.Length)-8];
				if(Fill.Length>0)
				{
					Interlocked.Increment(ref PendingWrites);
					f.fstream.BeginWrite(Fill,0,Fill.Length,CB,Tag);
				}
				BlockSize = BitConverter.GetBytes((short)(DataWrite + key.Length));
			}
			else
			{
				BlockSize = BitConverter.GetBytes((short)key.Length);
			}

			f.fstream.Seek((long)OFFSET,SeekOrigin.Begin);
			Interlocked.Increment(ref PendingWrites);
			f.fstream.BeginWrite(BlockSize,0,2,CB,Tag);
			if(DataBuffer.Length-DataWrite==0)
			{
				// No need for more packets;
				NextBlock = BitConverter.GetBytes((int)-1);
				f.fstream.Seek((long)OFFSET + (long)(f.MaxBlockSize-4),SeekOrigin.Begin);
				Interlocked.Increment(ref PendingWrites);
				f.fstream.BeginWrite(NextBlock,0,4,CB,Tag);
				f.SeekLock.ReleaseMutex();
				if(Interlocked.Decrement(ref PendingWrites)==0)
				{
					if(OnWrite!=null) OnWrite(this,Tag);
				}
				return;
			}

			// Need More Blocks
			keysize = BitConverter.GetBytes((short)0);
			f.SeekLock.ReleaseMutex();
			while(DataBuffer.Length-DataWritten!=0)
			{
				OtherBlock = f.GetFreeBlock();
				NextBlock = BitConverter.GetBytes(OtherBlock.OFFSET);
				f.SeekLock.WaitOne();
				f.fstream.Seek((long)OFFSET+(long)(f.MaxBlockSize-4),SeekOrigin.Begin);
				Interlocked.Increment(ref PendingWrites);
				f.fstream.BeginWrite(NextBlock,0,4,CB,Tag);

				OFFSET = OtherBlock.OFFSET;
				f.fstream.Seek((long)OFFSET+2,SeekOrigin.Begin);
				Interlocked.Increment(ref PendingWrites);
				f.fstream.BeginWrite(keysize,0,2,CB,Tag);

				if(DataBuffer.Length-DataWritten>(int)(f.MaxBlockSize-8))
				{
					DataWrite = f.MaxBlockSize-8;
				}
				else
				{
					DataWrite = DataBuffer.Length-DataWritten;
				}
				Interlocked.Increment(ref PendingWrites);
				f.fstream.BeginWrite(DataBuffer,DataIDX,DataWrite,CB,Tag);
				DataWritten += DataWrite;
				DataIDX += DataWrite;
				Fill = new byte[(int)f.MaxBlockSize-DataWrite-8];
				if(Fill.Length>0)
				{
					Interlocked.Increment(ref PendingWrites);
					f.fstream.BeginWrite(Fill,0,Fill.Length,CB,Tag);
				}
				BlockSize = BitConverter.GetBytes((short)DataWrite);
				f.fstream.Seek((long)OFFSET,SeekOrigin.Begin);
				Interlocked.Increment(ref PendingWrites);
				f.fstream.BeginWrite(BlockSize,0,2,CB,Tag);
				if(DataBuffer.Length-DataWritten==0)
				{
					f.fstream.Seek((long)OFFSET+(long)(f.MaxBlockSize-4),SeekOrigin.Begin);
					NextBlock = BitConverter.GetBytes((int)-1);
					Interlocked.Increment(ref PendingWrites);
					f.fstream.BeginWrite(NextBlock,0,4,CB,Tag);
				}
				f.SeekLock.ReleaseMutex();
			}

			if(Interlocked.Decrement(ref PendingWrites)==0)
			{
				if(OnWrite!=null) OnWrite(this,Tag);
			}
		}

		internal DataBlock(FileHashDB READER_sender, int offset, OnReadHandler rCB, object Tag)
		{
			OnRead += rCB;
			_Tag = Tag;
			CB = new AsyncCallback(ReadBlockSink);
			f = READER_sender;
			
			f.SeekLock.WaitOne();
			f.fstream.Seek((long)offset,SeekOrigin.Begin);
			f.fstream.BeginRead(buffer,0,512,CB,offset);
			f.SeekLock.ReleaseMutex();
		}

		private void WriteBlockSink(IAsyncResult result)
		{
			f.fstream.EndWrite(result);
			
			if(Interlocked.Decrement(ref PendingWrites)==0)
			{
				if(this.OnWrite!=null) OnWrite(this,result.AsyncState);
			}
		}

		private void ReadBlockSink(IAsyncResult result)
		{
			f.fstream.EndRead(result);
			int offset = (int)result.AsyncState;

			short BlockSize = BitConverter.ToInt16(buffer,0);
			int NextBlock = BitConverter.ToInt32(buffer,f.MaxBlockSize-4);
			short KeySize = BitConverter.ToInt16(buffer,2);

			if(KeySize!=0)
			{
				UTF8Encoding U = new UTF8Encoding();
				KEY = U.GetString(buffer,4,KeySize);
			}

			mstream.Write(buffer,4+KeySize,(int)BlockSize-(int)KeySize);
			if(NextBlock!=-1)
			{
				f.SeekLock.WaitOne();
				f.fstream.Seek((long)NextBlock,SeekOrigin.Begin);
				f.fstream.BeginRead(buffer,0,512,CB,NextBlock);
				f.SeekLock.ReleaseMutex();
			}
			else
			{
				mstream.Flush();
				if(this.OnRead!=null) OnRead(this,KEY,mstream.ToArray(),_Tag);
			}
		}
	}
}
