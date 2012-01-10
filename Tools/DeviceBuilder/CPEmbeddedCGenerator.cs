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
	public class CPEmbeddedCGenerator : CodeGenerator
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

		Hashtable SequenceTable = new Hashtable();
		Hashtable ChoiceTable = new Hashtable();
		int SequenceCounter=0;
		int ChoiceCounter=0;



		private string UseInfoString = "";
		private string UseSystem = "";
		public string SampleApplication = "";

		public PLATFORMS Platform = PLATFORMS.POSIX;
		public SUBTARGETS SubTarget = SUBTARGETS.NONE;
		public LANGUAGES Language = LANGUAGES.C;

		public ArrayList AllServices = new ArrayList();
		public int WinSock = 0;

		private static string cl = "\r\n";
		public string CodeNewLine 
		{
			get {return cl;}
			set {cl = value;}
		}

		private string pc_methodPrefix = "UPnP";
		private string pc_methodLibPrefix = "SAMPLE";
		private string pc_methodPrefixDef = "UPnP";
		private static CodeProcessor PrivateClassDeclarations;
		private static CodeProcessor PublicClassDeclarations;

		public ArrayList CustomTagList = new ArrayList();

		public CPEmbeddedCGenerator(ServiceGenerator.StackConfiguration Config, string SampleApplication):base(Config)
		{
			this.SampleApplication = SampleApplication;

			switch(Config.newline)
			{
				case ServiceGenerator.NEWLINETYPE.CRLF:
					cl = "\r\n";
					break;
				case ServiceGenerator.NEWLINETYPE.LF:
					cl = "\n";
					break;
			}
			switch(Config.TargetPlatform)
			{
				case ServiceGenerator.PLATFORMS.MICROSTACK_POSIX:
					this.Platform = PLATFORMS.POSIX;
					this.SubTarget = SUBTARGETS.NONE;
					break;
				case ServiceGenerator.PLATFORMS.MICROSTACK_WINSOCK1:
					this.Platform = PLATFORMS.WINDOWS;
					this.SubTarget = SUBTARGETS.NONE;
					this.WinSock = 1;
					break;
				case ServiceGenerator.PLATFORMS.MICROSTACK_WINSOCK2:
					this.Platform = PLATFORMS.WINDOWS;
					this.SubTarget = SUBTARGETS.NONE;
					this.WinSock = 2;
					break;
				case ServiceGenerator.PLATFORMS.MICROSTACK_POCKETPC:
					this.Platform = PLATFORMS.WINDOWS;
					this.SubTarget = SUBTARGETS.PPC2003;
					this.WinSock = 1;
					break;
			}
		}

		private void AddLicense(CodeProcessor cs,string filename) 
		{
			string l = License;
			l = l.Replace("<FILE>",filename);
			cs.Append(l);
		}

		private void AddAllServices(UPnPDevice device) 
		{
			foreach (UPnPService s in device.Services) AllServices.Add(s);
			foreach (UPnPDevice d in device.EmbeddedDevices) AddAllServices(d);
		}


		public static void BuildComplexTypeSerializer_Header(CodeProcessor cs, SortedList SortedServiceList, string pc_methodPrefix)
		{
			cs.Append(cl);
			cs.Comment("Complex Type Serializers");
			IDictionaryEnumerator en = SortedServiceList.GetEnumerator();
			while(en.MoveNext())
			{
				UPnPService S = (UPnPService)en.Value;
				foreach(UPnPComplexType CT in S.GetComplexTypeList())
				{
					cs.Append("char* "+pc_methodPrefix+"Serialize_"+CT.Name_LOCAL+"(struct "+CT.Name_LOCAL+" *c);"+cl);
				}
			}
			cs.Append(cl);
		}

		public static void BuildComplexTypeSerializer_Collection(ref int VarIndex, ref int SeqIndex, ref int ChoIndex, Hashtable SequenceTable,Hashtable ChoiceTable,UPnPComplexType CT,CodeProcessor cs,SortedList SortedServiceList,string pc_methodPrefix, UPnPComplexType.ItemCollection ic)
		{
//			foreach(UPnPComplexType.ItemCollection ec in ic.NestedCollections)
//			{
//				BuildComplexTypeSerializer_Collection(ref VarIndex,SequenceTable,ChoiceTable,CT,cs,SortedServiceList,pc_methodPrefix,ec);
//			}

			if (ic.GetType()==typeof(UPnPComplexType.Sequence))
			{
				++VarIndex;
				++SeqIndex;
				cs.Append("	char* Var"+VarIndex.ToString()+" = "+pc_methodPrefix+"Serialize_"+CT.Name_LOCAL+"_SEQUENCE"+SequenceTable[ic].ToString()+"(c->_sequence_"+SeqIndex.ToString()+");"+cl);
			}
			else if (ic.GetType()==typeof(UPnPComplexType.Choice))
			{
				++VarIndex;
				++ChoIndex;
				cs.Append("	char* Var"+VarIndex.ToString()+" = "+pc_methodPrefix+"Serialize_"+CT.Name_LOCAL+"_SEQUENCE"+SequenceTable[ic].ToString()+"(c->_choice_"+ChoIndex.ToString()+");"+cl);
			}
			else
			{
				// Generic Item Collection
			}
		}
		public static void BuildComplexTypeSerializer_Container(ref int VarIndex, ref int SeqIndex, ref int ChoIndex, Hashtable SequenceTable,Hashtable ChoiceTable,UPnPComplexType CT,CodeProcessor cs, SortedList SortedServiceList, string pc_methodPrefix, UPnPComplexType.GenericContainer gc)
		{
			foreach(UPnPComplexType.ItemCollection ic in gc.Collections)
			{
				BuildComplexTypeSerializer_Collection(ref VarIndex, ref SeqIndex, ref ChoIndex, SequenceTable,ChoiceTable,CT,cs,SortedServiceList,pc_methodPrefix, ic);
			}
		}
		public static void BuildComplexTypeSerializer_ContentData(CodeProcessor cs,int VarX,UPnPComplexType.ContentData cd,UPnPComplexType CT,string pc_methodPrefix)
		{
			if (cd.TypeNS=="http://www.w3.org/2001/XMLSchema")
			{
				// XSD Simple Type
				cs.Append("	if ((Var"+VarX.ToString()+" = (char*)malloc(");
				cs.Append(((cd.Name.Length*2)+5).ToString()+"+");
				switch(cd.Type)
				{
					case "unsignedByte":
					case "unsignedInt":
					case "unsignedShort":
					case "unsignedLong":
						cs.Append(ulong.MaxValue.ToString().Length.ToString());
						break;
					case "boolean":
					case "int":
					case "integer":
					case "positiveInteger":
					case "negativeInteger":
					case "nonNegativeInteger":
					case "nonPositiveInteger":
					case "long":
					case "short":
						cs.Append(long.MaxValue.ToString().Length.ToString());
						break;
					case "decimal":
					case "float":
					case "double":
						cs.Append(double.MaxValue.ToString().Length.ToString());
						break;
					default:
						cs.Append("(int)strlen(s->"+cd.Name+")");
						break;
				}
                cs.Append(")) == NULL) ILIBCRITICALEXIT(254);" + cl);
				cs.Append("	len += sprintf(Var"+VarX.ToString()+",\"<"+cd.Name+">%");
				switch(cd.Type)
				{
					case "unsignedByte":
					case "unsignedInt":
					case "unsignedShort":
					case "unsignedLong":
						cs.Append("u");
						break;
					case "boolean":
					case "int":
					case "integer":
					case "positiveInteger":
					case "negativeInteger":
					case "nonNegativeInteger":
					case "nonPositiveInteger":
					case "long":
					case "short":
						cs.Append("d");
						break;
					case "decimal":
					case "float":
					case "double":
						cs.Append("f");
						break;
					default:
						cs.Append("s");
						break;
				}
				cs.Append("</"+cd.Name+">\",s->"+cd.Name+");"+cl);
			}
			else
			{
				// XSD User Defined
				cs.Append("	temp = "+pc_methodPrefix+"Serialize_"+cd.Type+"(s->"+cd.Name+");"+cl);
                cs.Append("	if ((Var" + VarX.ToString() + " = (char*)malloc(" + (6 + cd.Name.Length * 2).ToString() + " + (int)strlen(temp))) == NULL) ILIBCRITICALEXIT(254);" + cl);
//				cs.Append("	Var"+VarX.ToString()+" = "+pc_methodPrefix+"Serialize_"+cd.Type+"(s->"+cd.Name+");"+cl);
				cs.Append("	len += sprintf(Var"+VarX.ToString()+",\"<"+cd.Name+">%s</"+cd.Name+">\",temp);"+cl);
				cs.Append("	free(temp);"+cl);
//				cs.Append("	len += (int)strlen(Var"+VarX.ToString()+");"+cl);
			}
		}
		public static void BuildComplexTypeSerializer_Sequence(CodeProcessor cs, UPnPComplexType CT, Hashtable SeqTable, UPnPComplexType.Sequence s,string pc_methodPrefix)
		{
			int VarX=0;
			int SeqX=0;
			int ChoX=0;

			cs.Append("char* "+pc_methodPrefix+"Serialize_"+CT.Name_LOCAL+"_SEQUENCE"+SeqTable[s].ToString()+"(struct SEQUENCE_"+SeqTable[s].ToString()+" *s)"+cl);
			cs.Append("{"+cl);
			cs.Append("	int len=0;"+cl);
			cs.Append("	char* RetVal;"+cl);

			foreach(UPnPComplexType.ItemCollection ic in s.NestedCollections)
			{
				if (ic.GetType()==typeof(UPnPComplexType.Sequence))
				{
					++SeqX;
					++VarX;
					cs.Append("	char* Var"+VarX.ToString()+" = "+pc_methodPrefix+"Serialize_"+CT.Name_LOCAL+"_SEQUENCE"+SeqTable[ic].ToString()+"(s->_sequence_"+SeqX.ToString()+");"+cl);
				}
				else if (ic.GetType()==typeof(UPnPComplexType.Choice))
				{
					++ChoX;
					++VarX;
				}
			}
			for(int i=1;i<=s.Items.Length;++i)
			{
				cs.Append("	char* Var"+(VarX+i).ToString()+";"+cl);
			}
			foreach(UPnPComplexType.ContentData cd in s.Items)
			{
				if (cd.TypeNS!="http://www.w3.org/2001/XMLSchema")
				{
					cs.Append("	char* temp;"+cl);
					break;
				}
			}
			cs.Append(cl);
			for(int i=1;i<=VarX;++i)
			{
				cs.Append("	len += (int)strlen(Var"+i.ToString()+");"+cl);
			}
			cs.Append(cl);
			foreach(UPnPComplexType.ContentData cd in s.Items)
			{
				++VarX;
				BuildComplexTypeSerializer_ContentData(cs,VarX,cd,CT,pc_methodPrefix);
			}

            cs.Append("	if ((RetVal = (char*)malloc(len + 1)) == NULL) ILIBCRITICALEXIT(254);" + cl);
			cs.Append("	len = 0;"+cl);
			for(int i=1;i<=VarX;++i)
			{
				cs.Append("	len += sprintf(RetVal + len, \"%s\", Var"+i.ToString()+");"+cl);
			}
			for(int i=1;i<=VarX;++i)
			{
				cs.Append("	free(Var"+i.ToString()+");"+cl);
			}

			cs.Append("	return(RetVal);"+cl);
			cs.Append("}"+cl);
		}
		public static void BuildComplexTypeSerializer_Sequence_FREE_PreDeclaration(CodeProcessor cs, UPnPComplexType CT, Hashtable SeqTable, UPnPComplexType.Sequence s,string pc_methodPrefix)
		{
//			int VarX=0;
//			int SeqX=0;
//			int ChoX=0;

			cs.Append("void "+pc_methodPrefix+"FREE_"+CT.Name_LOCAL+"_SEQUENCE"+SeqTable[s].ToString()+"(struct SEQUENCE_"+SeqTable[s].ToString()+" *s);"+cl);

//			foreach(UPnPComplexType.ItemCollection ic in s.NestedCollections)
//			{
//				if (ic.GetType()==typeof(UPnPComplexType.Sequence))
//				{
//					++SeqX;
//					++VarX;
//					cs.Append("void "+pc_methodPrefix+"FREE_"+CT.Name_LOCAL+"_SEQUENCE"+SeqTable[ic].ToString()+"(s->_sequence_"+SeqX.ToString()+");"+cl);
//				}
//				else if (ic.GetType()==typeof(UPnPComplexType.Choice))
//				{
//					++ChoX;
//					++VarX;
//					//ToDo: Add code for Choice
//				}
//			}
		}
		public static void BuildComplexTypeSerializer_Sequence_PreDeclaration(CodeProcessor cs, UPnPComplexType CT, Hashtable SeqTable, UPnPComplexType.Sequence s,string pc_methodPrefix, string pc_methodLibPrefix)
		{
//			int VarX=0;
//			int SeqX=0;
//			int ChoX=0;

			cs.Append("char* "+pc_methodPrefix+"Serialize_"+CT.Name_LOCAL+"_SEQUENCE"+SeqTable[s].ToString()+"(struct SEQUENCE_"+SeqTable[s].ToString()+" *s);"+cl);

			//			foreach(UPnPComplexType.ItemCollection ic in s.NestedCollections)
			//			{
			//				if (ic.GetType()==typeof(UPnPComplexType.Sequence))
			//				{
			//					++SeqX;
			//					++VarX;
			//					cs.Append("void "+pc_methodPrefix+"FREE_"+CT.Name_LOCAL+"_SEQUENCE"+SeqTable[ic].ToString()+"(s->_sequence_"+SeqX.ToString()+");"+cl);
			//				}
			//				else if (ic.GetType()==typeof(UPnPComplexType.Choice))
			//				{
			//					++ChoX;
			//					++VarX;
			//					//ToDo: Add code for Choice
			//				}
			//			}
		}
		public static void BuildComplexTypeSerializer_Sequence_FREE(CodeProcessor cs, UPnPComplexType CT, Hashtable SeqTable, UPnPComplexType.Sequence s,string pc_methodPrefix)
		{
			int VarX=0;
			int SeqX=0;
			int ChoX=0;

			cs.Append("void "+pc_methodPrefix+"FREE_"+CT.Name_LOCAL+"_SEQUENCE"+SeqTable[s].ToString()+"(struct SEQUENCE_"+SeqTable[s].ToString()+" *s)"+cl);
			cs.Append("{"+cl);

			foreach(UPnPComplexType.ItemCollection ic in s.NestedCollections)
			{
				if (ic.GetType()==typeof(UPnPComplexType.Sequence))
				{
					++SeqX;
					++VarX;
					cs.Append("	"+pc_methodPrefix+"FREE_"+CT.Name_LOCAL+"_SEQUENCE"+SeqTable[ic].ToString()+"(s->_sequence_"+SeqX.ToString()+");"+cl);
				}
				else if (ic.GetType()==typeof(UPnPComplexType.Choice))
				{
					++ChoX;
					++VarX;
					//ToDo: Add code for Choice
				}
			}

			foreach(UPnPComplexType.ContentData cd in s.Items)
			{
				++VarX;
				//BuildComplexTypeSerializer_ContentData(cs,VarX,cd,CT,pc_methodPrefix);
				if (cd.TypeNS!="http://www.w3.org/2001/XMLSchema")
				{
					cs.Append("	"+pc_methodPrefix+"FREE_"+cd.Type+"(s->"+cd.Name+");"+cl);
				}
			}
			cs.Append("	free(s);"+cl);
			cs.Append("}"+cl);
		}
		public static void BuildComplexTypeSerializer_FREE(CodeProcessor cs, UPnPComplexType CT, Hashtable SeqTable,string pc_methodPrefix)
		{
			int VarX=0;
			int SeqX=0;
			int ChoX=0;

			cs.Append("void "+pc_methodPrefix+"FREE_"+CT.Name_LOCAL+"(struct "+CT.Name_LOCAL+" *s)"+cl);
			cs.Append("{"+cl);
			foreach(UPnPComplexType.GenericContainer gc in CT.Containers)
			{
				foreach(UPnPComplexType.ItemCollection ic in gc.Collections)
				{
					if (ic.GetType()==typeof(UPnPComplexType.Sequence))
					{
						++SeqX;
						++VarX;
						cs.Append("	"+pc_methodPrefix+"FREE_"+CT.Name_LOCAL+"_SEQUENCE"+SeqTable[ic].ToString()+"(s->_sequence_"+SeqX.ToString()+");"+cl);
					}
					else if (ic.GetType()==typeof(UPnPComplexType.Choice))
					{
						++ChoX;
						++VarX;
						//ToDo: Add code for Choice
					}	
					else
					{
						//ToDO: Add case for generic Item Collections
					}
				}
			}
			cs.Append("	free(s);"+cl);
			cs.Append("}"+cl);
		}
		public static void BuildComplexTypeSerializer(Hashtable SequenceTable, Hashtable ChoiceTable,CodeProcessor cs, SortedList SortedServiceList, string pc_methodPrefix, string pc_methodLibPrefix)
		{
			IDictionaryEnumerator en = SortedServiceList.GetEnumerator();
			while(en.MoveNext())
			{
				UPnPService S = (UPnPService)en.Value;

				#region Free Resources from Serializer
					#region PreDeclarations
				//
				// We need predeclarators
				//
				foreach(UPnPComplexType CT in S.GetComplexTypeList())
				{
					cs.Append("void "+pc_methodPrefix+"FREE_"+CT.Name_LOCAL+"(struct "+CT.Name_LOCAL+" *);"+cl);
					IDictionaryEnumerator d = ((Hashtable)SequenceTable[CT]).GetEnumerator();
					while(d.MoveNext())
					{
						UPnPComplexType.Sequence s = (UPnPComplexType.Sequence)d.Key;
						BuildComplexTypeSerializer_Sequence_FREE_PreDeclaration(cs,CT,(Hashtable)SequenceTable[CT],s,pc_methodPrefix);
					}
					d = ((Hashtable)ChoiceTable[CT]).GetEnumerator();
					while(d.MoveNext())
					{
						//ToDo: Build the Choice Serializer Free
					}
				}
				cs.Append(cl);
				foreach(UPnPComplexType CT in S.GetComplexTypeList())
				{
					IDictionaryEnumerator d = ((Hashtable)SequenceTable[CT]).GetEnumerator();
					while(d.MoveNext())
					{
						UPnPComplexType.Sequence s = (UPnPComplexType.Sequence)d.Key;
						BuildComplexTypeSerializer_Sequence_PreDeclaration(cs,CT,(Hashtable)SequenceTable[CT],s,pc_methodPrefix,pc_methodLibPrefix);
					}
					d = ((Hashtable)ChoiceTable[CT]).GetEnumerator();
					while(d.MoveNext())
					{
						//ToDo: Build the Choice Serializer Free
					}
				}
				cs.Append(cl);
				#endregion
					#region The Implementation
				foreach(UPnPComplexType CT in S.GetComplexTypeList())
				{
					//int VarIndex = 0;
					//int SeqIndex = 0;
					//int ChoIndex = 0;

					BuildComplexTypeSerializer_FREE(cs,CT,(Hashtable)SequenceTable[CT],pc_methodPrefix);

					IDictionaryEnumerator d = ((Hashtable)SequenceTable[CT]).GetEnumerator();
					while(d.MoveNext())
					{
						UPnPComplexType.Sequence s = (UPnPComplexType.Sequence)d.Key;
						BuildComplexTypeSerializer_Sequence_FREE(cs,CT,(Hashtable)SequenceTable[CT],s,pc_methodPrefix);
					}
					d = ((Hashtable)ChoiceTable[CT]).GetEnumerator();
					while(d.MoveNext())
					{
						//ToDo: Build the Choice Serializer Free
					}
				}
				#endregion
				#endregion

				#region Serializer
				foreach(UPnPComplexType CT in S.GetComplexTypeList())
				{
					int VarIndex = 0;
					int SeqIndex = 0;
					int ChoIndex = 0;


					IDictionaryEnumerator d = ((Hashtable)SequenceTable[CT]).GetEnumerator();
					while(d.MoveNext())
					{
						UPnPComplexType.Sequence s = (UPnPComplexType.Sequence)d.Key;
						BuildComplexTypeSerializer_Sequence(cs,CT,(Hashtable)SequenceTable[CT],s,pc_methodPrefix);
					}
					d = ((Hashtable)ChoiceTable[CT]).GetEnumerator();
					while(d.MoveNext())
					{

						//ToDo: Build the Choice Serializer
					}

					cs.Append("char* "+pc_methodPrefix+"Serialize_"+CT.Name_LOCAL+"(struct "+CT.Name_LOCAL+" *c)"+cl);
					cs.Append("{"+cl);
					cs.Append("	int len = 0;"+cl);
					cs.Append("	int memReq = 1;"+cl);
					cs.Append("	char* RetVal;"+cl+cl);
					foreach(UPnPComplexType.GenericContainer gc in CT.Containers)
					{
						BuildComplexTypeSerializer_Container(ref VarIndex,ref SeqIndex, ref ChoIndex,SequenceTable,ChoiceTable,CT,cs,SortedServiceList,pc_methodPrefix,gc);
					}

					for(int i=0;i<VarIndex;++i)
					{
						cs.Append("	memReq += (int)strlen(Var"+VarIndex.ToString()+");"+cl);
					}
                    cs.Append("	if ((RetVal = (char*)malloc(memReq)) == NULL) ILIBCRITICALEXIT(254);" + cl);
					for(int i=0;i<VarIndex;++i)
					{
						cs.Append("	len += sprintf(RetVal + len, \"%s\", Var"+VarIndex.ToString()+");"+cl);
					}
					for(int i=0;i<VarIndex;++i)
					{
						cs.Append("	free(Var"+VarIndex.ToString()+");"+cl);
					}
						
					#region Depracated Code
							//					int minReq = 1; // Memory reserved for string terminator
							//					foreach(UPnPComplexType.Field f in CT.GetFields())
							//					{
							//						minReq += 5; // < >  </ >
							//						minReq += (CT.Name_LOCAL.Length*2);
							//					}
							//					cs.Append("	int MemoryRequirement="+minReq.ToString()+";"+cl);
//					cs.Append("	int index=0;"+cl);
//					foreach(UPnPComplexType.Field f in CT.GetFields())
//					{
//						if (f.MaxOccurs=="unbounded" || (f.MaxOccurs!="unbounded" && f.MaxOccurs!="" && int.Parse(f.MaxOccurs)>0))
//						{
//							//cs.Append("	char* temp;"+cl);
//							cs.Append("	int i;"+cl);
//							break;
//						}
//					}
//					foreach(UPnPComplexType.Field f in CT.GetFields())
//					{
//						if (f.TypeNS!="http://www.w3.org/2001/XMLSchema")
//						{
//							// Contains other Complex Types
//							if (f.MaxOccurs=="unbounded" || (f.MaxOccurs!="unbounded" && f.MaxOccurs!="" && int.Parse(f.MaxOccurs)>0))
//							{
//								// Array
//								cs.Append("	char **_"+f.Name+" = (char**)malloc(sizeof(char*)*c->"+f.Name+"Length);"+cl);
//							}
//							else
//							{
//								// Non-Array
//								cs.Append("	char *_"+f.Name+";"+cl);
//							}
//
//						}
//					}
//					cs.Append(cl);
//					cs.Comment("Tabulate Memory Requirement");
//					foreach(UPnPComplexType.Field f in CT.GetFields())
//					{
//						if (f.MaxOccurs=="unbounded" || (f.MaxOccurs!="unbounded" && f.MaxOccurs!="" && int.Parse(f.MaxOccurs)>0))
//						{
//							//Array
//							cs.Append("	for(i=0;i<c->"+f.Name+"Length;++i)"+cl);
//							cs.Append("	{"+cl);
//							if (f.TypeNS=="http://www.w3.org/2001/XMLSchema")
//							{
//								//Primitive
//								switch(f.Type)
//								{
//									case "boolean":
//										cs.Append("	MemoryRequirement += 1; // bool value for "+f.Name+cl);
//										break;
//									case "int":
//										cs.Append("	MemoryRequirement += "+int.MaxValue.ToString().Length.ToString() + "; // max length for 32 bit integer for "+f.Name+cl);
//										break;
//									default:
//										// assumed to be string
//										cs.Append("	MemoryRequirement += (int)strlen(c->"+f.Name+"[i]);"+cl);
//										break;
//								}
//							}
//							else
//							{
//								//Complex
//								cs.Append("	_"+f.Name+"[i] = "+pc_methodPrefix+"Serialize_"+f.Type+"(c->"+f.Name+"[i]);"+cl);
//								cs.Append("	MemoryRequirement += (int)strlen(_"+f.Name+"[i]);"+cl);
//							}
//							cs.Append("	}"+cl);
//						}
//						else
//						{
//							//Non-Array
//							if (f.TypeNS=="http://www.w3.org/2001/XMLSchema")
//							{
//								// Primitive Fields
//								switch(f.Type)
//								{
//									case "boolean":
//										cs.Append("	MemoryRequirement += 1; // bool value for "+f.Name+cl);
//										break;
//									case "int":
//										cs.Append("	MemoryRequirement += "+int.MaxValue.ToString().Length.ToString() + "; // max length for 32 bit integer for "+f.Name+cl);
//										break;
//									default:
//										// assumed to be string
//										cs.Append("	MemoryRequirement += (int)strlen(c->"+f.Name+");"+cl);
//										break;
//								}
//							}
//							else
//							{
//								// Complex Fields
//								cs.Append("	_"+f.Name+" = "+pc_methodPrefix+"Serialize_"+f.Type+"(c->"+f.Name+");"+cl);
//								cs.Append("	MemoryRequirement += (int)strlen(_"+f.Name+");"+cl);
//							}
//						}
//					}
//					cs.Append(cl);
//					cs.Append("	RetVal = (char*)malloc(MemoryRequirement);"+cl);
//					cs.Append(cl);
//					foreach(UPnPComplexType.Field f in CT.GetFields())
//					{
//						if (f.MaxOccurs=="unbounded" || (f.MaxOccurs!="unbounded" && f.MaxOccurs!="" && int.Parse(f.MaxOccurs)>0))
//						{
//							// Array
//							cs.Append("	for(i=0;i<c->"+f.Name+"Length;++i)"+cl);
//							cs.Append("	{"+cl);
//							if (f.TypeNS=="http://www.w3.org/2001/XMLSchema")
//							{
//								//Primitive
//								cs.Append("	index += sprintf(RetVal+index,\"<"+f.Name+">%");
//								switch(f.Type)
//								{
//									case "int":
//									case "boolean":
//										cs.Append("d");
//										break;
//									default:
//										cs.Append("s");
//										break;
//								}
//								cs.Append("</"+f.Name+">\",c->"+f.Name+"[i]);"+cl);
//							}
//							else
//							{
//								//Complex
//								cs.Append("	index += sprintf(RetVal+index,\"<"+f.Name+">%s</"+f.Name+">\",_"+f.Name+"[i]);"+cl);
//								cs.Append("	free(_"+f.Name+"[i]);"+cl);
//							}
//							cs.Append("	}"+cl);
//						}
//						else
//						{
//							// Non-Array
//							if (f.TypeNS=="http://www.w3.org/2001/XMLSchema")
//							{
//								// Primitive
//								cs.Append("	index += sprintf(RetVal+index,\"<"+f.Name+">%");
//								switch(f.Type)
//								{
//									case "int":
//									case "boolean":
//										cs.Append("d");
//										break;
//									default:
//										cs.Append("s");
//										break;
//								}
//								cs.Append("</"+f.Name+">\",c->"+f.Name+");"+cl);
//							}
//							else
//							{
//								// Complex
//								cs.Append("	_"+f.Name+" = "+pc_methodPrefix+"Serialize_"+f.Type+"(c->"+f.Name+");"+cl);
//								cs.Append("	index += sprintf(RetVal+index,\"<"+f.Name+">%s</"+f.Name+">\",_"+f.Name+");"+cl);
//								cs.Append("	free(_"+f.Name+");"+cl);
//							}
//						}
//					}
					#endregion

					cs.Append("	return(RetVal);"+cl);
					cs.Append("}"+cl);
				}
				#endregion


			}
		}

		private void Build_UPnPStructs_Defs(CodeProcessor cs,UPnPDevice device,Hashtable serviceNames)
		{
			cs.Append("struct UPnPDevice"+cl);
			cs.Append("{"+cl);
			cs.Append("	void* CP;"+cl);
			cs.Append("	char* DeviceType;"+cl);
			cs.Append("	char* UDN;"+cl);
			cs.Append(cl);
			cs.Append("	char* LocationURL;"+cl);
			cs.Append("	char* PresentationURL;"+cl);
			cs.Append("	char* FriendlyName;"+cl);
			cs.Append("	char* ManufacturerName;"+cl);
			cs.Append("	char* ManufacturerURL;"+cl);
			cs.Append("	char* ModelName;"+cl);
			cs.Append("	char* ModelDescription;"+cl);
			cs.Append("	char* ModelNumber;"+cl);
			cs.Append("	char* ModelURL;"+cl);
			cs.Append(cl);
			cs.Append("	int SCPDError;"+cl);
			cs.Append("	int SCPDLeft;"+cl);
			cs.Append("	int ReferenceCount;"+cl);
			cs.Append("	int ReferenceTiedToEvents;"+cl);
			cs.Append("	char* InterfaceToHost;"+cl);
			cs.Append("	int CacheTime;"+cl);
			cs.Append("	void *Tag;"+cl);
			cs.Append(cl);
			cs.Append("	struct UPnPDevice *Parent;"+cl);
			cs.Append("	struct UPnPDevice *EmbeddedDevices;"+cl);
			cs.Append("	struct UPnPService *Services;"+cl);
			cs.Append("	struct UPnPDevice *Next;"+cl);
			cs.Append("};"+cl);
			cs.Append(cl);

			cs.Append("struct UPnPService"+cl);
			cs.Append("{"+cl);
			cs.Append("	char* ServiceType;"+cl);
			cs.Append("	char* ServiceId;"+cl);
			cs.Append("	char* ControlURL;"+cl);
			cs.Append("	char* SubscriptionURL;"+cl);
			cs.Append("	char* SCPDURL;"+cl);
			cs.Append("	char* SubscriptionID;"+cl);
			cs.Append(cl);
			cs.Append("	struct UPnPAction *Actions;"+cl);
			cs.Append("	struct UPnPStateVariable *Variables;"+cl);
			cs.Append("	struct UPnPDevice *Parent;"+cl);
			cs.Append("	struct UPnPService *Next;"+cl);
			cs.Append("};"+cl);
			cs.Append(cl);

			cs.Append("struct UPnPStateVariable"+cl);
			cs.Append("{"+cl);
			cs.Append("	struct UPnPStateVariable *Next;"+cl);
			cs.Append("	struct UPnPService *Parent;"+cl);
			cs.Append(cl);
			cs.Append("	char* Name;"+cl);
			cs.Append("	char **AllowedValues;"+cl);
			cs.Append("	int NumAllowedValues;"+cl);
			cs.Append("	char* Min;"+cl);
			cs.Append("	char* Max;"+cl);
			cs.Append("	char* Step;"+cl);
			cs.Append("};"+cl);
			cs.Append(cl);

			cs.Append("struct UPnPAction"+cl);
			cs.Append("{"+cl);
			cs.Append("	char* Name;"+cl);
			cs.Append("	struct UPnPAction *Next;"+cl);
			cs.Append("};"+cl);
			cs.Append(cl);

			cs.Append("struct UPnPAllowedValue"+cl);
			cs.Append("{"+cl);
			cs.Append("	struct UPnPAllowedValue *Next;"+cl);
			cs.Append(cl);
			cs.Append("	char* Value;"+cl);
			cs.Append("};"+cl);
			cs.Append(cl);

			/*
			DText parser = new DText();
			parser.ATTRMARK = ":";

			foreach(UPnPDevice d in device.EmbeddedDevices)
			{
				Build_UPnPStructs_Defs(cs,d,serviceNames);
			}

			foreach(UPnPService s in device.Services)
			{
				string name = (string)serviceNames[s];

				cs.Append("struct UPnPService_"+name+cl);
				cs.Append("{"+cl);
				cs.Append("	char* ControlURL;"+cl);
				cs.Append("	char* SubscriptionURL;"+cl);
				cs.Append("	char* XML;"+cl);
				cs.Append("	int XMLLength;"+cl);
				cs.Append(cl);
				cs.Append("	void* PrivateData;"+cl);
				cs.Append("};"+cl);
			}

			parser[0] = device.DeviceURN;
			cs.Append("struct UPnPDevice_"+parser[4]+ cl);
			cs.Append("{"+cl);
			cs.Append("	char* LocationURL;"+cl);
			cs.Append("	char* BaseURL;"+cl);
			cs.Append("	char* FriendlyName;"+cl);
			cs.Append("	char* UDN;"+cl);
			cs.Append("	char* XML;"+cl);
			cs.Append("	int XMLLength;"+cl);
			cs.Append(cl);
			cs.Append("	void* PrivateData;"+cl);
			foreach(UPnPService s in device.Services)
			{
				string name = (string)serviceNames[s];
				cs.Append("	struct UPnPService_"+name+" *"+name+";"+cl);
			}
			cs.Append("};"+cl);	
			cs.Append(cl);
			*/
		}
		public void BuildIsLegacy(CodeProcessor cs)
		{
			cs.Append("int "+this.pc_methodPrefix+"IsLegacyDevice(struct packetheader *header)"+cl);
			cs.Append("{"+cl);
			cs.Append("	struct packetheader_field_node *f;"+cl);
			cs.Append("	struct parser_result_field *prf;"+cl);
			cs.Append("	struct parser_result *r, *r2;"+cl);
			cs.Append("	int Legacy=1;"+cl);

			cs.Append("	// Check version of Device"+cl);
			cs.Append("	f = header->FirstField;"+cl);
			cs.Append("	while(f!=NULL)"+cl);
			cs.Append("	{"+cl);
			cs.Append("		if (f->FieldLength == 6 && strncasecmp(f->Field, \"SERVER\", 6) == 0)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			// Check UPnP version of the Control Point which invoked us"+cl);
			cs.Append("			r = ILibParseString(f->FieldData, 0, f->FieldDataLength, \" \", 1);"+cl);
			cs.Append("			prf = r->FirstResult;"+cl);
			cs.Append("			while(prf != NULL)"+cl);
			cs.Append("			{"+cl);
			cs.Append("				if (prf->datalength > 5 && memcmp(prf->data, \"UPnP/\", 5) == 0)"+cl);
			cs.Append("				{"+cl);
			cs.Append("					r2 = ILibParseString(prf->data + 5, 0, prf->datalength - 5, \".\", 1);"+cl);
			cs.Append("					r2->FirstResult->data[r2->FirstResult->datalength] = 0;"+cl);
			cs.Append("					r2->LastResult->data[r2->LastResult->datalength] = 0;"+cl);
			cs.Append("					if (atoi(r2->FirstResult->data) == 1 && atoi(r2->LastResult->data) > 0)"+cl);
			cs.Append("					{"+cl);
			cs.Append("						Legacy = 0;"+cl);
			cs.Append("					}"+cl);
			cs.Append("					ILibDestructParserResults(r2);"+cl);
			cs.Append("				}"+cl);
			cs.Append("				prf = prf->NextResult;"+cl);
			cs.Append("			}"+cl);
			cs.Append("			ILibDestructParserResults(r);"+cl);
			cs.Append("		}"+cl);
			cs.Append("		if (Legacy)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			f = f->NextField;"+cl);
			cs.Append("		}"+cl);
			cs.Append("		else"+cl);
			cs.Append("		{"+cl);
			cs.Append("			break;"+cl);
			cs.Append("		}"+cl);
			cs.Append("	}"+cl);
			cs.Append("	return(Legacy);"+cl);
			cs.Append("}"+cl);
		}
		public void CP_GenerateHeaderFile(UPnPDevice device,DirectoryInfo outputDirectory, Hashtable serviceNames)
		{
			SortedList SL = new SortedList();
			IDictionaryEnumerator en = serviceNames.GetEnumerator();

			while(en.MoveNext())
			{
				SL[en.Value] = en.Key;
			}

			PrivateClassDeclarations = new CodeProcessor(new StringBuilder(),this.Language == LANGUAGES.CPP);
			PublicClassDeclarations = new CodeProcessor(new StringBuilder(),this.Language == LANGUAGES.CPP);
			CodeProcessor cs = new CodeProcessor(new StringBuilder(),this.Language == LANGUAGES.CPP);
			cs.NewLine = this.CodeNewLine;
			cs.ClassDefinitions = PrivateClassDeclarations;
			cs.PublicClassDefinitions = PublicClassDeclarations;
			PrivateClassDeclarations.CodeTab = Indent;
			PublicClassDeclarations.CodeTab = Indent;
			cs.CodeTab = Indent;

			AddLicense(cs,pc_methodPrefix+"ControlPoint.h");

			cs.Append(cl);
			cs.Append("#ifndef __"+pc_methodPrefix+"ControlPoint__"+cl);
			cs.Append("#define __"+pc_methodPrefix+"ControlPoint__"+cl);
			cs.Append(cl);
			cs.Append("#include \"UPnPControlPointStructs.h\""+cl);
			//Build_UPnPStructs_Defs(cs,device,serviceNames);

			DText Parser = new DText();
			Parser.ATTRMARK = ":";
			Parser[0] = device.DeviceURN;
			cs.Append(cl);


			EmbeddedCGenerator.BuildComplexTypeDefinitionsAndHeaders(SL,cs,SequenceTable,ChoiceTable,ref SequenceCounter,ref ChoiceCounter,this.pc_methodPrefix,this.pc_methodLibPrefix);
//			BuildComplexTypeSerializer_Header(cs,SL,this.pc_methodPrefix);
//			EmbeddedCGenerator.BuildComplexTypeParser_Header(cs,SL,this.pc_methodPrefix,this.pc_methodLibPrefix);
			

			cs.Append(cl);
			cs.Append("	void "+pc_methodPrefixDef+"AddRef(struct UPnPDevice *device);"+cl);
			cs.Append("	void "+pc_methodPrefixDef+"Release(struct UPnPDevice *device);"+cl);
			cs.Append(cl);
			cs.Append("	struct UPnPDevice* "+pc_methodPrefix+"GetDevice1(struct UPnPDevice *device,int index);"+cl);
			cs.Append("	int "+pc_methodPrefix+"GetDeviceCount(struct UPnPDevice *device);"+cl);
			cs.Append("	struct UPnPDevice* "+pc_methodPrefixDef+"GetDeviceAtUDN(void *v_CP,char* UDN);"+cl);
			cs.Append(cl);
			cs.Append("	void PrintUPnPDevice(int indents, struct UPnPDevice *device);"+cl);
			cs.Append(cl);
			cs.Append("void *"+this.pc_methodPrefix+"CreateControlPoint(void *Chain, void(*A)(struct UPnPDevice*),void(*R)(struct UPnPDevice*));"+cl);
			cs.Append("void "+this.pc_methodPrefix+"_CP_IPAddressListChanged(void *CPToken);"+cl);
		
			cs.Append("struct UPnPDevice* "+pc_methodPrefixDef+"GetDeviceEx(struct UPnPDevice *device, char* DeviceType, int start,int number);"+cl);
			cs.Append("int "+pc_methodPrefixDef+"HasAction(struct UPnPService *s, char* action);"+cl);
			cs.Append("void "+pc_methodPrefix+"UnSubscribeUPnPEvents(struct UPnPService *service);"+cl);
			cs.Append("void "+pc_methodPrefix+"SubscribeForUPnPEvents(struct UPnPService *service, void(*callbackPtr)(struct UPnPService* service,int OK));"+cl);
			cs.Append("struct UPnPService *"+pc_methodPrefix+"GetService(struct UPnPDevice *device, char* ServiceName, int length);"+cl);

			en = SL.GetEnumerator();
			while(en.MoveNext())
			{
				string name = (string)en.Key;
				cs.Append("	struct UPnPService *"+pc_methodPrefixDef+"GetService_"+name+"(struct UPnPDevice *device);"+cl);
			}		

			// Generate event callback methods
			cs.Append(cl);
			IDictionaryEnumerator en2 = SL.GetEnumerator();
			while(en2.MoveNext())
			{
				UPnPService s = (UPnPService)en2.Value;
				string name = (string)en2.Key;
				foreach(UPnPStateVariable variable in s.GetStateVariables()) 
				{
					if (variable.SendEvent == true) 
					{
						cs.Append("extern void (*"+pc_methodPrefix+"EventCallback_"+name+"_"+variable.Name+")(struct UPnPService* Service,");
						cs.Append(ToCType(variable.GetNetType().FullName)+" "+variable.Name);
						if (variable.GetNetType()==typeof(byte[]))
						{
							cs.Append(", int " + variable.Name+"Length");
						}
						cs.Append(");"+cl);
					}
				}
			}
			cs.Append(cl);

			en.Reset();
			while(en.MoveNext())
			{
				UPnPService s = (UPnPService)en.Value;
				string name = (string)en.Key;
				foreach(UPnPAction A in s.Actions)
				{
					cs.Append("	void "+pc_methodPrefix+"Invoke_"+name+"_"+A.Name+"(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService*,int ErrorCode,void *user");
					if (A.HasReturnValue)
					{
						if (A.GetRetArg().RelatedStateVar.ComplexType==null)
						{
							// NonComplex
							cs.Append(","+ToCType(A.GetRetArg().RelatedStateVar.GetNetType().FullName)+" "+A.GetRetArg().Name);
							if (A.GetRetArg().RelatedStateVar.GetNetType()==typeof(byte[]))
							{
								cs.Append(", int " + A.GetRetArg().Name+"Length");
							}
						}
						else
						{
							// Complex
							cs.Append(", struct " + A.GetRetArg().RelatedStateVar.ComplexType.Name_LOCAL+" *"+A.GetRetArg().Name);
						}
					}
					foreach(UPnPArgument Arg in A.Arguments)
					{
						if (!Arg.IsReturnValue && Arg.Direction=="out")
						{
							if (Arg.RelatedStateVar.ComplexType==null)
							{
								//NonComplex
								cs.Append(","+ToCType(Arg.RelatedStateVar.GetNetType().FullName)+" "+Arg.Name);
								if (Arg.RelatedStateVar.GetNetType()==typeof(byte[]))
								{
									cs.Append(", int " + Arg.Name+"Length");
								}
							}
							else
							{
								//Complex
								cs.Append(", struct "+Arg.RelatedStateVar.ComplexType.Name_LOCAL+" *"+Arg.Name);
							}
						}
					}
					cs.Append(")");
					cs.Append(",void* _user");
					foreach(UPnPArgument Arg in A.Arguments)
					{
						if (Arg.Direction=="in")
						{
							if (Arg.RelatedStateVar.ComplexType==null)
							{
								//NonComplex
								cs.Append(", "+ToCType(Arg.RelatedStateVar.GetNetType().FullName)+" "+Arg.Name);
								if (Arg.RelatedStateVar.GetNetType()==typeof(byte[]))
								{
									cs.Append(", int " + Arg.Name+"Length");
								}	
							}
							else
							{
								//Complex
								cs.Append(", struct "+Arg.RelatedStateVar.ComplexType.Name_LOCAL+" *"+Arg.Name);
							}
						}
					}
					cs.Append(");"+cl);
				}
			}

			cs.Append(cl);
			cs.Append("#endif"+cl);

			StreamWriter writer5 = File.CreateText(outputDirectory.FullName + "\\"+pc_methodPrefix+"ControlPoint.h");
			writer5.Write(cs.ToString());
			writer5.Close();
		}

		private int CalculateLength(string s)
		{
			int ln = s.Length;
			int c = 0;

			while(s.IndexOf("\\r",c)!=-1)
			{
				c = s.IndexOf("\\r",c)+1;
				ln-=1;
			}

			c = 0;
			while(s.IndexOf("\\n",c)!=-1)
			{
				c = s.IndexOf("\\n",c)+1;
				ln-=1;
			}

			c = 0;
			while(s.IndexOf("\\0",c)!=-1)
			{
				c = s.IndexOf("\\0",c)+1;
				ln-=1;
			}

			c = 0;
			while(s.IndexOf("\\\"",c)!=-1)
			{
				c = s.IndexOf("\\\"",c)+1;
				ln-=1;
			}

			return(ln);
		}
		private void BuildEventParser(CodeProcessor cs, UPnPService s, string urn, string name)
		{
			bool eventedservice = false;
			foreach(UPnPStateVariable V in s.GetStateVariables())
			{
				if (V.SendEvent) eventedservice = true;
			}

			if (urn.StartsWith("urn:schemas-upnp-org:"))
			{
				// Standard Service
				urn = urn.Substring(urn.LastIndexOf(":")+1);
			}
			else
			{
				// Proprietary Service
				urn = urn.Replace(":","_");
				urn = urn.Replace("-","_");
				urn = urn.Replace(".","_");
			}

			cs.Append("void "+this.pc_methodPrefix+urn+"_EventSink(char* buffer, int bufferlength, struct UPnPService *service)"+cl);
			cs.Append("{"+cl);
			cs.Append("	struct "+this.pc_methodLibPrefix+"XMLNode *xml,*rootXML;"+cl);
			if (eventedservice == true) 
			{
				cs.Append("	char *tempString;"+cl);
				cs.Append("	int tempStringLength;"+cl);
			}
			cs.Append("	int flg,flg2;"+cl);
			cs.Append(cl);

			bool placeLong = false;
			bool placeULong = false;
			foreach(UPnPStateVariable V in s.GetStateVariables())
			{
				if (V.SendEvent)
				{
					cs.Append("	"+ToCType(V.GetNetType().FullName)+" "+V.Name+" = 0;"+cl);
					if (V.GetNetType()==typeof(byte[]))
					{
						cs.Append("	int " + V.Name+"Length;"+cl);
					}
					switch(V.GetNetType().FullName)
					{
						case "System.SByte":
						case "System.Int16":
						case "System.Int32":
							placeLong = true;
							break;
						case "System.Byte":
						case "System.UInt16":
						case "System.UInt32":
							placeULong = true;
							break;
					}
				}
			}
			if (placeLong == true) cs.Append(" long TempLong;"+cl);
			if (placeULong == true) cs.Append(" unsigned long TempULong;"+cl);

			cs.Append(cl);
			cs.Comment("Parse SOAP");
			cs.Append("	rootXML = xml = "+this.pc_methodLibPrefix+"ParseXML(buffer, 0, bufferlength);"+cl);
			cs.Append("	"+this.pc_methodLibPrefix+"ProcessXMLNodeList(xml);"+cl);
			cs.Append(cl);

			cs.Append("while(xml != NULL)"+cl);
			cs.Append("{"+cl);
			cs.Append("	if (xml->NameLength == 11 && memcmp(xml->Name, \"propertyset\", 11)==0)"+cl);
			cs.Append("	{"+cl);
			cs.Append("		if (xml->Next->StartTag != 0)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			flg = 0;"+cl);
			cs.Append("			xml = xml->Next;"+cl);
			cs.Append("			while(flg==0)"+cl);
			cs.Append("			{"+cl);
			cs.Append("				if (xml->NameLength == 8 && memcmp(xml->Name, \"property\", 8)==0)"+cl);
			cs.Append("				{"+cl);
			cs.Append("					xml = xml->Next;"+cl);
			cs.Append("					flg2 = 0;"+cl);
			cs.Append("					while(flg2==0)"+cl);
			cs.Append("					{"+cl);

			foreach(UPnPStateVariable V in s.GetStateVariables())
			{
				if (V.SendEvent)
				{
					cs.Append("					if (xml->NameLength == "+V.Name.Length.ToString()+" && memcmp(xml->Name, \""+V.Name+"\", "+V.Name.Length.ToString()+") == 0)"+cl);
					cs.Append("					{"+cl);
					cs.Append("						tempStringLength = "+this.pc_methodLibPrefix+"ReadInnerXML(xml,&tempString);"+cl);
					
					if (ToCType(V.GetNetType().FullName)=="char*")
					{
						cs.Append("							tempString[tempStringLength] = '\\0';"+cl);
						cs.Append("							"+V.Name+" = tempString;"+cl);
					}
					else
					{
						switch(V.GetNetType().FullName)
						{
							case "System.Boolean":
								cs.Append("							if (tempStringLength >= 5 && strncasecmp(tempString, \"false\", 5)==0)"+cl);
								cs.Append("							{"+cl);
								cs.Append("								"+V.Name+" = 0;"+cl);
								cs.Append("							}"+cl);
								cs.Append("							else if (tempStringLength >= 1 && strncmp(tempString, \"0\", 1)==0)"+cl);
								cs.Append("							{"+cl);
								cs.Append("								"+V.Name+" = 0;"+cl);
								cs.Append("							}"+cl);
								cs.Append("							else"+cl);
								cs.Append("							{"+cl);
								cs.Append("								"+V.Name+" = 1;"+cl);
								cs.Append("							}"+cl);
								break;
							case "System.DateTime":
								cs.Append("							tempString[tempStringLength] = '\\0';"+cl);
								cs.Append("							"+V.Name+" = "+this.pc_methodLibPrefix+"Time_Parse(tempString);"+cl);
								break;
							case "System.SByte":
							case "System.Int16":
							case "System.Int32":
								cs.Append("							if ("+this.pc_methodLibPrefix+"GetLong(tempString, tempStringLength, &TempLong)==0)"+cl);
								cs.Append("							{"+cl);
								cs.Append("								"+V.Name+" = ("+ToCType(V.GetNetType().FullName)+") TempLong;"+cl);
								cs.Append("							}"+cl);
								break;
							case "System.Byte":
							case "System.UInt16":
							case "System.UInt32":
								cs.Append("							if ("+this.pc_methodLibPrefix+"GetULong(tempString, tempStringLength, &TempULong)==0)"+cl);
								cs.Append("							{"+cl);
								cs.Append("								"+V.Name+" = ("+ToCType(V.GetNetType().FullName)+") TempULong;"+cl);
								cs.Append("							}"+cl);
								break;
							case "System.Byte[]":
								cs.Append("							"+V.Name+"Length="+this.pc_methodLibPrefix+"Base64Decode(tempString, tempStringLength, &"+V.Name+");"+cl);
								break;
						}
					}

					//Trigger Event
					cs.Append("							if ("+this.pc_methodPrefix+"EventCallback_"+name+"_"+V.Name+" != NULL)"+cl);
					cs.Append("							{"+cl);
					cs.Append("								"+this.pc_methodPrefix+"EventCallback_"+name+"_"+V.Name+"(service,"+V.Name);
					if (V.GetNetType()==typeof(byte[]))
					{
						cs.Append(","+V.Name+"Length");
					}
					cs.Append(");"+cl);
					cs.Append("							}"+cl);
					if (V.GetNetType()==typeof(byte[]))
					{
						cs.Append("							free("+V.Name+");"+cl);
					}
					cs.Append("						}"+cl);
				}
			}

			cs.Append("						if (xml->Peer!=NULL)"+cl);
			cs.Append("						{"+cl);
			cs.Append("							xml = xml->Peer;"+cl);
			cs.Append("						}"+cl);
			cs.Append("						else"+cl);
			cs.Append("						{"+cl);
			cs.Append("							flg2 = -1;"+cl);
			cs.Append("							xml = xml->Parent;"+cl);
			cs.Append("						}"+cl);
			cs.Append("					}"+cl);
			cs.Append("				}"+cl);
			cs.Append("				if (xml->Peer!=NULL)"+cl);
			cs.Append("				{"+cl);
			cs.Append("					xml = xml->Peer;"+cl);
			cs.Append("				}"+cl);
			cs.Append("				else"+cl);
			cs.Append("				{"+cl);
			cs.Append("					flg = -1;"+cl);
			cs.Append("					xml = xml->Parent;"+cl);
			cs.Append("				}"+cl);
			cs.Append("			}"+cl);
			cs.Append("		}"+cl);
			cs.Append("	}"+cl);
			cs.Append("	xml = xml->Peer;"+cl);
			cs.Append("}"+cl);
			cs.Append(cl);
			cs.Append(""+this.pc_methodLibPrefix+"DestructXMLNodeList(rootXML);"+cl);
			cs.Append("}"+cl);
		}


		private void SCPDJunk(CodeProcessor cs)
		{
//			cs.Append("								AddRequest(HTTP, p,&addr, &HTTP_Sink_"+name+", TempService);"+cl);	

			cs.Append("								if (SCPDURLLength >= 7 && memcmp(SCPDURL, \"http://\", 6) == 0)"+cl);
			cs.Append("								{"+cl);
			cs.Comment("Explicit");
			cs.Append("									"+pc_methodLibPrefix+"ParseUri(SCPDURL, &IP, &Port, &Path);"+cl);
			cs.Append("									p = "+pc_methodPrefix+"BuildPacket(IP, Port, Path, \"GET\");"+cl);
			cs.Append("									free(Path);"+cl);
			cs.Append("								}"+cl);
			cs.Append("								else"+cl);
			cs.Append("								{"+cl);
			cs.Comment("Relative");
			cs.Append("									"+pc_methodLibPrefix+"ParseUri(BaseURL, &IP, &Port, &Path);"+cl);
			cs.Append("									free(Path);"+cl);
			cs.Append("									if (memcmp(SCPDURL, \"/\", 1)!=0)"+cl);
			cs.Append("									{"+cl);
            cs.Append("										if ((Path = (char*)malloc(SCPDURLLength + 2)) == NULL) ILIBCRITICALEXIT(254);" + cl);
			cs.Append("										memcpy(Path, \"/\", 1);"+cl);
			cs.Append("										memcpy(Path + 1, SCPDURL, SCPDURLLength);"+cl);
			cs.Append("										Path[SCPDURLLength + 1] = '\\0';"+cl);
			cs.Append("										p = "+pc_methodPrefix+"BuildPacket(IP, Port, Path, \"GET\");"+cl);
			cs.Append("										free(Path);"+cl);
			cs.Append("									}"+cl);
			cs.Append("									else"+cl);
			cs.Append("									{"+cl);
			cs.Append("										p = "+pc_methodPrefix+"BuildPacket(IP, Port, SCPDURL, \"GET\");"+cl);
			cs.Append("									}"+cl);
			cs.Append("								}"+cl);
			cs.Append(cl);
			cs.Append("								memset((char *)&addr, 0, sizeof(addr));"+cl);
			cs.Append("								addr.sin_family = AF_INET;"+cl);
			cs.Append("								addr.sin_addr.s_addr = inet_addr(IP);"+cl);
			cs.Append("								addr.sin_port = htons(Port);"+cl);
			cs.Append("								free(IP);"+cl);
		}

		public override bool Generate(UPnPDevice[] devices, DirectoryInfo outputDirectory)
		{
			bool CPOK = false;
			string WS = "";
			StreamWriter W;
			bool RetVal = false;
			ServiceGenerator.Configuration DeviceConf;

			#region PocketPC 2003 Specific
			if (SampleApplication!=null && this.SubTarget==SUBTARGETS.PPC2003)
			{
				#region SampleProjectDlg.h

				WS = SourceCodeRepository.Get_CP_SampleProjectDlg_h();

				#region Write to disk
				W = File.CreateText(outputDirectory.FullName + "\\SampleProjectDlg.h");
				W.Write(WS);
				W.Close();
				#endregion

				#endregion
				#region SampleProject.cpp
				WS = SourceCodeRepository.Get_CP_SampleProject_cpp();
				#region Write to disk
				W = File.CreateText(outputDirectory.FullName + "\\SampleProject.cpp");
				W.Write(WS);
				W.Close();
				#endregion

				#endregion
				#region SampleProject.h
				WS = SourceCodeRepository.Get_CP_SampleProject_h();
				#region Write to disk
				W = File.CreateText(outputDirectory.FullName + "\\SampleProject.h");
				W.Write(WS);
				W.Close();
				#endregion
				#endregion
				#region newres.h
				WS = SourceCodeRepository.Get_CP_newres_h();
				#region Write to disk
				W = File.CreateText(outputDirectory.FullName + "\\newres.h");
				W.Write(WS);
				W.Close();
				#endregion
				#endregion
				#region resource.h
				WS = SourceCodeRepository.Get_CP_resource_h();
				#region Write to disk
				W = File.CreateText(outputDirectory.FullName + "\\resource.h");
				W.Write(WS);
				W.Close();
				#endregion
				#endregion
				#region SampleProject.rc
				WS = SourceCodeRepository.Get_CP_SampleProject_rc();
				#region Write to disk
				W = File.CreateText(outputDirectory.FullName + "\\SampleProject.rc");
				W.Write(WS);
				W.Close();
				#endregion
				#endregion
				#region SampleProject.vcw
				WS = SourceCodeRepository.Get_CP_SampleProject_vcw();
				#region Write to disk
				W = File.CreateText(outputDirectory.FullName + "\\SampleProject.vcw");
				W.Write(WS);
				W.Close();
				#endregion
				#endregion
				#region StdAfx.h
				WS = SourceCodeRepository.Get_CP_StdAfx_h();
				#region Write to disk
				W = File.CreateText(outputDirectory.FullName + "\\StdAfx.h");
				W.Write(WS);
				W.Close();
				#endregion
				#endregion
				#region StdAfx.cpp
				WS = SourceCodeRepository.Get_CP_StdAfx_cpp();
				#region Write to disk
				W = File.CreateText(outputDirectory.FullName + "\\StdAfx.cpp");
				W.Write(WS);
				W.Close();
				#endregion
				#endregion
				#region SampleProject.ico
				#region Write to disk
				byte[] b = SourceCodeRepository.Get_CP_SampleProject_ico();
				FileStream F = File.Create(outputDirectory.FullName + "\\SampleProject.ico",b.Length);
				F.Write(b,0,b.Length);
				F.Close();
				#endregion
				#endregion
			}
			#endregion

			foreach(UPnPDevice device in devices)
			{
				DeviceConf = (ServiceGenerator.Configuration)device.User;
				if (DeviceConf.ConfigType==ServiceGenerator.ConfigurationType.CONTROLPOINT)
				{
					CPOK = true;
					RetVal = GenerateEx(device,outputDirectory,SourceCodeRepository.CreateTableOfServiceNames(device),ref this.SampleApplication);
					if (!RetVal){break;}
				}
			}

			if (CPOK)
			{
				SourceCodeRepository.Generate_SSDPClient(Configuration.prefixlib,outputDirectory,Configuration.UPNP_1dot1);
				SourceCodeRepository.Generate_UPnPControlPointStructs(Configuration.prefixlib,outputDirectory);
			}

			if (SampleApplication!=null)
			{
				SampleApplication = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_CreateControlPoint}}}","//{{{END_CreateControlPoint}}}",SampleApplication);
				SampleApplication = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_CP_DISCOVER_REMOVE_SINK}}}","//{{{END_CP_DISCOVER_REMOVE_SINK}}}",SampleApplication);
				
				SampleApplication = SampleApplication.Replace("//{{{CP_EventSink}}}","");
				SampleApplication = SampleApplication.Replace("//{{{CP_InvokeSink}}}","");
				SampleApplication = SampleApplication.Replace("//{{{CP_EventRegistrations}}}","");
				SampleApplication = SampleApplication.Replace("//{{{CP_SampleInvoke}}}","");
				SampleApplication = SampleApplication.Replace("//{{{CreateControlPoint}}}","");
		
				SampleApplication = SampleApplication.Replace("//{{{CP_CONNECTING_ADD/REMOVE_SINKS}}}","");
				SampleApplication = SourceCodeRepository.RemoveAndClearTag("//{{{CP_DISCOVER/REMOVE_SINKS_BEGIN}}}","//{{{CP_DISCOVER/REMOVE_SINKS_END}}}",SampleApplication);
			}

			return(RetVal);
		}

		protected bool GenerateEx(UPnPDevice device,DirectoryInfo outputDirectory, Hashtable serviceNames, ref string SampleApp)
		{
			#region Initialization
			//StreamWriter W;
			DText PP = new DText();
			PP.ATTRMARK = ":";

			bool OkToDo=false;
			string WS,tmp;
			StreamWriter writer;

			SequenceTable.Clear();
			ChoiceTable.Clear();
			SequenceCounter=0;
			ChoiceCounter=0;

			if (this.SubTarget==SUBTARGETS.NONE)
			{
				UseSystem = this.Platform.ToString();
			}
			else
			{
				UseSystem = this.SubTarget.ToString();
			}
			UseInfoString = UseSystem + ", UPnP/1.0, MicroStack/" + UseVersion;

			if (this.Language == LANGUAGES.C)
			{
				pc_methodPrefix = ((ServiceGenerator.Configuration)device.User).Prefix;
				pc_methodLibPrefix = Configuration.prefixlib;
				pc_methodPrefixDef = CallingConvention + pc_methodPrefix;
			}

			AllServices.Clear();
			AddAllServices(device);
			Fix(device,0, serviceNames);

			SortedList SL = new SortedList();
			IDictionaryEnumerator en = serviceNames.GetEnumerator();
			DText Parser = new DText();
			Parser.ATTRMARK = ":";
			Parser[0] = device.DeviceURN;
			string DeviceName = Parser[4];

			// *** Generate Main Code
			Log("Writing main stack module...");


			while(en.MoveNext())
			{
				SL[en.Value] = en.Key;
			}
			en = SL.GetEnumerator();

			PrivateClassDeclarations = new CodeProcessor(new StringBuilder(),this.Language == LANGUAGES.CPP);
			PublicClassDeclarations = new CodeProcessor(new StringBuilder(),this.Language == LANGUAGES.CPP);
			CodeProcessor cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
			cs.NewLine = this.CodeNewLine;
			cs.ClassDefinitions = PrivateClassDeclarations;
			cs.PublicClassDefinitions = PublicClassDeclarations;
			PrivateClassDeclarations.CodeTab = Indent;
			PublicClassDeclarations.CodeTab = Indent;
			cs.CodeTab = Indent;

			if (this.Language == LANGUAGES.CPP) 
			{
				AddLicense(cs,pc_methodPrefix + "ControlPoint.cpp");
			} 
			else
			{
				AddLicense(cs,pc_methodPrefix + "ControlPoint.c");
			}
			cs.Append(cl);
			#endregion


			#region UPnPControlPoint.h
			WS = SourceCodeRepository.GetControlPoint_H_Template(this.pc_methodPrefix);

			#region GetService
			cs = new CodeProcessor(new StringBuilder(),this.Language == LANGUAGES.CPP);
			en.Reset();
			while(en.MoveNext())
			{
				string name = (string)en.Key;
				cs.Append("	struct UPnPService *"+pc_methodPrefixDef+"GetService_"+name+"(struct UPnPDevice *device);"+cl);
			}	
			WS = WS.Replace("//{{{UPnPGetService}}}",cs.ToString());
			#endregion
			#region Event Callbacks
			cs = new CodeProcessor(new StringBuilder(),this.Language == LANGUAGES.CPP);
			en.Reset();
			while(en.MoveNext())
			{
				UPnPService s = (UPnPService)en.Value;
				string name = (string)en.Key;
				foreach(UPnPStateVariable variable in s.GetStateVariables()) 
				{
					if (variable.SendEvent == true) 
					{
						cs.Append("extern void (*"+pc_methodPrefix+"EventCallback_"+name+"_"+variable.Name+")(struct UPnPService* Service,");
						cs.Append(ToCType(variable.GetNetType().FullName)+" "+variable.Name);
						if (variable.GetNetType()==typeof(byte[]))
						{
							cs.Append(", int " + variable.Name+"Length");
						}
						cs.Append(");"+cl);
					}
				}
			}
			WS = WS.Replace("//{{{ExternEventCallbacks}}}",cs.ToString());
			#endregion
			#region Invoke Methods
			cs = new CodeProcessor(new StringBuilder(),this.Language == LANGUAGES.CPP);
			en.Reset();
			while(en.MoveNext())
			{
				UPnPService s = (UPnPService)en.Value;
				ServiceGenerator.ServiceConfiguration SConf = (ServiceGenerator.ServiceConfiguration)s.User;

				string name = (string)en.Key;
				foreach(UPnPAction A in s.Actions)
				{
					cs.Append("	void "+pc_methodPrefix+"Invoke_"+name+"_"+A.Name+"(struct UPnPService *service, void (*CallbackPtr)(struct UPnPService *sender,int ErrorCode,void *user");
					if (A.HasReturnValue)
					{
						if (A.GetRetArg().RelatedStateVar.ComplexType==null)
						{
							// NonComplex
							cs.Append(","+ToCType(A.GetRetArg().RelatedStateVar.GetNetType().FullName)+" "+A.GetRetArg().Name);
							if (A.GetRetArg().RelatedStateVar.GetNetType()==typeof(byte[]))
							{
								cs.Append(", int " + A.GetRetArg().Name+"Length");
							}
						}
						else
						{
							// Complex
							cs.Append(", struct " + A.GetRetArg().RelatedStateVar.ComplexType.Name_LOCAL+" *"+A.GetRetArg().Name);
						}
					}
					foreach(UPnPArgument Arg in A.Arguments)
					{
						if (!Arg.IsReturnValue && Arg.Direction=="out")
						{
							if (Arg.RelatedStateVar.ComplexType==null)
							{
								//NonComplex
								cs.Append(","+ToCType(Arg.RelatedStateVar.GetNetType().FullName)+" "+Arg.Name);
								if (Arg.RelatedStateVar.GetNetType()==typeof(byte[]))
								{
									cs.Append(",int " + Arg.Name+"Length");
								}
							}
							else
							{
								//Complex
								cs.Append(",struct "+Arg.RelatedStateVar.ComplexType.Name_LOCAL+" *_"+Arg.Name);
							}
						}
					}
					cs.Append(")");
					cs.Append(",void* _user");
					foreach(UPnPArgument Arg in A.Arguments)
					{
						if (Arg.Direction=="in")
						{
							if (Arg.RelatedStateVar.ComplexType==null)
							{
								//NonComplex
								cs.Append(", "+ToCType(Arg.RelatedStateVar.GetNetType().FullName)+" ");
								if (Arg.RelatedStateVar.GetNetType()==typeof(string) && !SConf.Actions_ManualEscape.Contains(A))
								{
									cs.Append("unescaped_");
								}
								cs.Append(Arg.Name);
								if (Arg.RelatedStateVar.GetNetType()==typeof(byte[]))
								{
									cs.Append(", int " + Arg.Name+"Length");
								}	
							}
							else
							{
								//Complex
								cs.Append(", struct "+Arg.RelatedStateVar.ComplexType.Name_LOCAL+" *"+Arg.Name);
							}
						}
					}
					cs.Append(");"+cl);
				}
				bool NeedManualComment = false;
				foreach(UPnPAction A in s.Actions)
				{
					if (SConf.Actions_ManualEscape.Contains(A))
					{
						// Manual Escape
						NeedManualComment = true;
						break;
					}
				}
				if (NeedManualComment)
				{
					cs.Append(cl);
					cs.Comment("The string parameters for the following actions MUST be MANUALLY escaped");
					foreach(UPnPAction A in s.Actions)
					{
						if (SConf.Actions_ManualEscape.Contains(A))
						{
							// Manual Escape
							cs.Comment("	void "+pc_methodPrefix+"Invoke_"+name+"_"+A.Name);
						}
					}
					cs.Append(cl);
				}
			}
			WS = WS.Replace("//{{{UPnPInvoke_Methods}}}",cs.ToString());
			#endregion

			#region Complex Types
			cs = new CodeProcessor(new StringBuilder(),this.Language == LANGUAGES.CPP);
			EmbeddedCGenerator.BuildComplexTypeDefinitionsAndHeaders(SL,cs,SequenceTable,ChoiceTable,ref SequenceCounter,ref ChoiceCounter,this.pc_methodPrefix,this.pc_methodLibPrefix);
			WS = WS.Replace("//{{{UPnPComplexTypes}}}",cs.ToString());
			#endregion

			#region XML Custom Tags
			#region Custom Processing
	
			CustomTagList.Clear();
			IDictionaryEnumerator NamespaceEnumerator = ((ServiceGenerator.Configuration)device.User).CustomFieldTable.GetEnumerator();
			while(NamespaceEnumerator.MoveNext())
			{
				IDictionaryEnumerator EntryEnumerator = ((Hashtable)NamespaceEnumerator.Value).GetEnumerator();
				while(EntryEnumerator.MoveNext())
				{
					CustomTagList.Add(new object[2]{EntryEnumerator.Key,NamespaceEnumerator.Key});
				}
			}


			if (this.CustomTagList.Count>0)
			{
				WS = SourceCodeRepository.RemoveTag("//{{{BEGIN_CustomTagSpecific}}}","//{{{END_CustomTagSpecific}}}",WS);
			}
			else
			{
				WS = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_CustomTagSpecific}}}","//{{{END_CustomTagSpecific}}}",WS);
			}
			#endregion
			#region #define
			cs = new CodeProcessor(new StringBuilder(),this.Language == LANGUAGES.CPP);

			foreach(object[] foo in this.CustomTagList)
			{
				PP[0] = (string)foo[0];
				string FieldName;
				string FieldNameSpace = (string)foo[1];
				
				if (PP.DCOUNT()==1)
				{
					FieldName = PP[1];
				}
				else
				{
					FieldName = PP[2];
				}

				cs.Append("/*! \\def "+FieldName.ToUpper()+cl);
				cs.Append("	\\brief Custom XML Element: Local Name"+cl);
				cs.Append("*/"+cl);
				cs.Append("#define " + FieldName.ToUpper() + " \"" + FieldName + "\""+cl);
				cs.Append("/*! \\def "+FieldName.ToUpper()+"_NAMESPACE"+cl);
				cs.Append("	\\brief Custom XML Element: Fully Qualified Namespace"+cl);
				cs.Append("*/"+cl);
				cs.Append("#define " + FieldName.ToUpper() + "_NAMESPACE \"" + FieldNameSpace + "\""+cl);				
			}
			cs.Append(cl);
			foreach(object[] foo in this.CustomTagList)
			{
				PP[0] = (string)foo[0];
				string FieldName;
				string FieldNameSpace = (string)foo[1];
				
				if (PP.DCOUNT()==1)
				{
					FieldName = PP[1];
				}
				else
				{
					FieldName = PP[2];
				}
				cs.Append(" char *UPnPGetCustomXML_" + FieldName + "(struct UPnPDevice *d);"+cl);
			}
			WS = WS.Replace("//{{{CustomXMLTags}}}",cs.ToString());
			#endregion
			#endregion

			#region Prefixes
			WS = WS.Replace("UPnP/","upnp/");
			WS = WS.Replace("UPnPControlPointStructs.h","upnpcontrolpointstructs.h");
			WS = WS.Replace("UPnPDevice","upnpdevice");
			WS = WS.Replace("UPnPService","upnpservice");
			WS = WS.Replace("UPnPAction","upnpaction");
			WS = WS.Replace("UPnPStateVariable","upnpstatevariable");
			WS = WS.Replace("UPnPAllowedValue","upnpallowedvalue");
			WS = WS.Replace("DeviceDescriptionInterruptSink","devicedescriptioninterruptsink");
			WS = WS.Replace("DeviceExpired","deviceexpired");
			WS = WS.Replace("SubscribeForUPnPEvents","subscribeforupnpevents");
			WS = WS.Replace("UnSubscribeUPnPEvents","unsubscribeupnpevents");

			WS = WS.Replace("UPnP",this.pc_methodPrefix);
			WS = WS.Replace("ILib",this.pc_methodLibPrefix);

			WS = WS.Replace("upnp/","UPnP/");
			WS = WS.Replace("upnpaction","UPnPAction");
			WS = WS.Replace("upnpstatevariable","UPnPStateVariable");
			WS = WS.Replace("upnpallowedvalue","UPnPAllowedValue");
			WS = WS.Replace("devicedescriptioninterruptsink","DeviceDescriptionInterruptSink");
			WS = WS.Replace("deviceexpired","deviceexpired");
			WS = WS.Replace("upnpdevice","UPnPDevice");
			WS = WS.Replace("upnpservice","UPnPService");
			WS = WS.Replace("subscribeforupnpevents","SubscribeForUPnPEvents");
			WS = WS.Replace("unsubscribeupnpevents","UnSubscribeUPnPEvents");
			WS = WS.Replace("upnpcontrolpointstructs.h","UPnPControlPointStructs.h");


			#endregion

			#region Write to disk

			writer = File.CreateText(outputDirectory.FullName + "\\"+pc_methodPrefix+"ControlPoint.h");
			writer.Write(WS);
			writer.Close();
			#endregion

			#endregion
			#region UPnPControlPoint.c
			WS = SourceCodeRepository.GetControlPoint_C_Template(this.pc_methodPrefix);

			#region Event Callback Methods
			cs = new CodeProcessor(new StringBuilder(),this.Language == LANGUAGES.CPP);
			en = SL.GetEnumerator();
			while(en.MoveNext())
			{
				UPnPService s = (UPnPService)en.Value;
				string name = (string)en.Key;
				foreach(UPnPStateVariable variable in s.GetStateVariables()) 
				{
					if (variable.SendEvent == true) 
					{
						cs.Append("void (*"+pc_methodPrefix+"EventCallback_"+name+"_"+variable.Name+")(struct UPnPService* Service,");
						cs.Append(ToCType(variable.GetNetType().FullName)+" value");
						if (variable.GetNetType()==typeof(byte[]))
						{
							cs.Append(", int valueLength");
						}
						cs.Append(");"+cl);
					}
				}
			}
			WS = WS.Replace("//{{{EventCallbacks}}}",cs.ToString());
			#endregion
			#region GetDevice2 If Statement
			cs = new CodeProcessor(new StringBuilder(),this.Language == LANGUAGES.CPP);
			cs.Append("if (strncmp(device->DeviceType,\""+device.DeviceURN_Prefix+"\","+device.DeviceURN_Prefix.Length.ToString()+")==0 && atoi(device->DeviceType+"+device.DeviceURN_Prefix.Length.ToString()+")>=1)"+cl);			
			WS = WS.Replace("//{{{UPnPGetDevice2_if_statement}}}",cs.ToString());
			#endregion
			#region GetService Methods
			cs = new CodeProcessor(new StringBuilder(),this.Language == LANGUAGES.CPP);

			en = SL.GetEnumerator();
			while(en.MoveNext())
			{
				UPnPService S = (UPnPService)en.Value;
				string name = (string)en.Key;
				cs.Append("/*! \\fn "+pc_methodPrefix+"GetService_"+name+"(struct UPnPDevice *device)"+cl);
				cs.Append("	\\brief Returns the "+name+" service from the specified device");
				cs.Append("	\\par"+cl);
				cs.Append("	Service Type = " + S.ServiceURN_Prefix.Substring(0,S.ServiceURN_Prefix.Length-1) + "<br>"+cl);
				cs.Append("	Version >= " + S.Version+cl);
				cs.Append("	\\param device The device object to query"+cl);
				cs.Append("	\\returns A pointer to the service object"+cl);
				cs.Append("*/"+cl);
				cs.Append("struct UPnPService *"+pc_methodPrefix+"GetService_"+name+"(struct UPnPDevice *device)"+cl);
				cs.Append("{"+cl);
				cs.Append("	return("+pc_methodPrefix+"GetService(device,\""+S.ServiceURN+"\","+S.ServiceURN.Length.ToString()+"));"+cl);
				cs.Append("}"+cl);
			}
			WS = WS.Replace("//{{{UPnPGetServiceMethods}}}",cs.ToString());
			#endregion
			#region GetDeviceCount If Statement
			cs = new CodeProcessor(new StringBuilder(),this.Language == LANGUAGES.CPP);
			cs.Append("	if (strncmp(device->DeviceType,\"" + device.DeviceURN + "\","+device.DeviceURN.Length.ToString()+")==0)"+cl);
			cs.Append("	{"+cl);
			cs.Append("		++RetVal;"+cl);
			cs.Append("	}"+cl);
			WS = WS.Replace("//{{{GetDeviceCountIfStatement}}}",cs.ToString());
			#endregion
			#region Service Event Sink
			cs = new CodeProcessor(new StringBuilder(),this.Language == LANGUAGES.CPP);
			Hashtable ES = new Hashtable();
			en.Reset();
			while(en.MoveNext())
			{
				UPnPService s = (UPnPService)en.Value;
				string sname = (string)en.Key;
				string surn = s.ServiceURN;
				surn = surn.Substring(0,surn.LastIndexOf(":"));
				if (ES.ContainsKey(surn)==false)
				{
					ES[surn] = true;
					BuildEventParser(cs,s,surn,sname);
				}
			}
			WS = WS.Replace("//{{{Service_EventSink}}}",cs.ToString());
			#endregion
			#region OnEvent_Sink If Statement
			cs = new CodeProcessor(new StringBuilder(),this.Language == LANGUAGES.CPP);

			ES = new Hashtable();
			en.Reset();
			bool isfirst = true;
			while(en.MoveNext())
			{
				UPnPService s = (UPnPService)en.Value;
				string surn = s.ServiceURN;
				surn = surn.Substring(0,surn.LastIndexOf(":"));
				if (ES.ContainsKey(surn)==false)
				{
					ES[surn] = true;
					if (surn.StartsWith("urn:schemas-upnp-org:"))
					{
						// Standard Service
						surn = surn.Substring(surn.LastIndexOf(":")+1);
					}
					else
					{
						// Proprietary Service
						surn = surn.Replace(":","_");
						surn = surn.Replace("-","_");
						surn = surn.Replace(".","_");
					}
					if (isfirst == false) cs.Append("else"+cl);
					cs.Append("if (type_length>"+s.ServiceURN.Substring(0,s.ServiceURN.LastIndexOf(":")+1).Length.ToString() + " && strncmp(\""+s.ServiceURN.Substring(0,s.ServiceURN.LastIndexOf(":")+1)+"\",service->ServiceType,"+s.ServiceURN.Substring(0,s.ServiceURN.LastIndexOf(":")+1).Length.ToString()+")==0)"+cl);
					cs.Append("{"+cl);
					cs.Append("	"+this.pc_methodPrefix+surn+"_EventSink(buffer, BufferSize, service);"+cl);
					cs.Append("}"+cl);
					isfirst = false;
				}
			}
			WS = WS.Replace("//{{{EventSink_IfStatement}}}",cs.ToString());
			#endregion

			#region CreateSSDPClient
			cs = new CodeProcessor(new StringBuilder(),this.Language == LANGUAGES.CPP);
			cs.Append("	cp->SSDP = "+pc_methodLibPrefix+"CreateSSDPClientModule(Chain,\""+device.DeviceURN+"\", "+device.DeviceURN.Length.ToString()+", &"+pc_methodPrefix+"SSDP_Sink,cp);"+cl);
			WS = WS.Replace("//{{{CreateSSDPClientModule}}}",cs.ToString());
			#endregion

			#region Invocation Sink
			cs = new CodeProcessor(new StringBuilder(),this.Language == LANGUAGES.CPP);
			en.Reset();
			while(en.MoveNext())
			{
				UPnPService S = (UPnPService)en.Value;
				string name = (string)en.Key;

				foreach(UPnPAction A in S.Actions)
				{
					#region Invoke Sink Method
					cs.Append("void "+pc_methodPrefixDef+"Invoke_"+name+"_"+A.Name+"_Sink(");

					cs.Append(cl);
					cs.Append("		void *WebReaderToken,"+cl);
					cs.Append("		int IsInterrupt,"+cl);
					cs.Append("		struct packetheader *header,"+cl);
					cs.Append("		char *buffer,"+cl);
					cs.Append("		int *p_BeginPointer,"+cl);
					cs.Append("		int EndPointer,"+cl);
					cs.Append("		int done,"+cl);
					cs.Append("		void *_service,"+cl);
					cs.Append("		void *state,"+cl);
					cs.Append("		int *PAUSE)"+cl);
					
					cs.Append("{"+cl);
					cs.Append("	struct UPnPService *Service = (struct UPnPService*)_service;"+cl);
					cs.Append("	struct InvokeStruct *_InvokeData = (struct InvokeStruct*)state;"+cl);
					int NumArgs = 0;
					foreach(UPnPArgument Arg in A.Arguments)
					{
						if (Arg.IsReturnValue||Arg.Direction=="out") ++NumArgs;
					}
					if (NumArgs>0)
					{
						cs.Append("	int ArgLeft = "+NumArgs.ToString()+";"+cl);
						cs.Append("	struct "+this.pc_methodLibPrefix+"XMLNode *xml;"+cl);
						cs.Append("	struct "+this.pc_methodLibPrefix+"XMLNode *__xml;"+cl);
						cs.Append("	char *tempBuffer;"+cl);
						cs.Append("	int tempBufferLength;"+cl);

						bool needlongvar = false;
						bool needulongvar = false;
						bool hascomplex = false;
						foreach(UPnPArgument arg in A.Arguments)
						{
							if (arg.IsReturnValue || arg.Direction=="out")
							{
								if (arg.RelatedStateVar.ComplexType!=null)
								{
									hascomplex = true;
								}
								switch(arg.RelatedStateVar.GetNetType().FullName)
								{
									case "System.SByte":
									case "System.Int16":
									case "System.Int32":
										needlongvar = true;
										break;
									case "System.Byte":
									case "System.UInt16":
									case "System.UInt32":
										needulongvar = true;
										break;
								}
							}
						}
						if (needlongvar == true) cs.Append("long TempLong;"+cl);
						if (needulongvar == true) cs.Append("unsigned long TempULong;"+cl);
						if (hascomplex == true) cs.Append("struct "+this.pc_methodLibPrefix+"XMLNode *t_node;"+cl);
						if (A.HasReturnValue)
						{
							cs.Append("	"+this.ToCType(A.GetRetArg().RelatedStateVar.GetNetType().FullName)+" ");
							if (A.GetRetArg().RelatedStateVar.GetNetType()==typeof(byte[]))
							{
								cs.Append("__");
							}
							cs.Append(A.GetRetArg().Name);
							if (this.ToCType(A.GetRetArg().RelatedStateVar.GetNetType().FullName).EndsWith("*"))
							{
								cs.Append(" = NULL");
							}
							else
							{
								cs.Append(" = 0");
							}
							cs.Append(";"+cl);
							if (A.GetRetArg().RelatedStateVar.GetNetType()==typeof(byte[]))
							{
								cs.Append("	int __"+A.GetRetArg().Name+"Length=0;"+cl);
							}
						}
						foreach(UPnPArgument arg in A.Arguments)
						{
							if (arg.Direction=="out" && !arg.IsReturnValue)
							{
								if (arg.RelatedStateVar.ComplexType==null)
								{
									cs.Append("	"+ToCType(arg.RelatedStateVar.GetNetType().FullName)+" ");
									if (arg.RelatedStateVar.GetNetType()==typeof(byte[]))
									{
										cs.Append("__");
									}
									cs.Append(arg.Name);
									if (ToCType(arg.RelatedStateVar.GetNetType().FullName).EndsWith("*"))
									{
										cs.Append(" = NULL");
									} 
									else 
									{
										cs.Append(" = 0");
									}
									cs.Append(";"+cl);
									if (arg.RelatedStateVar.GetNetType()==typeof(byte[]))
									{
										cs.Append("	int __"+arg.Name+"Length = 0;"+cl);
									}
								}
								else
								{
									// Complex Type
									cs.Append(" struct "+arg.RelatedStateVar.ComplexType.Name_LOCAL+"* "+arg.Name+";"+cl);
								}
							}
						}
						cs.Append("	LVL3DEBUG(char *DebugBuffer;)"+cl);
						cs.Append(cl);

                        cs.Append("	UNREFERENCED_PARAMETER( WebReaderToken );" + cl);
                        cs.Append("	UNREFERENCED_PARAMETER( IsInterrupt );" + cl);
                        cs.Append("	UNREFERENCED_PARAMETER( PAUSE );" + cl);
                        cs.Append(cl);

						cs.Append("	if (done == 0) return;"+cl);
                        cs.Append("	LVL3DEBUG(if ((DebugBuffer = (char*)malloc(EndPointer + 1)) == NULL) ILIBCRITICALEXIT(254);)" + cl);
						cs.Append("	LVL3DEBUG(memcpy(DebugBuffer,buffer,EndPointer);)"+cl);
						cs.Append("	LVL3DEBUG(DebugBuffer[EndPointer]=0;)"+cl);
						cs.Append("	LVL3DEBUG(printf(\"\\r\\n SOAP Recieved:\\r\\n%s\\r\\n\",DebugBuffer);)"+cl);
						cs.Append("	LVL3DEBUG(free(DebugBuffer);)"+cl);

						cs.Append("	if (_InvokeData->CallbackPtr == NULL)"+cl);
						cs.Append("	{"+cl);
						cs.Append("		"+this.pc_methodPrefix+"Release(Service->Parent);"+cl);
						cs.Append("		free(_InvokeData);"+cl);
						//						cs.Append("		"+pc_methodLibPrefix+"CloseRequest(reader);"+cl);
						cs.Append("		return;"+cl);
						cs.Append("	}"+cl);
						cs.Append("	else"+cl);
						cs.Append("	{"+cl);
						cs.Append("		if (header == NULL)"+cl);
						cs.Append("		{"+cl);
						cs.Comment("Connection Failed");
						cs.Append("			((void (*)(struct UPnPService*,int,void*");
						if (A.HasReturnValue)
						{
							cs.Append(","+ToCType(A.GetRetArg().RelatedStateVar.GetNetType().FullName));
						}
						foreach(UPnPArgument Arg in A.Arguments)
						{
							if (!Arg.IsReturnValue && Arg.Direction=="out")
							{
								cs.Append(","+ToCType(Arg.RelatedStateVar.GetNetType().FullName));
							}
						}
						cs.Append("))_InvokeData->CallbackPtr)(Service,IsInterrupt==0?-1:IsInterrupt,_InvokeData->User");
						if (A.HasReturnValue)
						{
							cs.Append(",INVALID_DATA");
						}
						foreach(UPnPArgument Arg in A.Arguments)
						{
							if (!Arg.IsReturnValue && Arg.Direction=="out")
							{
								cs.Append(",INVALID_DATA");
							}
						}
						cs.Append(");"+cl);
						cs.Append("			"+this.pc_methodPrefix+"Release(Service->Parent);"+cl);
						cs.Append("			free(_InvokeData);"+cl);
						//						cs.Append("			"+pc_methodLibPrefix+"CloseRequest(reader);"+cl);
						cs.Append("			return;"+cl);
						cs.Append("		}"+cl);
						cs.Append("		else if (!ILibWebClientIsStatusOk(header->StatusCode))"+cl);
						cs.Append("		{"+cl);
						cs.Comment("SOAP Fault");
						cs.Append("			((void (*)(struct UPnPService*,int,void*");
						if (A.HasReturnValue)
						{
							cs.Append(","+ToCType(A.GetRetArg().RelatedStateVar.GetNetType().FullName));
						}
						foreach(UPnPArgument Arg in A.Arguments)
						{
							if (!Arg.IsReturnValue && Arg.Direction=="out")
							{
								cs.Append(", "+ToCType(Arg.RelatedStateVar.GetNetType().FullName));
							}
						}
						cs.Append("))_InvokeData->CallbackPtr)(Service, "+this.pc_methodPrefix+"GetErrorCode(buffer, EndPointer-(*p_BeginPointer)), _InvokeData->User");
						if (A.HasReturnValue)
						{
							cs.Append(", INVALID_DATA");
						}
						foreach(UPnPArgument Arg in A.Arguments)
						{
							if (!Arg.IsReturnValue && Arg.Direction=="out")
							{
								cs.Append(", INVALID_DATA");
							}
						}
						cs.Append(");"+cl);
						cs.Append("			"+this.pc_methodPrefix+"Release(Service->Parent);"+cl);
						cs.Append("			free(_InvokeData);"+cl);
						cs.Append("			return;"+cl);
						cs.Append("		}"+cl);
						cs.Append("	}"+cl);
						cs.Append(cl);

						cs.Append("	__xml = xml = "+this.pc_methodLibPrefix+"ParseXML(buffer,0,EndPointer-(*p_BeginPointer));"+cl);
						cs.Append("	if ("+this.pc_methodLibPrefix+"ProcessXMLNodeList(xml)==0)"+cl);
						cs.Append("	{"+cl);
						cs.Append("	while(xml != NULL)"+cl);
						cs.Append("	{"+cl);
						cs.Append("		if (xml->NameLength == "+(A.Name.Length+8).ToString()+" && memcmp(xml->Name, \"" + A.Name+"Response\", "+(A.Name.Length+8).ToString()+") == 0)"+cl);
						cs.Append("		{"+cl);
						cs.Append("			xml = xml->Next;"+cl);
						cs.Append("			while(xml != NULL)"+cl);
						cs.Append("			{"+cl);

						bool IsFirst = true;
						foreach(UPnPArgument arg in A.Arguments)
						{
							if (arg.IsReturnValue || arg.Direction=="out")
							{
								if (IsFirst == false) cs.Append("else "+cl);
								cs.Append("					if (xml->NameLength == "+arg.Name.Length.ToString()+" && memcmp(xml->Name, \""+arg.Name+"\", "+arg.Name.Length.ToString()+") == 0)"+cl);
								cs.Append("					{"+cl);
								cs.Append("						--ArgLeft;"+cl);
								if (arg.RelatedStateVar.ComplexType==null)
								{
									cs.Append("						tempBufferLength = "+this.pc_methodLibPrefix+"ReadInnerXML(xml, &tempBuffer);"+cl);
									if (ToCType(arg.RelatedStateVar.GetNetType().FullName)=="char*")
									{
										cs.Append("						if (tempBufferLength != 0)"+cl);
										cs.Append("						{"+cl);
										cs.Append("							tempBuffer[tempBufferLength] = '\\0';"+cl);
										cs.Append("							"+arg.Name+" = tempBuffer;"+cl);
										if (arg.RelatedStateVar.GetNetType()==typeof(string))
										{
											cs.Append("							ILibInPlaceXmlUnEscape("+arg.Name+");"+cl);
										}
										cs.Append("						}"+cl);
									}
									else
									{
										switch(arg.RelatedStateVar.GetNetType().FullName)
										{
											case "System.Byte[]":
												cs.Append("								__"+arg.Name+"Length="+this.pc_methodLibPrefix+"Base64Decode(tempBuffer, tempBufferLength, &__"+arg.Name+");"+cl);
												break;
											case "System.Boolean":
												cs.Append("								"+arg.Name+" = 1;"+cl);
												cs.Append("								if (strncasecmp(tempBuffer, \"false\", 5)==0 || strncmp(tempBuffer, \"0\", 1)==0 || strncasecmp(tempBuffer, \"no\", 2) == 0)"+cl);
												cs.Append("								{"+cl);
												cs.Append("									"+arg.Name+" = 0;"+cl);
												cs.Append("								}"+cl);
												break;
											case "System.DateTime":
												cs.Append("								if (tempBufferLength!=0)"+cl);
												cs.Append("								{"+cl);
												cs.Append("									tempBuffer[tempBufferLength] = '\\0';"+cl);
												cs.Append("									"+arg.Name+" = "+this.pc_methodLibPrefix+"Time_Parse(tempBuffer);"+cl);
												cs.Append("								}"+cl);
												break;
											case "System.SByte":
											case "System.Int16":
											case "System.Int32":
												cs.Append("								if ("+this.pc_methodLibPrefix+"GetLong(tempBuffer,tempBufferLength,&TempLong)==0)"+cl);
												cs.Append("								{"+cl);
												cs.Append("									"+arg.Name+" = ("+ToCType(arg.RelatedStateVar.GetNetType().FullName)+") TempLong;"+cl);
												cs.Append("								}"+cl);
												break;
											case "System.Byte":
											case "System.UInt16":
											case "System.UInt32":
												cs.Append("								if ("+this.pc_methodLibPrefix+"GetULong(tempBuffer,tempBufferLength,&TempULong)==0)"+cl);
												cs.Append("								{"+cl);
												cs.Append("									"+arg.Name+" = ("+ToCType(arg.RelatedStateVar.GetNetType().FullName)+") TempULong;"+cl);
												cs.Append("								}"+cl);
												break;
										}
									}
								}
								else
								{
									// Complex Type
									cs.Append("						if (!"+this.pc_methodPrefix+"IsLegacyDevice(header))"+cl);
									cs.Append("						{"+cl);
									cs.Append("							"+arg.Name+" = "+this.pc_methodPrefix+"Parse_"+arg.RelatedStateVar.ComplexType.Name_LOCAL+"(xml->Next);"+cl);
									cs.Append("						}"+cl);
									cs.Append("						else"+cl);
									cs.Append("						{"+cl);
									cs.Append("							tempBufferLength = "+this.pc_methodLibPrefix+"ReadInnerXML(xml,&tempBuffer);"+cl);
									cs.Append("							tempBufferLength = "+this.pc_methodLibPrefix+"InPlaceXmlUnEscape(tempBuffer);"+cl);
									cs.Append("							t_node = "+this.pc_methodLibPrefix+"ParseXML(tempBuffer,0,tempBufferLength);"+cl);
									cs.Append("							"+this.pc_methodLibPrefix+"ProcessXMLNodeList(t_node);"+cl);
									cs.Append("							"+arg.Name+" = "+this.pc_methodPrefix+"Parse_"+arg.RelatedStateVar.ComplexType.Name_LOCAL+"(t_node);"+cl);
									cs.Append("							"+this.pc_methodLibPrefix+"DestructXMLNodeList(t_node);"+cl);
									cs.Append("						}"+cl);
								}
								/*
								if (ToCType(arg.RelatedStateVar.GetNetType().FullName)=="char*")
								{
									cs.Append("							else"+cl);
									cs.Append("							{"+cl);
									cs.Append("								"+arg.Name+" = NULL;"+cl);
									cs.Append("							}"+cl);
								}
								*/
								//cs.Append("						}"+cl);
								cs.Append("					}"+cl);
								IsFirst = false;
							}
						}

						cs.Append("			xml = xml->Peer;"+cl);
						cs.Append("		  }"+cl);
						cs.Append("     }"+cl);
						cs.Append("		if (xml!=NULL) {xml = xml->Next;}"+cl);
						cs.Append("	}"+cl);
						cs.Append("	"+this.pc_methodLibPrefix+"DestructXMLNodeList(__xml);"+cl);
						cs.Append("	}"+cl); // End of the if{} for ILibProcessXMLNodeList()
					}
					else
					{
                        cs.Append("	UNREFERENCED_PARAMETER(WebReaderToken);" + cl);
                        cs.Append("	UNREFERENCED_PARAMETER(IsInterrupt);"+cl);
                        cs.Append("	UNREFERENCED_PARAMETER(PAUSE);"+cl);

						cs.Append("	if (done == 0) return;"+cl);
						//						cs.Append("	"+pc_methodLibPrefix+"CloseRequest(reader);"+cl);
						cs.Append("	if (_InvokeData->CallbackPtr == NULL)"+cl);
						cs.Append("	{"+cl);
						cs.Append("		"+pc_methodPrefix+"Release(Service->Parent);"+cl);
						cs.Append("		free(_InvokeData);"+cl);
						//						cs.Append("		"+pc_methodLibPrefix+"CloseRequest(reader);"+cl);
						cs.Append("		return;"+cl);
						cs.Append("	}"+cl);
						cs.Append("	else"+cl);
						cs.Append("	{"+cl);
						cs.Append("		if (header == NULL)"+cl);
						cs.Append("		{"+cl);
						cs.Comment("Connection Failed");
						cs.Append("			((void (*)(struct UPnPService*,int,void*");
						if (A.HasReturnValue)
						{
							cs.Append(","+ToCType(A.GetRetArg().RelatedStateVar.GetNetType().FullName));
							if (A.GetRetArg().RelatedStateVar.GetNetType()==typeof(byte[]))
							{
								cs.Append(",int");
							}
						}
						foreach(UPnPArgument Arg in A.Arguments)
						{
							if (!Arg.IsReturnValue && Arg.Direction=="out")
							{
								cs.Append(","+ToCType(Arg.RelatedStateVar.GetNetType().FullName));
								if (Arg.RelatedStateVar.GetNetType()==typeof(byte[]))
								{
									cs.Append(",int");
								}
							}
						}
						cs.Append("))_InvokeData->CallbackPtr)(Service,-1,_InvokeData->User");
						if (A.HasReturnValue)
						{
							cs.Append(",INVALID_DATA");
							if (A.GetRetArg().RelatedStateVar.GetNetType()==typeof(byte[]))
							{
								cs.Append(",INVALID_DATA");
							}
						}
						foreach(UPnPArgument Arg in A.Arguments)
						{
							if (!Arg.IsReturnValue && Arg.Direction=="out")
							{
								cs.Append(",INVALID_DATA");
								if (Arg.RelatedStateVar.GetNetType()==typeof(byte[]))
								{
									cs.Append(",INVALID_DATA");
								}
							}
						}
						cs.Append(");"+cl);
						cs.Append("			"+this.pc_methodPrefix+"Release(Service->Parent);"+cl);
						cs.Append("			free(_InvokeData);"+cl);
						//						cs.Append("			"+pc_methodLibPrefix+"CloseRequest(reader);"+cl);
						cs.Append("			return;"+cl);
						cs.Append("		}"+cl);
						cs.Append("		else if (!ILibWebClientIsStatusOk(header->StatusCode))"+cl);
						cs.Append("		{"+cl);
						cs.Comment("SOAP Fault");
						cs.Append("			((void (*)(struct UPnPService*,int,void*");
						if (A.HasReturnValue)
						{
							cs.Append(","+ToCType(A.GetRetArg().RelatedStateVar.GetNetType().FullName));
							if (A.GetRetArg().RelatedStateVar.GetNetType()==typeof(byte[]))
							{
								cs.Append(",int");
							}
						}
						foreach(UPnPArgument Arg in A.Arguments)
						{
							if (!Arg.IsReturnValue && Arg.Direction=="out")
							{
								cs.Append(","+ToCType(Arg.RelatedStateVar.GetNetType().FullName));
								if (Arg.RelatedStateVar.GetNetType()==typeof(byte[]))
								{
									cs.Append(",int");
								}
							}
						}
						cs.Append("))_InvokeData->CallbackPtr)(Service,"+this.pc_methodPrefix+"GetErrorCode(buffer,EndPointer-(*p_BeginPointer)),_InvokeData->User");
						if (A.HasReturnValue)
						{
							cs.Append(",INVALID_DATA");
							if (A.GetRetArg().RelatedStateVar.GetNetType()==typeof(byte[]))
							{
								cs.Append(",INVALID_DATA");
							}
						}
						foreach(UPnPArgument Arg in A.Arguments)
						{
							if (!Arg.IsReturnValue && Arg.Direction=="out")
							{
								cs.Append(",INVALID_DATA");
								if (Arg.RelatedStateVar.GetNetType()==typeof(byte[]))
								{
									cs.Append(",INVALID_DATA");
								}
							}
						}
						cs.Append(");"+cl);
						cs.Append("			"+this.pc_methodPrefix+"Release(Service->Parent);"+cl);
						cs.Append("			free(_InvokeData);"+cl);
						cs.Append("			return;"+cl);
						cs.Append("		}"+cl);

						cs.Append("	}"+cl);
					}
					cs.Append(cl);	
					if (NumArgs>0)
					{
						cs.Append("	if (ArgLeft!=0)"+cl);
						cs.Append("	{"+cl);
						cs.Append("		((void (*)(struct UPnPService*,int,void*");
						if (A.HasReturnValue)
						{
							cs.Append(","+ToCType(A.GetRetArg().RelatedStateVar.GetNetType().FullName));
							if (A.GetRetArg().RelatedStateVar.GetNetType()==typeof(byte[]))
							{
								cs.Append(",int");
							}
						}
						foreach(UPnPArgument Arg in A.Arguments)
						{
							if (!Arg.IsReturnValue && Arg.Direction=="out")
							{
								cs.Append(","+ToCType(Arg.RelatedStateVar.GetNetType().FullName));
								if (Arg.RelatedStateVar.GetNetType()==typeof(byte[]))
								{
									cs.Append(",int");
								}
							}
						}
						cs.Append("))_InvokeData->CallbackPtr)(Service,-2,_InvokeData->User");
						if (A.HasReturnValue)
						{
							cs.Append(",INVALID_DATA");
							if (A.GetRetArg().RelatedStateVar.GetNetType()==typeof(byte[]))
							{
								cs.Append(",INVALID_DATA");
							}
						}
						foreach(UPnPArgument Arg in A.Arguments)
						{
							if (!Arg.IsReturnValue && Arg.Direction=="out")
							{
								cs.Append(",INVALID_DATA");
								if (Arg.RelatedStateVar.GetNetType()==typeof(byte[]))
								{
									cs.Append(",INVALID_DATA");
								}
							}
						}
						cs.Append(");"+cl);
						cs.Append("	}"+cl);

						cs.Append("	else"+cl);
						cs.Append("{"+cl);
					}
					cs.Append("		((void (*)(struct UPnPService*,int,void*");
					if (A.HasReturnValue)
					{
						cs.Append(","+ToCType(A.GetRetArg().RelatedStateVar.GetNetType().FullName));
						if (A.GetRetArg().RelatedStateVar.GetNetType()==typeof(byte[]))
						{
							cs.Append(",int");
						}
					}
					foreach(UPnPArgument Arg in A.Arguments)
					{
						if (!Arg.IsReturnValue && Arg.Direction=="out")
						{
							cs.Append(","+ToCType(Arg.RelatedStateVar.GetNetType().FullName));
							if (Arg.RelatedStateVar.GetNetType()==typeof(byte[]))
							{
								cs.Append(",int");
							}
						}
					}
					cs.Append("))_InvokeData->CallbackPtr)(Service,0,_InvokeData->User");
					if (A.HasReturnValue)
					{
						if (A.GetRetArg().RelatedStateVar.GetNetType()==typeof(byte[]))
						{
							cs.Append(",__"+A.GetRetArg().Name+",__"+A.GetRetArg().Name+"Length");
						}
						else
						{
							cs.Append(","+A.GetRetArg().Name);
						}
					}
					foreach(UPnPArgument Arg in A.Arguments)
					{
						if (!Arg.IsReturnValue && Arg.Direction=="out")
						{
							if (Arg.RelatedStateVar.GetNetType()==typeof(byte[]))
							{
								cs.Append(",__"+Arg.Name+",__"+Arg.Name+"Length");
							}
							else
							{
								cs.Append(","+Arg.Name);
							}
						}
					}
					cs.Append(");"+cl);
					if (NumArgs>0)
					{
						cs.Append("	}"+cl);
					}
					cs.Append("	"+this.pc_methodPrefix+"Release(Service->Parent);"+cl);
					cs.Append("	free(_InvokeData);"+cl);
					//					cs.Append("	"+pc_methodLibPrefix+"CloseRequest(reader);"+cl);
					foreach(UPnPArgument Arg in A.Arguments)
					{
						if (Arg.IsReturnValue || Arg.Direction=="out")
						{
							if (Arg.RelatedStateVar.GetNetType()==typeof(byte[]))
							{
								cs.Append("	if (__"+Arg.Name+"!=NULL)"+cl);
								cs.Append("	{"+cl);
								cs.Append("		free(__"+Arg.Name+");"+cl);
								cs.Append("	}"+cl);
							}
						}
					}
					cs.Append("}"+cl);
					#endregion
				}
			}
			WS = WS.Replace("//{{{Invocation_Sinks}}}",cs.ToString());
			#endregion
			#region Invocation Methods
			cs = new CodeProcessor(new StringBuilder(),this.Language == LANGUAGES.CPP);
			en.Reset();
			while(en.MoveNext())
			{
				UPnPService S = (UPnPService)en.Value;
				ServiceGenerator.ServiceConfiguration SConf = (ServiceGenerator.ServiceConfiguration)S.User;

				string name = (string)en.Key;

				foreach(UPnPAction A in S.Actions)
				{
					#region Invoke Method -- Doxygen Comments
					cs.Append("/*! \\fn "+pc_methodPrefixDef+"Invoke_"+name+"_"+A.Name+"(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService *sender,int ErrorCode,void *user");
					if (A.HasReturnValue)
					{
						if (A.GetRetArg().RelatedStateVar.ComplexType==null)
						{
							// Non Complex Type
							cs.Append(","+ToCType(A.GetRetArg().RelatedStateVar.GetNetType().FullName)+" "+A.GetRetArg().Name);
							if (A.GetRetArg().RelatedStateVar.GetNetType()==typeof(byte[]))
							{
								// length
								cs.Append(",int "+A.GetRetArg().Name+"Length");
							}
						}
						else
						{
							// Complex Type
							cs.Append(", struct " + A.GetRetArg().RelatedStateVar.ComplexType.Name_LOCAL+" *"+A.GetRetArg().Name);
						}
					}
					foreach(UPnPArgument Arg in A.Arguments)
					{
						if (!Arg.IsReturnValue && Arg.Direction=="out")
						{
							if (Arg.RelatedStateVar.ComplexType==null)
							{
								// NonComplex
								cs.Append(","+ToCType(Arg.RelatedStateVar.GetNetType().FullName)+" " + Arg.Name);
								if (Arg.RelatedStateVar.GetNetType()==typeof(byte[]))
								{
									//length
									cs.Append(",int "+Arg.Name+"Length");
								}
							}
							else
							{
								// Complex
								cs.Append(",struct " + Arg.RelatedStateVar.ComplexType.Name_LOCAL+"* "+Arg.Name);
							}
						}
					}

					cs.Append("), void *_user");
					foreach(UPnPArgument Arg in A.Arguments)
					{
						if (Arg.Direction=="in")
						{
							if (Arg.RelatedStateVar.ComplexType==null)
							{
								// Non Complex
								cs.Append(", "+ToCType(Arg.RelatedStateVar.GetNetType().FullName)+" ");
								if (Arg.RelatedStateVar.GetNetType()==typeof(string) && !SConf.Actions_ManualEscape.Contains(A))
								{
									cs.Append("unescaped_");
								}
								cs.Append(Arg.Name);
								if (Arg.RelatedStateVar.GetNetType()==typeof(byte[]))
								{
									cs.Append(", int " + Arg.Name+"Length");
								}
							}
							else
							{
								// Complex
								cs.Append(", struct " + Arg.RelatedStateVar.ComplexType.Name_LOCAL+"* _"+Arg.Name);
							}
						}
					}
					cs.Append(")"+cl);
					cs.Append("	\\brief Invokes the "+A.Name+" action in the "+name+" service"+cl);
					cs.Append("	\\param service The UPnPService instance to invoke the action on"+cl);
					cs.Append("	\\param CallbackPtr The function pointer to be called when the invocation returns"+cl);
					foreach(UPnPArgument Arg in A.Arguments)
					{
						if (Arg.Direction=="in")
						{
							//
							// Loop through the Arguments, so we can add comments for the
							// in arguments
							//
							if (Arg.RelatedStateVar.ComplexType==null)
							{
								// Non Complex
								cs.Append("	\\param ");
								if (Arg.RelatedStateVar.GetNetType()==typeof(string) && !SConf.Actions_ManualEscape.Contains(A))
								{
									cs.Append("unescaped_");
								}
								cs.Append(Arg.Name);
								cs.Append(" Value of the "+Arg.Name+" parameter. ");
								if (Arg.RelatedStateVar.GetNetType()==typeof(string))
								{
									if (!SConf.Actions_ManualEscape.Contains(A))
									{
										// Automatic Escaping
										cs.Append(" <b>Automatically</b> escaped");
									}
									else
									{
										// User Must Escape
										cs.Append(" <b>MUST</b> be properly escaped.");
									}
									cs.Append(cl);
								}
								if (Arg.RelatedStateVar.GetNetType()==typeof(byte[]))
								{
									cs.Append("	\\param "+Arg.Name+"Length Size of \\a "+Arg.Name+cl);
								}
							}
							else
							{
								// Complex
								cs.Append(" \\param _"+Arg.Name+" Value of the "+Arg.Name+" parameter. "+cl);
							}
						}
					}
					cs.Append("*/"+cl);
					#endregion
					#region Invoke Method -- Implementation
					cs.Append("void "+pc_methodPrefixDef+"Invoke_"+name+"_"+A.Name+"(struct UPnPService *service,void (*CallbackPtr)(struct UPnPService *sender,int ErrorCode,void *user");
					if (A.HasReturnValue)
					{
						if (A.GetRetArg().RelatedStateVar.ComplexType==null)
						{
							// Non Complex Type
							cs.Append(","+ToCType(A.GetRetArg().RelatedStateVar.GetNetType().FullName)+" "+A.GetRetArg().Name);
							if (A.GetRetArg().RelatedStateVar.GetNetType()==typeof(byte[]))
							{
								// length
								cs.Append(",int "+A.GetRetArg().Name+"Length");
							}
						}
						else
						{
							// Complex Type
							cs.Append(", struct " + A.GetRetArg().RelatedStateVar.ComplexType.Name_LOCAL+" *"+A.GetRetArg().Name);
						}
					}
					foreach(UPnPArgument Arg in A.Arguments)
					{
						if (!Arg.IsReturnValue && Arg.Direction=="out")
						{
							if (Arg.RelatedStateVar.ComplexType==null)
							{
								// NonComplex
								cs.Append(","+ToCType(Arg.RelatedStateVar.GetNetType().FullName)+" "+Arg.Name);
								if (Arg.RelatedStateVar.GetNetType()==typeof(byte[]))
								{
									//length
									cs.Append(",int "+Arg.Name+"Length");
								}
							}
							else
							{
								// Complex
								cs.Append(",struct " + Arg.RelatedStateVar.ComplexType.Name_LOCAL+" *"+Arg.Name);
							}
						}
					}

					cs.Append("), void* user");
					foreach(UPnPArgument Arg in A.Arguments)
					{
						if (Arg.Direction=="in")
						{
							if (Arg.RelatedStateVar.ComplexType==null)
							{
								// Non Complex
								cs.Append(", "+ToCType(Arg.RelatedStateVar.GetNetType().FullName)+" ");
								if (Arg.RelatedStateVar.GetNetType()==typeof(string) && !SConf.Actions_ManualEscape.Contains(A))
								{
									cs.Append("unescaped_");
								}
								cs.Append(Arg.Name);
								if (Arg.RelatedStateVar.GetNetType()==typeof(byte[]))
								{
									cs.Append(", int " + Arg.Name+"Length");
								}
							}
							else
							{
								// Complex
								cs.Append(", struct " + Arg.RelatedStateVar.ComplexType.Name_LOCAL+"* _"+Arg.Name);
							}
						}
					}
					cs.Append(")"+cl);
					cs.Append("{"+cl);
					cs.Append("	int headerLength;"+cl);
					cs.Append("	char *headerBuffer;"+cl);
					cs.Append("	char *SoapBodyTemplate;"+cl);
					cs.Append("	char* buffer;"+cl);
					cs.Append("	int bufferLength;"+cl);
					cs.Append("	char* IP;"+cl);
					cs.Append("	unsigned short Port;"+cl);
					cs.Append("	char* Path;"+cl);
                    cs.Append("	size_t len;" + cl);
                    cs.Append("	struct InvokeStruct *invoke_data;" + cl);

					foreach(UPnPArgument Arg in A.Arguments)
					{
						if (Arg.Direction=="in" && (Arg.RelatedStateVar.GetNetType()==typeof(DateTime) || Arg.RelatedStateVar.GetNetType()==typeof(byte[])))
						{
							cs.Append("	char* __"+Arg.Name+";"+cl);
							if (Arg.RelatedStateVar.GetNetType()==typeof(byte[]))
							{
								cs.Append("	int __"+Arg.Name+"Length;"+cl);
							}
						}
						else if (Arg.Direction=="in" && Arg.RelatedStateVar.GetNetType()==typeof(string) &&!SConf.Actions_ManualEscape.Contains(A))
						{
							cs.Append(" char* "+Arg.Name+";"+cl);
						}
						else if (Arg.Direction=="in" && Arg.RelatedStateVar.GetNetType()==typeof(bool))
						{
							cs.Append("	int __"+Arg.Name+";"+cl);
						}
					}
					foreach(UPnPArgument Arg in A.Arguments)
					{
						if (Arg.Direction=="in" && Arg.RelatedStateVar.ComplexType!=null)
						{
							// Need to Serialize complex type to XML
							cs.Append("	char* " + Arg.Name + ";"+cl);
						}
					}
                    cs.Append("	if ((invoke_data = (struct InvokeStruct*)malloc(sizeof(struct InvokeStruct))) == NULL) ILIBCRITICALEXIT(254);" + cl);
                    cs.Append(cl);
					cs.Append("	if (service == NULL)"+cl);
					cs.Append("	{"+cl);
					cs.Append("		free(invoke_data);"+cl);
					cs.Append("		return;"+cl);
					cs.Append("	}"+cl);
					cs.Append(cl);
					foreach(UPnPArgument Arg in A.Arguments)
					{
						if (Arg.Direction=="in" && Arg.RelatedStateVar.ComplexType!=null)
						{
							// Need to Serialize complex type to XML
							cs.Append("	"+Arg.Name + " = "+this.pc_methodPrefix+"Serialize_"+Arg.RelatedStateVar.ComplexType.Name_LOCAL+"(_"+Arg.Name+");"+cl);
						}
					}
					cs.Append(cl);
					foreach(UPnPArgument Arg in A.Arguments)
					{
						if (Arg.Direction=="in" && Arg.RelatedStateVar.GetNetType()==typeof(bool))
						{
							cs.Append("	("+Arg.Name+"!=0)?(__"+Arg.Name+"=1):(__"+Arg.Name+"=0);"+cl);
						}
						if (Arg.Direction=="in" && Arg.RelatedStateVar.GetNetType()==typeof(DateTime))
						{
							cs.Append("	__"+Arg.Name+"="+this.pc_methodLibPrefix+"Time_Serialize("+Arg.Name+");"+cl);
						}
					}
					foreach(UPnPArgument Arg in A.Arguments)
					{
						if (Arg.Direction=="in" && Arg.RelatedStateVar.GetNetType()==typeof(byte[]))
						{
							cs.Append("	__"+Arg.Name+"Length = "+this.pc_methodLibPrefix+"Base64Encode("+Arg.Name+","+Arg.Name+"Length,&__"+Arg.Name+");"+cl);
						}
					}
					if (!SConf.Actions_ManualEscape.Contains(A))
					{
						foreach(UPnPArgument Arg in A.Arguments)
						{
							if (Arg.Direction=="in" && Arg.RelatedStateVar.GetNetType()==typeof(string))
							{
                                cs.Append("	if ((" + Arg.Name + " = (char*)malloc(1 + ILibXmlEscapeLength(unescaped_" + Arg.Name + "))) == NULL) ILIBCRITICALEXIT(254);" + cl);
								cs.Append("	ILibXmlEscape("+Arg.Name+",unescaped_"+Arg.Name+");"+cl);
							}
						}
					}

					string TotalBufferSizeDynamic = "(int)strlen(service->ServiceType)";
					int TotalBufferSizeStatic = 232 + (A.Name.Length*2);
					foreach(UPnPArgument Arg in A.Arguments)
					{
						if (Arg.Direction=="in")
						{
							TotalBufferSizeStatic += (Arg.Name.Length*2)+5;
							switch(Arg.RelatedStateVar.GetNetType().ToString())
							{
								case "System.Uri":
								case "System.String":
									TotalBufferSizeDynamic += "+(int)strlen("+Arg.Name+")";
									break;
								case "System.Boolean":
									TotalBufferSizeStatic += 5;
									break;
								case "System.Char":
									TotalBufferSizeStatic += 4;
									break;
								case "System.Byte[]":
									TotalBufferSizeDynamic += "+__"+Arg.Name+"Length";
									break;
								case "System.Byte":
									TotalBufferSizeStatic += 3;
									break;
								case "System.SByte":
									TotalBufferSizeStatic += 4;
									break;
								case "System.Int16":
									TotalBufferSizeStatic += 6;
									break;
								case "System.Int32":
									TotalBufferSizeStatic += 11;
									break;
								case "System.Int64":
									TotalBufferSizeStatic += 21;
									break;
								case "System.UInt16":
									TotalBufferSizeStatic += 5;
									break;
								case "System.UInt32":
									TotalBufferSizeStatic += 10;
									break;
								case "System.UInt64":
									TotalBufferSizeStatic += 20;
									break;
								case "System.Single":
									TotalBufferSizeStatic += 40;
									break;
								case "System.Double":
									TotalBufferSizeStatic += 50;
									break;
								case "System.DateTime":
									TotalBufferSizeStatic += 20;
									break;
							}
						}
					}
					
					//cs.Append("	dxx = "+TotalBufferSizeDynamic+"+"+TotalBufferSizeStatic+";"+cl);
					//cs.Append("	buffer = (char*)malloc(dxx);"+cl);

                    cs.Append("	len = " + TotalBufferSizeDynamic + "+" + TotalBufferSizeStatic + ";" + cl);
                    cs.Append("	if ((buffer = (char*)malloc(len)) == NULL) ILIBCRITICALEXIT(254);" + cl);

					string bufferString = this.PrintfTransform(A.Name+" xmlns:u=\"%s\">");
					foreach(UPnPArgument Arg in A.Arguments)
					{
						if (Arg.Direction=="in")
						{
							bufferString += "<"+Arg.Name+">"+ToSPrintfType(Arg.RelatedStateVar.GetNetType().FullName)+"</"+Arg.Name+">";
						}
					}
					cs.Append("	SoapBodyTemplate = \"%s" + bufferString + "</u:"+A.Name+"%s\";"+cl);

					cs.Append("	bufferLength = snprintf(buffer, len, SoapBodyTemplate, UPNPCP_SOAP_BodyHead, service->ServiceType");
					foreach(UPnPArgument Arg in A.Arguments)
					{
						if (Arg.Direction=="in")
						{
							if (Arg.RelatedStateVar.GetNetType()==typeof(DateTime) || Arg.RelatedStateVar.GetNetType()==typeof(bool) || Arg.RelatedStateVar.GetNetType()==typeof(byte[]))
							{
								cs.Append(", __"+Arg.Name);
							}
							else
							{
								cs.Append(", "+Arg.Name);
							}
						}
					}
					cs.Append(", UPNPCP_SOAP_BodyTail);"+cl);

					cs.Append("	LVL3DEBUG(printf(\"\\r\\n SOAP Sent: \\r\\n%s\\r\\n\",buffer);)"+cl);


					foreach(UPnPArgument Arg in A.Arguments)
					{
						if (Arg.Direction=="in")
						{
							if (Arg.RelatedStateVar.GetNetType()==typeof(DateTime) || Arg.RelatedStateVar.GetNetType()==typeof(byte[]))
							{
								cs.Append("	free(__"+Arg.Name+");"+cl);
							}
						}
					}
					foreach(UPnPArgument Arg in A.Arguments)
					{
						if (Arg.Direction=="in" && Arg.RelatedStateVar.ComplexType!=null)
						{
							// Need to Serialize complex type to XML
							cs.Append("	free(" + Arg.Name + ");"+cl);
						}
					}
					if (!SConf.Actions_ManualEscape.Contains(A))
					{
						foreach(UPnPArgument Arg in A.Arguments)
						{
							if (Arg.Direction=="in" && Arg.RelatedStateVar.GetNetType()==typeof(string))
							{
								cs.Append("	free(" + Arg.Name + ");"+cl);
							}
						}
					}
					cs.Append(cl);
					cs.Append("	"+pc_methodPrefix+"AddRef(service->Parent);"+cl);
					cs.Append("	"+pc_methodLibPrefix+"ParseUri(service->ControlURL, &IP, &Port, &Path, NULL);"+cl);
					cs.Append(cl);
					
					cs.Append("	len = " + (10+A.Name.Length+96+15+UseInfoString.Length) + " + (int)strlen("+this.pc_methodPrefix+"PLATFORM) + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType);"+cl);
                    cs.Append("	if ((headerBuffer = (char*)malloc(len)) == NULL) ILIBCRITICALEXIT(254);" + cl);
                    //cs.Append(" dxx = " + (A.Name.Length+96) + " + (int)strlen(service->ServiceType) + (int)strlen(Path) + (int)strlen(IP) + (int)strlen(service->ServiceType);");
					//cs.Append("	headerBuffer = (char*)malloc(dxx);"+cl);
					cs.Append("	headerLength = snprintf(headerBuffer, len, UPNPCP_SOAP_Header, Path, IP, Port, "+this.pc_methodPrefix+"PLATFORM, service->ServiceType, \""+A.Name+"\", bufferLength);"+cl);

					cs.Append(cl);
					cs.Append("	invoke_data->CallbackPtr = (voidfp)CallbackPtr;"+cl);
					cs.Append("	invoke_data->User = user;"+cl);
					cs.Append("	//"+this.pc_methodLibPrefix+"WebClient_SetQosForNextRequest(((struct "+this.pc_methodPrefix+"CP*)service->Parent->CP)->HTTP, "+this.pc_methodPrefix+"InvocationPriorityLevel);"+cl);
					cs.Append("	"+this.pc_methodLibPrefix+"WebClient_PipelineRequestEx("+cl);
					cs.Append("		((struct "+this.pc_methodPrefix+"CP*)service->Parent->CP)->HTTP,"+cl);
                    cs.Append("		(struct sockaddr*)&(service->Parent->LocationAddr)," + cl);
					cs.Append("		headerBuffer,"+cl);
					cs.Append("		headerLength,"+cl);
					cs.Append("		0,"+cl);
					cs.Append("		buffer,"+cl);
					cs.Append("		bufferLength,"+cl);
					cs.Append("		0,"+cl);
					cs.Append("		&"+pc_methodPrefix+"Invoke_"+name+"_"+A.Name+"_Sink,"+cl);
					cs.Append("		service,"+cl);
					cs.Append("		invoke_data);"+cl);
					
					cs.Append(cl);
					cs.Append("	free(IP);"+cl);
					cs.Append("	free(Path);"+cl);
					cs.Append("}"+cl);
					#endregion
				}
			}
			WS = WS.Replace("//{{{Invocation_Methods}}}",cs.ToString());
			#endregion

			#region Custom XML Tags in Device Description
			#region Custom Processing
			cs = new CodeProcessor(new StringBuilder(),this.Language == LANGUAGES.CPP);
			OkToDo = false;
			foreach(object[] foo in this.CustomTagList)
			{
				PP[0] = (string)foo[0];

				string FieldName;
				string FieldNameSpace = (string)foo[1];
				
				if (PP.DCOUNT()==1)
				{
					FieldName = PP[1];
				}
				else
				{
					FieldName = PP[2];
				}
				OkToDo = true;

				cs.Append("if (xml->NameLength == "+FieldName.Length.ToString()+" && memcmp(xml->Name, \""+FieldName+"\", "+FieldName.Length.ToString()+") == 0)"+cl);
				cs.Append("{"+cl);
				cs.Append("	tempString = ILibXML_LookupNamespace(xml, xml->NSTag, xml->NSLength);"+cl);
				cs.Append("	if (strcmp(tempString,\""+FieldNameSpace+"\")==0)"+cl);
				cs.Append("	{"+cl);
				cs.Append("		tempStringLength = ILibReadInnerXML(xml,&tempString);"+cl);
                cs.Append("		if ((tempString2 = (char*)malloc(tempStringLength + 1)) == NULL) ILIBCRITICALEXIT(254);" + cl);
				cs.Append("		memcpy(tempString2, tempString, tempStringLength);"+cl);
				cs.Append("		tempString2[tempStringLength] = 0;"+cl);
				cs.Append("		if (ILibGetEntry(device->CustomTagTable, \""+FieldNameSpace+"\", "+FieldNameSpace.Length.ToString()+") == NULL)"+cl);
				cs.Append("		{"+cl);
				cs.Append("			ILibAddEntry(device->CustomTagTable, \""+FieldNameSpace+"\", "+FieldNameSpace.Length.ToString()+", ILibInitHashTree());"+cl);
				cs.Append("		}"+cl);
				cs.Append("		ILibAddEntry(ILibGetEntry(device->CustomTagTable, \""+FieldNameSpace+"\", "+FieldNameSpace.Length.ToString()+"), xml->Name, xml->NameLength, tempString2);"+cl);
				cs.Append("	}"+cl);
				cs.Append("} "+cl);
				cs.Append("else"+cl);
			}

			WS = WS.Replace("//{{{CustomXMLTags}}}",cs.ToString());
			if (OkToDo)
			{
				WS = SourceCodeRepository.RemoveTag("//{{{BEGIN_CustomTagSpecific}}}","//{{{END_CustomTagSpecific}}}",WS);
			}
			else
			{
				WS = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_CustomTagSpecific}}}","//{{{END_CustomTagSpecific}}}",WS);
			}
			#endregion
			#region HasXXX
			cs = new CodeProcessor(new StringBuilder(),this.Language == LANGUAGES.CPP);
			foreach(object[] foo in this.CustomTagList)
			{
				PP[0] = (string)foo[0];
				string FieldName;
				string FieldNameSpace = (string)foo[1];
				
				if (PP.DCOUNT()==1)
				{
					FieldName = PP[1];
				}
				else
				{
					FieldName = PP[2];
				}

				cs.Append("	/*! \\fn UPnPGetCustomXML_" + FieldName + "(struct UPnPDevice *d)"+cl);
				cs.Append("	\\brief Obtains the meta data associated with \\a "+FieldName.ToUpper()+" and "+FieldName.ToUpper()+"_NAMESPACE"+cl);
				cs.Append("	\\param d The UPnPDevice to query"+cl);
				cs.Append("	\\returns The associated meta-data. NULL if this tag was not present on the device"+cl);
				cs.Append("*/"+cl);
				cs.Append("char* UPnPGetCustomXML_" + FieldName + "(struct UPnPDevice *d)"+cl);
				cs.Append("{"+cl);
				cs.Append("	return(UPnPGetCustomTagFromDevice(d,"+FieldName.ToUpper()+"_NAMESPACE, "+FieldName.ToUpper()+"));"+cl);
				cs.Append("}"+cl);				
			}
			WS = WS.Replace("//{{{CustomXMLTags2}}}",cs.ToString());
			#endregion
			#endregion

			#region Complex Types
			cs = new CodeProcessor(new StringBuilder(),this.Language == LANGUAGES.CPP);
			EmbeddedCGenerator.BuildComplexTypeParser(SequenceTable,ChoiceTable,cs,SL,this.pc_methodPrefix,this.pc_methodLibPrefix);
			BuildComplexTypeSerializer(SequenceTable,ChoiceTable,cs,SL,this.pc_methodPrefix,this.pc_methodLibPrefix);
			WS = WS.Replace("//{{{UPnPComplexTypes}}}",cs.ToString());
			#endregion

			#region Version Information
			if (!Configuration.HTTP_1dot1)
			{
				WS = WS.Replace("!HTTPVERSION!","1.0");
				WS = SourceCodeRepository.RemoveAndClearTag("//{{{ REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT--> }}}","//{{{ <--REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT }}}",WS);
			}
			else
			{
				WS = WS.Replace("!HTTPVERSION!","1.1");
				WS = SourceCodeRepository.RemoveTag("//{{{ REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT--> }}}","//{{{ <--REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT }}}",WS);
			}
			WS = WS.Replace("!MICROSTACKVERSION!",this.UseVersion);
			if (device.ArchitectureVersion=="1.0")
			{
				WS = WS.Replace("!UPNPVERSION!","1.0");
			}
			else
			{
				WS = WS.Replace("!UPNPVERSION!","1.1");
			}
			#endregion

			#region Prefixes
			WS = this.FixPrefix_DeviceService(device,WS);
			WS = WS.Replace("UPnP/","upnp/");
			WS = WS.Replace("UPnPControlPointStructs.h","upnpcontrolpointstructs.h");
			WS = WS.Replace("UPnPDevice","upnpdevice");
			WS = WS.Replace("UPnPService","upnpservice");
			WS = WS.Replace("UPnPAction","upnpaction");
			WS = WS.Replace("UPnPStateVariable","upnpstatevariable");
			WS = WS.Replace("UPnPAllowedValue","upnpallowedvalue");
			WS = WS.Replace("DeviceDescriptionInterruptSink","devicedescriptioninterruptsink");
			WS = WS.Replace("DeviceExpired","deviceexpired");
			WS = WS.Replace("SubscribeForUPnPEvents","subscribeforupnpevents");
			WS = WS.Replace("UnSubscribeUPnPEvents","unsubscribeupnpevents");
			WS = WS.Replace("UPnPSSDP_NOTIFY","upnpssdpnotify");
			WS = WS.Replace("UPnPSSDP_MSEARCH","upnpssdpmsearch");
			WS = WS.Replace("UPnPSSDP_MESSAGE","upnpssdpmessage");
			WS = WS.Replace(" UPnP "," _upnp_ ");
			WS = WS.Replace(" UPnPIcon","_upnpicon_");

			WS = WS.Replace("UPnP",this.pc_methodPrefix);
            WS = WS.Replace("\"" + this.pc_methodPrefix + "Error\"", "\"UPnPError\""); // There is one case where we have to turn the string back to "UPnPError", canceling the change made the line before
			WS = WS.Replace("ILib",this.pc_methodLibPrefix);

			WS = WS.Replace(" _upnp_ "," UPnP ");
			WS = WS.Replace("upnp/","UPnP/");
			WS = WS.Replace("upnpaction","UPnPAction");
			WS = WS.Replace("upnpstatevariable","UPnPStateVariable");
			WS = WS.Replace("upnpallowedvalue","UPnPAllowedValue");
			WS = WS.Replace("devicedescriptioninterruptsink","DeviceDescriptionInterruptSink");
			WS = WS.Replace("deviceexpired","deviceexpired");
			WS = WS.Replace("upnpdevice","UPnPDevice");
			WS = WS.Replace("upnpservice","UPnPService");
			WS = WS.Replace("subscribeforupnpevents","SubscribeForUPnPEvents");
			WS = WS.Replace("unsubscribeupnpevents","UnSubscribeUPnPEvents");
			WS = WS.Replace("upnpcontrolpointstructs.h","UPnPControlPointStructs.h");
			WS = WS.Replace("upnpssdpnotify","UPnPSSDP_NOTIFY");
			WS = WS.Replace("upnpssdpmsearch","UPnPSSDP_MSEARCH");
			WS = WS.Replace("upnpssdpmessage","UPnPSSDP_MESSAGE");
			WS = WS.Replace("_upnpicon_"," UPnPIcon");
			WS = this.FixPrefix2_DeviceService(device,WS);


			#endregion

			#region Reformat String
			WS = CodeProcessor.ProcessCode(WS,Indent);
			#endregion

			#region Write to disk

			writer = File.CreateText(outputDirectory.FullName + "\\"+pc_methodPrefix+"ControlPoint.c");
			writer.Write(WS);
			writer.Close();
			#endregion

			#endregion


			#region Sample Application

			#region Main.c / SampleProjectDlg.cpp
			if (SampleApp!=null)
			{
				WS = SampleApp;

				#region ControlPoint Variable
				WS = WS.Replace("//{{{MICROSTACK_VARIABLE}}}","//{{{MICROSTACK_VARIABLE}}}"+cl+"void *UPnP_CP = NULL;");
				#endregion
				#region CreateControlPoint
				if (!Configuration.BareBonesSample)
				{
					tmp = "\tUPnP_CP = UPnPCreateControlPoint(MicroStackChain,&UPnPDeviceDiscoverSink,&UPnPDeviceRemoveSink);"+cl;
					tmp = tmp.Replace("UPnP",((ServiceGenerator.Configuration)device.User).Prefix);
					WS = SourceCodeRepository.InsertTextBeforeTag(WS,"//{{{CreateControlPoint}}}",tmp);
				}
				#endregion
				#region ControlPoint Includes
				WS = SourceCodeRepository.InsertTextBeforeTag(WS,"//{{{MicroStack_Include}}}","#include \""+this.pc_methodPrefix+"ControlPoint.h\""+cl);
				#endregion
				#region Discover/Remove Sink
				if (!Configuration.BareBonesSample)
				{
					tmp = SourceCodeRepository.GetTextBetweenTags(WS,"//{{{BEGIN_CP_DISCOVER_REMOVE_SINK}}}","//{{{END_CP_DISCOVER_REMOVE_SINK}}}");
					tmp = tmp.Replace("{{{PREFIX}}}", this.pc_methodPrefix);
					#region Sample Invocations
					cs = new CodeProcessor(new StringBuilder(), this.Language == LANGUAGES.CPP);
                    cs.ident = 1;
					Build_SampleInvoke(device, cs, serviceNames);
					tmp = tmp.Replace("//{{{CP_SampleInvoke}}}", cs.ToString());
					#endregion
					WS = SourceCodeRepository.InsertTextBeforeTag(WS,"//{{{BEGIN_CP_DISCOVER_REMOVE_SINK}}}",tmp);
				}
				#endregion

				#region IPAddress Monitor
				if (Configuration.DefaultIPAddressMonitor)
				{
					WS = WS.Replace("//{{{IPAddress_Changed}}}","//{{{IPAddress_Changed}}}"+cl+"    UPnP_CP_IPAddressListChanged(UPnP_CP);");

					if (Configuration.TargetPlatform==ServiceGenerator.PLATFORMS.MICROSTACK_WINSOCK2)
					{
						WS = SourceCodeRepository.RemoveTag("//{{{BEGIN_WINSOCK2_IPADDRESS_MONITOR}}}","//{{{END_WINSOCK2_IPADDRESS_MONITOR}}}",WS);
						WS = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_POSIX/WINSOCK1_IPADDRESS_MONITOR}}}","//{{{END_POSIX/WINSOCK1_IPADDRESS_MONITOR}}}",WS);
					}
					else
					{
						WS = SourceCodeRepository.RemoveTag("//{{{BEGIN_POSIX/WINSOCK1_IPADDRESS_MONITOR}}}","//{{{END_POSIX/WINSOCK1_IPADDRESS_MONITOR}}}",WS);
						WS = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_WINSOCK2_IPADDRESS_MONITOR}}}","//{{{END_WINSOCK2_IPADDRESS_MONITOR}}}",WS);				
					}
				}
				else
				{
					WS = WS.Replace("//{{{IPAddress_Changed}}}","");
					WS = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_WINSOCK2_IPADDRESS_MONITOR}}}","//{{{END_WINSOCK2_IPADDRESS_MONITOR}}}",WS);
					WS = SourceCodeRepository.RemoveAndClearTag("//{{{BEGIN_POSIX/WINSOCK1_IPADDRESS_MONITOR}}}","//{{{END_POSIX/WINSOCK1_IPADDRESS_MONITOR}}}",WS);
				}
				#endregion

				#region Invoke Sink
				if (!Configuration.BareBonesSample)
				{
					cs = new CodeProcessor(new StringBuilder(),this.Language == LANGUAGES.CPP);
					en = SL.GetEnumerator();
					while(en.MoveNext())
					{
						UPnPService s = (UPnPService)en.Value;
						string name = (string)en.Key;
						foreach(UPnPAction A in s.Actions)
						{
							#region ResponseSink
							#region Header
							cs.Append("void "+pc_methodPrefix+"ResponseSink_"+name+"_"+A.Name+"(struct UPnPService* Service, int ErrorCode, void *User");
							if (A.HasReturnValue)
							{
								cs.Append(", "+ToCType(A.GetRetArg().RelatedStateVar.GetNetType().FullName)+" "+A.GetRetArg().Name);
								if (A.GetRetArg().RelatedStateVar.GetNetType()==typeof(Byte[]))
								{
									cs.Append(", int "+A.GetRetArg().Name+"Length");
								}
							}
							foreach(UPnPArgument Arg in A.Arguments)
							{
								if (!Arg.IsReturnValue && Arg.Direction=="out")
								{
									cs.Append(","+ToCType(Arg.RelatedStateVar.GetNetType().FullName)+" "+Arg.Name);
									if (Arg.RelatedStateVar.GetNetType()==typeof(Byte[]))
									{
										cs.Append(", int " + Arg.Name+"Length");
									}
								}
							}
							cs.Append(")"+cl);
							#endregion
							#region Body
							cs.Append("{"+cl);
                            cs.Append(" UNREFERENCED_PARAMETER( Service );" + cl);
                            cs.Append(" UNREFERENCED_PARAMETER( User );" + cl);
                            cs.Append(cl);

							if (Configuration.TargetPlatform==ServiceGenerator.PLATFORMS.MICROSTACK_POCKETPC)
							{
								#region PocketPC Specific
								cs.Append("	CString display;"+cl);
								if (A.HasReturnValue && (A.GetRetArg().RelatedStateVar.GetNetType()==typeof(string)||
									A.GetRetArg().RelatedStateVar.GetNetType()==typeof(Uri)))
								{
									cs.Append("	wchar_t *wc_"+A.GetRetArg().Name+"=NULL;"+cl);
									cs.Append("	int wc_"+A.GetRetArg().Name+"Length=0;"+cl);
								}
								foreach (UPnPArgument Arg in A.Arguments)
								{
									if (Arg.Direction=="out" && !Arg.IsReturnValue && (Arg.RelatedStateVar.GetNetType()==typeof(string)||
										Arg.RelatedStateVar.GetNetType()==typeof(Uri)))
									{
										cs.Append("	wchar_t *wc_"+Arg.Name+" = NULL;"+cl);
										cs.Append("	int wc_"+Arg.Name+"Length = 0;"+cl);
									}
								}
								cs.Append(cl);
								foreach (UPnPArgument Arg in A.Arguments)
								{
									if (Arg.Direction=="out" && (Arg.RelatedStateVar.GetNetType()==typeof(string)||
										Arg.RelatedStateVar.GetNetType()==typeof(Uri)))
									{
										cs.Append("	if ("+Arg.Name+" != NULL)"+cl);
										cs.Append("	{"+cl);
										cs.Append("		wc_"+Arg.Name+"Length = MultiByteToWideChar(CP_UTF8, 0, "+Arg.Name+", -1, wc_"+Arg.Name+", 0);"+cl);
                                        cs.Append("		if ((wc_" + Arg.Name + " = (wchar_t*)malloc(sizeof(wchar_t)*wc_" + Arg.Name + "Length)) == NULL) ILIBCRITICALEXIT(254);" + cl);
										cs.Append("		MultiByteToWideChar(CP_UTF8, 0, "+Arg.Name+", -1, wc_"+Arg.Name+", wc_"+Arg.Name+"Length);"+cl);
										cs.Append("	}"+cl);
									}
								}
								cs.Append(cl);
							}
							#endregion

							if (Configuration.TargetPlatform==ServiceGenerator.PLATFORMS.MICROSTACK_POCKETPC)
							{
								cs.Append("	 display.Format(_T(\""+pc_methodPrefix+" Invoke Response: "+ name + "/" + A.Name + "[ErrorCode:%d](");
							}
							else
							{
								cs.Append("	 printf(\""+pc_methodPrefix+" Invoke Response: "+ name + "/" + A.Name + "[ErrorCode:%d](");
							}
							bool firstag = true;
							if (A.HasReturnValue)
							{
								cs.Append(ToPrintfType(A.GetRetArg().RelatedStateVar.GetNetType().FullName));
								firstag = false;
							}
							foreach(UPnPArgument Arg in A.Arguments)
							{
								if (!Arg.IsReturnValue && Arg.Direction=="out")
								{
									if (firstag==false)
									{
										cs.Append(",");
									}
									cs.Append(ToPrintfType(Arg.RelatedStateVar.GetNetType().FullName));
									firstag = false;
								}
							}
							if (Configuration.TargetPlatform==ServiceGenerator.PLATFORMS.MICROSTACK_POCKETPC)
							{
								cs.Append(")\\r\\n\")");
							}
							else
							{
								cs.Append(")\\r\\n\"");
							}
							cs.Append(",ErrorCode");
							if (A.HasReturnValue == true)
							{
								if (	Configuration.TargetPlatform==ServiceGenerator.PLATFORMS.MICROSTACK_POCKETPC &&
									(A.GetRetArg().RelatedStateVar.GetNetType()==typeof(string)||
									A.GetRetArg().RelatedStateVar.GetNetType()==typeof(Uri)))
								{
									cs.Append(",wc_" + A.GetRetArg().Name);
								}
								else
								{
									cs.Append("," + A.GetRetArg().Name);
								}
							}
							foreach (UPnPArgument Arg in A.Arguments)
							{
								if (!Arg.IsReturnValue && Arg.Direction=="out")
								{
									if (Configuration.TargetPlatform==ServiceGenerator.PLATFORMS.MICROSTACK_POCKETPC &&
										(Arg.RelatedStateVar.GetNetType()==typeof(string)||
										Arg.RelatedStateVar.GetNetType()==typeof(Uri)))
									{
										cs.Append(",wc_" + Arg.Name);
									}
									else
									{
										cs.Append("," + Arg.Name);
									}
								}
							}
							cs.Append(");"+cl);

							if (Configuration.TargetPlatform==ServiceGenerator.PLATFORMS.MICROSTACK_POCKETPC)
							{
								cs.Append("	if (that->m_Text.GetLength() > 16384)"+cl);
								cs.Append("	{"+cl);
								cs.Append("		that->m_Text = display;"+cl);
								cs.Append("	}"+cl);
								cs.Append("	else"+cl);
								cs.Append("	{"+cl);
								cs.Append("		that->m_Text += display;"+cl);
								cs.Append("	}"+cl);
								cs.Append("	that->SendMessage(WM_USER_UPDATE);"+cl);
						
								foreach (UPnPArgument Arg in A.Arguments)
								{
									if (Arg.Direction=="out" && (Arg.RelatedStateVar.GetNetType()==typeof(string)||
										Arg.RelatedStateVar.GetNetType()==typeof(Uri)))
									{
										cs.Append("	if (wc_"+Arg.Name+"!=NULL) {free(wc_"+Arg.Name+");}"+cl);
									}
								}
							}

							cs.Append("}"+cl);
							cs.Append(cl);
							#endregion
							#endregion
						}
					}
					WS = WS.Replace("//{{{CP_InvokeSink}}}","//{{{CP_InvokeSink}}}"+cl+cs.ToString());
				}
				#endregion
				#region Event Sink
				if (!Configuration.BareBonesSample)
				{
					cs = new CodeProcessor(new StringBuilder(),this.Language == LANGUAGES.CPP);
					en = SL.GetEnumerator();
					while(en.MoveNext())
					{
						UPnPService s = (UPnPService)en.Value;
						string name = (string)en.Key;
						foreach(UPnPStateVariable variable in s.GetStateVariables()) 
						{
							if (variable.SendEvent == true) 
							{
								#region Event Sink
								cs.Append("void "+pc_methodPrefix+"EventSink_"+name+"_"+variable.Name+"(struct UPnPService* Service,");
								cs.Append(ToCType(variable.GetNetType().FullName)+" "+variable.Name);
								if (variable.GetNetType()==typeof(byte[]))
								{
									cs.Append(",int " + variable.Name+"Length");
								}
								cs.Append(")"+cl);
								cs.Append("{"+cl);
								if (Configuration.TargetPlatform==ServiceGenerator.PLATFORMS.MICROSTACK_POCKETPC)
								{
									#region PocketPC Specific
									cs.Append("	CString display;"+cl);
									cs.Append("	wchar_t *FriendlyName = NULL;"+cl);
									cs.Append("	int FriendlyNameLength = 0;"+cl);
									if (variable.GetNetType()==typeof(string)||
										variable.GetNetType()==typeof(Uri))
									{
										cs.Append("	wchar_t *wc_var = NULL;"+cl);
										cs.Append("	int wc_varLength = 0;"+cl);
									}

									cs.Append(cl);
									cs.Append("	if (Service->Parent->FriendlyName!=NULL)"+cl);
									cs.Append("	{"+cl);
									cs.Append("		FriendlyNameLength = MultiByteToWideChar(CP_UTF8, 0, Service->Parent->FriendlyName, -1, FriendlyName, 0);"+cl);
                                    cs.Append("		if ((FriendlyName = (wchar_t*)malloc(sizeof(wchar_t)*FriendlyNameLength)) == NULL) ILIBCRITICALEXIT(254);" + cl);
									cs.Append("		MultiByteToWideChar(CP_UTF8, 0, Service->Parent->FriendlyName, -1, FriendlyName, FriendlyNameLength);"+cl);
									cs.Append("	}"+cl);
									#endregion
								}

								if (variable.GetNetType()==typeof(byte[]))
								{
									if (Configuration.TargetPlatform==ServiceGenerator.PLATFORMS.MICROSTACK_POCKETPC)
									{
										cs.Append("	 display.Format(_T(\""+pc_methodPrefix+" Event from %s/"+name+"/"+variable.Name+": [BINARY:%d]\\r\\n\"),FriendlyName, "+variable.Name+"Length);"+cl);
									}
									else
									{
										cs.Append("	 printf(\""+pc_methodPrefix+" Event from %s/"+name+"/"+variable.Name+": [BINARY:%d]\\r\\n\", Service->Parent->FriendlyName, "+variable.Name+"Length);"+cl);
									}
								}
								else if (variable.GetNetType()==typeof(string)||variable.GetNetType()==typeof(Uri))
								{
									cs.Append("	if ("+variable.Name+"!=NULL)"+cl);
									cs.Append("	{"+cl);
									if (Configuration.TargetPlatform==ServiceGenerator.PLATFORMS.MICROSTACK_POCKETPC)
									{
										cs.Append("		wc_varLength = MultiByteToWideChar(CP_UTF8, 0, "+variable.Name+", -1, wc_var, 0);"+cl);
                                        cs.Append("		if ((wc_var = (wchar_t*)malloc(sizeof(wchar_t)*wc_varLength)) == NULL) ILIBCRITICALEXIT(254);" + cl);
										cs.Append("		MultiByteToWideChar(CP_UTF8,0,"+variable.Name+",-1,wc_var,wc_varLength);"+cl);
										cs.Append("		display.Format(_T(\""+pc_methodPrefix+" Event from %s/"+name+"/"+variable.Name+": %s\\r\\n\"), FriendlyName, wc_var);"+cl);							
									}
									else
									{
										cs.Append("		printf(\""+pc_methodPrefix+" Event from %s/"+name+"/"+variable.Name+": %s\\r\\n\", Service->Parent->FriendlyName, "+variable.Name+");"+cl);							
									}
									cs.Append("	}"+cl);
								}
								else
								{
									if (Configuration.TargetPlatform==ServiceGenerator.PLATFORMS.MICROSTACK_POCKETPC)
									{
										cs.Append("	 display.Format(_T(\""+pc_methodPrefix+" Event from %s/"+name+"/"+variable.Name+": " + ToPrintfType(variable.GetNetType().FullName) + "\\r\\n\"),FriendlyName,"+variable.Name+");"+cl);
									}
									else
									{
										cs.Append("	 printf(\""+pc_methodPrefix+" Event from %s/"+name+"/"+variable.Name+": " + ToPrintfType(variable.GetNetType().FullName) + "\\r\\n\",Service->Parent->FriendlyName,"+variable.Name+");"+cl);
									}
								}
								if (Configuration.TargetPlatform==ServiceGenerator.PLATFORMS.MICROSTACK_POCKETPC)
								{
									#region PocketPC Specific
									cs.Append("	if (that->m_Text.GetLength() > 16384)"+cl);
									cs.Append("	{"+cl);
									cs.Append("		that->m_Text = display;"+cl);
									cs.Append("	}"+cl);
									cs.Append("	else"+cl);
									cs.Append("	{"+cl);
									cs.Append("		that->m_Text += display;"+cl);
									cs.Append("	}"+cl);
									cs.Append("	that->SendMessage(WM_USER_UPDATE);"+cl);
									cs.Append("	if (FriendlyName!=NULL) {free(FriendlyName);}"+cl);
									if (variable.GetNetType()==typeof(string)||variable.GetNetType()==typeof(Uri))
									{
										cs.Append("	if (wc_var!=NULL) {free(wc_var);}"+cl);
									}
									#endregion
								}
								cs.Append("}"+cl);
								cs.Append(cl);
								#endregion
							}
						}
					}
					WS = WS.Replace("//{{{CP_EventSink}}}","//{{{CP_EventSink}}}"+cl+cs.ToString());
				}
				#endregion
				#region Event Sink Registrations
				if (!Configuration.BareBonesSample)
				{
					cs = new CodeProcessor(new StringBuilder(),this.Language == LANGUAGES.CPP);


					en = SL.GetEnumerator();
					while(en.MoveNext())
					{
						UPnPService s = (UPnPService)en.Value;
						string name = (string)en.Key;
						foreach(UPnPStateVariable variable in s.GetStateVariables()) 
						{
							if (variable.SendEvent == true) 
							{
								cs.Append("    " + pc_methodPrefix+"EventCallback_"+name+"_"+variable.Name+" = &"+pc_methodPrefix+"EventSink_"+name+"_"+variable.Name+";" + cl);
							}
						}
					}


					WS = WS.Replace("//{{{CP_EventRegistrations}}}","//{{{CP_EventRegistrations}}}"+cl+cs.ToString());
				}
				#endregion

				#region C++ Specific
				if (Configuration.CPlusPlusWrapper)
				{
					// Set the Callback
					string CPSINK = "pUPnP->Set_ControlPoint_Handler_"+device.User2.ToString()+"(&"+pc_methodPrefix+"OnAddSink,&"+pc_methodPrefix+"OnRemoveSink);";
					WS = SourceCodeRepository.InsertTextBeforeTag(WS,"//{{{CP_CONNECTING_ADD/REMOVE_SINKS}}}",CPSINK);
			
					// Add the Sinks
					tmp = SourceCodeRepository.GetTextBetweenTags(WS,"//{{{CP_DISCOVER/REMOVE_SINKS_BEGIN}}}","//{{{CP_DISCOVER/REMOVE_SINKS_END}}}");
					tmp = tmp.Replace("{{{PREFIX}}}",this.pc_methodPrefix);
					tmp = tmp.Replace("{{{DEVICE}}}",device.User2.ToString());
					WS = SourceCodeRepository.InsertTextBeforeTag(WS,"//{{{CP_DISCOVER/REMOVE_SINKS_BEGIN}}}",tmp);
				}
				#endregion

				#region Prefixes

				WS = WS.Replace("UPnPAbstraction.h", "_upnpabstraction.h_");
				WS = WS.Replace("pUPnP","_pupnp_");
				WS = WS.Replace("CUPnP_","_cupnp_");

				WS = WS.Replace("UPnP/","upnp/");
				WS = WS.Replace("UPnPControlPointStructs.h","upnpcontrolpointstructs.h");
				WS = WS.Replace("UPnPDevice","upnpdevice");
				WS = WS.Replace("UPnPService","upnpservice");
				WS = WS.Replace("SubscribeForUPnPEvents","subscribeforupnpevents");
				WS = WS.Replace("UnSubscribeUPnPEvents","unsubscribeupnpevents");

				WS = WS.Replace("UPnPError","_upnperror_");
				WS = WS.Replace(" UPnP "," _upnp_ ");
				WS = WS.Replace("UPnP",this.pc_methodPrefix);
				WS = WS.Replace("ILib",this.pc_methodLibPrefix);
				WS = WS.Replace("_upnperror_","UPnPError");
				WS = WS.Replace("upnp/","UPnP/");
				WS = WS.Replace(" _upnp_ ", " UPnP ");

				WS = WS.Replace("_pupnp_","pUPnP");
				WS = WS.Replace("_cupnp_","CUPnP_");

				WS = WS.Replace("upnpdevice","UPnPDevice");
				WS = WS.Replace("upnpservice","UPnPService");
				WS = WS.Replace("upnpcontrolpointstructs.h","UPnPControlPointStructs.h");
				WS = WS.Replace("subscribeforupnpevents","SubscribeForUPnPEvents");
				WS = WS.Replace("unsubscribeupnpevents","UnSubscribeUPnPEvents");
				WS = WS.Replace("_upnpabstraction.h_","UPnPAbstraction.h");


				#endregion

				SampleApp = WS;
			}
			#endregion
			#endregion

			Log("UPnP Stack Generation Complete.");
			return true;
		}

		private void TypeCheckURI(CodeProcessor cs, UPnPArgument args)
		{
			cs.Append("	TempParser = "+pc_methodLibPrefix+"ParseString(p_" + args.Name + ", 0, p_" + args.Name + "Length, \"://\",3);"+cl);
			cs.Append("	if (TempParser->NumResults != 2)"+cl);
			cs.Append("	{"+cl);
			cs.Append("		"+pc_methodPrefix+"Response_Error(ReaderObject, 402, \"Argument[" + args.Name + "] illegal format\");"+cl);
			cs.Append("		return;"+cl);
			cs.Append("	}"+cl);
			cs.Append("	else"+cl);
			cs.Append("	{"+cl);
			cs.Append("		_" + args.Name + " = p_" + args.Name + ";"+cl);
			cs.Append("		_" + args.Name + "Length = p_" + args.Name + "Length;"+cl);
			cs.Append("	}"+cl);	
		}

		private void TypeCheckBoolean(CodeProcessor cs, UPnPArgument args)
		{
			cs.Append("	OK=0;"+cl);
			cs.Append("	if (p_" + args.Name + "Length == 4)"+cl);
			cs.Append("	{"+cl);
			cs.Append("		if (strncasecmp(p_" + args.Name + ", \"true\", 4) == 0)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			OK = 1;"+cl);
			cs.Append("			_" + args.Name + " = 1;"+cl);
			cs.Append("		}"+cl);
			cs.Append("	}"+cl);
			cs.Append("	if (p_" + args.Name + "Length == 5)"+cl);
			cs.Append("	{"+cl);
			cs.Append("		if (strncasecmp(p_" + args.Name + ", \"false\", 5) == 0)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			OK = 1;"+cl);
			cs.Append("			_" + args.Name + " = 0;"+cl);
			cs.Append("		}"+cl);
			cs.Append("	}"+cl);
			cs.Append("	if (p_" + args.Name + "Length == 1)"+cl);
			cs.Append("	{"+cl);
			cs.Append("		if (memcmp(p_" + args.Name + ", \"0\", 1) == 0)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			OK = 1;"+cl);
			cs.Append("			_" + args.Name + " = 0;"+cl);
			cs.Append("		}"+cl);
			cs.Append("		if (memcmp(p_" + args.Name + ", \"1\", 1) == 0)"+cl);
			cs.Append("		{"+cl);
			cs.Append("			OK = 1;"+cl);
			cs.Append("			_" + args.Name + " = 1;"+cl);
			cs.Append("		}"+cl);
			cs.Append("	}"+cl);
			cs.Append("	if (OK==0)"+cl);
			cs.Append("	{"+cl);
			cs.Append("		"+pc_methodPrefix+"Response_Error(ReaderObject, 402, \"Argument[" + args.Name + "] illegal value\");"+cl);
			cs.Append("		return;"+cl);
			cs.Append("	}"+cl);
		}
		private void TypeCheckIntegral(CodeProcessor cs, UPnPArgument args)
		{
			UPnPDebugObject obj = new UPnPDebugObject(args.RelatedStateVar.GetNetType());
			switch(args.RelatedStateVar.GetNetType().FullName)
			{
				case "System.SByte":
				case "System.Int16":
				case "System.Int32":
					cs.Append("	OK = "+pc_methodLibPrefix+"GetLong(p_" + args.Name + ",p_" + args.Name + "Length, &TempLong);"+cl);
					break;
				case "System.Byte":
				case "System.UInt16":
				case "System.UInt32":
					cs.Append("	OK = "+pc_methodLibPrefix+"GetULong(p_" + args.Name + ",p_" + args.Name + "Length, &TempULong);"+cl);
					break;
			}
			cs.Append("	if (OK != 0)"+cl);
			cs.Append("	{"+cl);
			cs.Append("		"+pc_methodPrefix+"Response_Error(ReaderObject, 402, \"Argument[" + args.Name + "] illegal value\");"+cl);
			cs.Append("		return;"+cl);
			cs.Append("	}"+cl);

			bool endtag = false;
			switch(args.RelatedStateVar.GetNetType().FullName)
			{
				case "System.SByte":
				case "System.Int16":
				case "System.Int32":
					if ((args.RelatedStateVar.GetNetType().FullName == "System.Int32") && (args.RelatedStateVar.Minimum==null && args.RelatedStateVar.Maximum==null))
					{
						// No need to check anything since this is an int without bounds.
					}
					else 
					{
						// Check lower and upper bounds.
						endtag = true;
						cs.Append("	else"+cl);
						cs.Append("	{"+cl);
						cs.Append("		if (!(TempLong >= ");
						if (args.RelatedStateVar.Minimum!=null)
						{
							cs.Append("(long)0x" + ToHex(args.RelatedStateVar.Minimum));
						}
						else
						{
							cs.Append("(long)0x" + ToHex(obj.GetStaticField("MinValue")));
						}
						cs.Append(" && TempLong <= ");
						if (args.RelatedStateVar.Maximum!=null)
						{
							cs.Append("(long)0x"+ToHex(args.RelatedStateVar.Maximum));
						}
						else
						{
							cs.Append("(long)0x"+ToHex(obj.GetStaticField("MaxValue")));
						}
						cs.Append("))"+cl);
						cs.Append("		{"+cl);
						cs.Append("		  "+pc_methodPrefix+"Response_Error(ReaderObject, 402, \"Argument[" + args.Name + "] out of Range\");"+cl);
						cs.Append("		  return;"+cl);
						cs.Append("		}"+cl);
					}
					break;
				case "System.Byte":
				case "System.UInt16":
				case "System.UInt32":
					if ((args.RelatedStateVar.GetNetType().FullName == "System.UInt32") && (args.RelatedStateVar.Minimum==null && args.RelatedStateVar.Maximum==null))
					{
						// No need to check anything since this is an int without bounds.
					}
					else 
					{
						endtag = true;
						cs.Append("	else"+cl);
						cs.Append("	{"+cl);
						cs.Append("		if (!(TempULong >= ");
						if (args.RelatedStateVar.Minimum!=null)
						{
							cs.Append("(unsigned long)0x"+ToHex(args.RelatedStateVar.Minimum));
						}
						else
						{
							cs.Append("(unsigned long)0x"+ToHex(obj.GetStaticField("MinValue")));
						}
						cs.Append(" && TempULong<=");
						if (args.RelatedStateVar.Maximum!=null)
						{
							cs.Append("(unsigned long)0x"+ToHex(args.RelatedStateVar.Maximum));
						}
						else
						{
							cs.Append("(unsigned long)0x"+ToHex(obj.GetStaticField("MaxValue")));
						}
						cs.Append("))"+cl);
						cs.Append("		{"+cl);
						cs.Append("		  "+pc_methodPrefix+"Response_Error(ReaderObject, 402, \"Argument[" + args.Name + "] out of Range\");"+cl);
						cs.Append("		  return;"+cl);
						cs.Append("		}"+cl);
					}
					break;
			}

			switch(args.RelatedStateVar.GetNetType().FullName)
			{
				case "System.SByte":
				case "System.Int16":
				case "System.Int32":
					cs.Append("	_" + args.Name + " = (" + ToCType(args.RelatedStateVar.GetNetType().FullName) + ")TempLong;"+cl);
					break;
				case "System.Byte":
				case "System.UInt16":
				case "System.UInt32":
					cs.Append("	_" + args.Name + " = (" + ToCType(args.RelatedStateVar.GetNetType().FullName) + ")TempULong;"+cl);
					break;
			}
			if (endtag == true) cs.Append(" }"+cl);
		}
		private void TypeCheckString(CodeProcessor cs, UPnPArgument args)
		{
			cs.Append("	_" + args.Name + " = p_" + args.Name + ";"+cl);
			cs.Append("	_" + args.Name + "Length = p_" + args.Name + "Length;"+cl);

			if (args.RelatedStateVar.AllowedStringValues!=null)
			{
				cs.Append("	OK = 0;"+cl);
				foreach(string val in args.RelatedStateVar.AllowedStringValues)
				{
					//cs.Append("	if (_" + args.Name + "Length == " + val.Length.ToString() + ")"+cl);
					//cs.Append("	{"+cl);
					cs.Append("		if (memcmp(_" + args.Name + ", \"" + val + "\\0\"," + (val.Length+1).ToString() + ") == 0)"+cl);
					cs.Append("		{"+cl);
					cs.Append("			OK = 1;"+cl);
					cs.Append("		}"+cl);
					//cs.Append("	}"+cl);
				}
				cs.Append("	if (OK==0)"+cl);
				cs.Append("	{"+cl);
				cs.Append("		"+pc_methodPrefix+"Response_Error(ReaderObject, 402, \"Argument[" + args.Name + "] contains a value that is not in AllowedValueList\");"+cl);
				cs.Append("		return;"+cl);
				cs.Append("	}"+cl);
			}
		}

		private void BuildSoapEvents(CodeProcessor cs, UPnPDevice device, Hashtable serviceNames)
		{
			string soap_eventblock = "<?xml version=\"1.0\" encoding=\"utf-8\"?><e:propertyset xmlns:e=\"urn:schemas-upnp-org:event-1-0\"><e:property><%s>%s</%s></e:property></e:propertyset>";
			foreach (UPnPService service in device.Services) 
			{
				int eventedStateVariables = 0;
				foreach (UPnPStateVariable statevar in service.GetStateVariables()) 
				{
					if (statevar.SendEvent == true) eventedStateVariables++;
				}

				foreach (UPnPStateVariable statevar in service.GetStateVariables()) 
				{
					if (statevar.SendEvent == true) 
					{
						string eventname = serviceNames[service] + "_" + statevar.Name;
						
						cs.DefinePublic("void "+pc_methodPrefixDef+"SetState_" + serviceNames[service] + "_" + statevar.Name + "(" + ToCType(statevar.GetNetType().ToString()) + " val)");
						cs.Append("{" + cl);

						cs.Append("  char* valstr;" + cl);
						cs.Append("  char* body;" + cl);
						cs.Append("  int bodylength;" + cl);

						// Data Type Handling Code
						switch (statevar.GetNetType().ToString()) 
						{
							case "System.Boolean":
								cs.Append("  if (val != 0) valstr = \"true\"; else valstr = \"false\";" + cl);
								break;
							case "System.Byte[]":
							case "System.Uri":
							case "System.String":
                                cs.Append("  if ((valstr = (char*)malloc((int)strlen(val) + 1)) == NULL) ILIBCRITICALEXIT(254);" + cl);
								cs.Append("  strcpy(valstr, val);" + cl);
								break;
							case "System.Byte":
							case "System.Int16":
							case "System.Int32":
                                cs.Append("  if ((valstr = (char*)malloc(10)) == NULL) ILIBCRITICALEXIT(254);" + cl);
								cs.Append("  sprintf(valstr, \"%d\", val);" + cl);
								break;
							case "System.Char":
							case "System.SByte":
							case "System.UInt16":
							case "System.UInt32":
                                cs.Append("  if ((valstr = (char*)malloc(10)) == NULL) ILIBCRITICALEXIT(254);" + cl);
								cs.Append("  sprintf(valstr, \"%u\", val);" + cl);
								break;
							case "System.Single":
							case "System.Double":
                                cs.Append("  if ((valstr = (char*)malloc(30)) == NULL) ILIBCRITICALEXIT(254);" + cl);
								cs.Append("  sprintf(valstr, \"%f\", val);" + cl);
								break;
							default:
								cs.Append("  char* valuestr = NULL;" + cl);
								break;
						}

						if (statevar.GetNetType().ToString() != "System.Boolean") 
						{
							cs.Append("  if (" + eventname + " != NULL) free("+eventname+");" + cl);
						}
						cs.Append("  " + eventname + " = valstr;" + cl);

                        cs.Append("  if ((body = (char*)malloc(" + (soap_eventblock.Length + (statevar.Name.Length * 2) + 1) + " + (int)strlen(valstr))) == NULL) ILIBCRITICALEXIT(254);" + cl);
						cs.Append("  bodylength = sprintf(body, \"" + PrintfTransform(soap_eventblock) + "\", \"" + statevar.Name + "\", valstr, \"" + statevar.Name + "\");" + cl);
						cs.Append("  "+pc_methodPrefix+"SendEvent(body, bodylength, \""+(string)serviceNames[service]+"\");" + cl);
						//cs.Append("  free(body);" + cl);

						cs.Append("}" + cl);
						cs.Append(cl);
					}
				}
				 
			}
			foreach(UPnPDevice d in device.EmbeddedDevices)
			{
				this.BuildSoapEvents(cs,d,serviceNames);
			}
		}
		private void BuildUPnPResponseHeaders(CodeProcessor cs, UPnPDevice device, Hashtable serviceNames)
		{
			foreach (UPnPService service in device.Services) 
			{
				foreach (UPnPAction action in service.Actions) 
				{
					cs.Append("void "+pc_methodPrefixDef+"Response_" + serviceNames[service] + "_" + action.Name + "(const void* UPnPToken");
					if (action.HasReturnValue)
					{
						cs.Append(", const " + ToCType(action.GetRetArg().RelatedStateVar.GetNetType().FullName) + " __ReturnValue");
						if (action.GetRetArg().RelatedStateVar.GetNetType().FullName=="System.Byte[]")
						{
							cs.Append(", const int __ReturnValueLength");
						}
					}
					foreach (UPnPArgument arg in action.Arguments) 
					{
						if (arg.Direction == "out") 
						{
							cs.Append(", const " + ToCType(arg.RelatedStateVar.GetNetType().ToString()) + " " + arg.Name);
						}
					}
					cs.Append(");" + cl);
				}
			}
			foreach(UPnPDevice d in device.EmbeddedDevices)
			{
				this.BuildUPnPResponseHeaders(cs,d,serviceNames);
			}
		}
		private void BuildStateVariableHeaders(CodeProcessor cs, UPnPDevice device, Hashtable serviceNames)
		{
			foreach (UPnPService service in device.Services) 
			{
				foreach (UPnPStateVariable statevar in service.GetStateVariables()) 
				{
					if (statevar.SendEvent == true) 
					{
						cs.Append("void "+pc_methodPrefixDef+"SetState_" + serviceNames[service] + "_" + statevar.Name + "(" + ToCType(statevar.GetNetType().ToString()) + " val);" + cl);
					}
				}
			}
			foreach(UPnPDevice d in device.EmbeddedDevices)
			{
				this.BuildStateVariableHeaders(cs,d,serviceNames);
			}
		}
	
		private void Fix(UPnPDevice device, int number, Hashtable serviceNameTable)
		{
			if (device.Root)
			{
				device.UniqueDeviceName = "%s";
			}
			else
			{
				device.UniqueDeviceName = "%s_" + number.ToString();
			}
			device.SerialNumber = "%s";
			foreach(UPnPService service in device.Services)
			{
				UPnPDebugObject obj = new UPnPDebugObject(service);
				obj.SetField("SCPDURL",(string)serviceNameTable[service] + "/scpd.xml");
				obj.SetField("__controlurl",(string)serviceNameTable[service] + "/control");
				obj.SetField("__eventurl",(string)serviceNameTable[service] + "/event");
			}

			foreach(UPnPDevice d in device.EmbeddedDevices)
			{
				Fix(d, ++number, serviceNameTable);
			}
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
			return(EmbeddedCGenerator.Static_ToCType(t));
		}

		public string ToPrintfType(string t) 
		{
			return(EmbeddedCGenerator.Static_ToPrintfType(t));
		}

		public string ToSPrintfType(string t) 
		{
			return(EmbeddedCGenerator.Static_ToSPrintfType(t));
		}

		public string ToSampleValue(string t) 
		{
			return(EmbeddedCGenerator.Static_ToSampleValue(t));
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
				case "System.DateTime":
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

		private void Build_SampleInvoke(UPnPDevice d, CodeProcessor cs, Hashtable serviceNames)
		{
			if (d.ParentDevice == null)
			{
				// RootDevice
			}
			else
			{
				// Non Root Device
			}

			foreach(UPnPService s in d.Services)
			{
				cs.Append("	tempService = "+pc_methodPrefix+"GetService_"+(string)serviceNames[s]+"(device);"+cl);
				foreach(UPnPAction A in s.Actions)
				{
					cs.Append("		"+pc_methodPrefix+"Invoke_"+(string)serviceNames[s]+"_"+A.Name+"(tempService, &"+pc_methodPrefix+"ResponseSink_" + (string)serviceNames[s] + "_" + A.Name + ",NULL");
					foreach(UPnPArgument Arg in A.Arguments)
					{
						if (Arg.Direction == "in")
						{
							cs.Append(", " + ToSampleValue(Arg.RelatedStateVar.GetNetType().FullName));
						}
					}
					cs.Append(");"+cl);
				}
				cs.Append(cl);
			}


//			foreach(UPnPDevice ed in d.EmbeddedDevices)
//			{
//				cs.Append("	embeddedDevice = "+this.pc_methodPrefix+"GetDevice(testDevice,\""+ed.DeviceURN.Substring(0,ed.DeviceURN.LastIndexOf(":"))+"\",1);"+cl);
//
//				Build_SampleInvoke(ed,cs);
//			}
		}
	}
}
