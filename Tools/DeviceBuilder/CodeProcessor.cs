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
using System.Text;

namespace UPnPStackBuilder
{
	public class CodeProcessor 
	{
		public static string cl = "\r\n";
		public string NewLine
		{
			get
			{
				return(cl);
			}
			set
			{
				cl = value;
			}
		}
		public StringBuilder SB;
		public int ident = 0;
		string temp = "";
		static public int Setting = 0;
		bool cppCommentStyle;
		public CodeProcessor ClassDefinitions;
		public CodeProcessor PublicClassDefinitions;
		public string CodeTab = "\t";

		private CodeProcessor(bool cppCommentStyle)
		{
			SB = new StringBuilder();
			this.cppCommentStyle = cppCommentStyle;
		}
		public CodeProcessor(StringBuilder sb, bool cppCommentStyle) 
		{
			SB = sb;
			this.cppCommentStyle = cppCommentStyle;
			this.ClassDefinitions = new CodeProcessor(cppCommentStyle);
			PublicClassDefinitions = new CodeProcessor(cppCommentStyle);
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

		public void Define(string code)
		{
			this.Append(code + cl);
			ClassDefinitions.Append(code + ";" + cl);
		}

		public void DefinePublic(string code)
		{
			this.Append(code + cl);
			PublicClassDefinitions.Append(code + ";" + cl);
		}

		public static string ProcessCode(string code, string tab)
		{
			CodeProcessor cs = new CodeProcessor(new StringBuilder(),true);
			cs.CodeTab = tab;
			cs.Append(code);
			return(cs.ToString());
		}
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


}
