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
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using OpenSource.UPnP;
using OpenSource.UPnP.AV.CdsMetadata;
using OpenSource.UPnP.AV.MediaServer.CP;

namespace UPnPWizard
{
	/// <summary>
	/// Summary description for CdsBrowserForm.
	/// </summary>
	public class CdsBrowserForm : System.Windows.Forms.Form, IDisposable
	{
		private System.ComponentModel.IContainer components;
		private System.Windows.Forms.ColumnHeader TitleColumn;
		private System.Windows.Forms.ColumnHeader CreatorColumn;
		private ContainerDiscovery m_Roots;

		private bool m_PendingUpdate = false;
		private System.Windows.Forms.Button buttonOk;
		private System.Windows.Forms.Button buttonCancel;
		private System.Windows.Forms.Button buttonUp;
		private System.Windows.Forms.ComboBox comboBoxContext;
		private System.Windows.Forms.ListView listViewMediaList;
		private System.Windows.Forms.ImageList treeImageList;
		private MediaBrowser m_Browser = null;

		public new void Dispose()
		{
			this.m_Roots.AllRoots.OnContainerChanged -= new CpRootContainer.Delegate_OnContainerChanged(OnAllRootsContainerChanged);
			this.m_Browser.OnRefreshComplete -= new MediaBrowser.Delegate_ContentFound(this.Sink_RefreshComplete);
		}

		public CdsBrowserForm()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//

			m_Roots = ContainerDiscovery.GetInstance();
			this.m_Browser = new MediaBrowser();

			this.m_Roots.AllRoots.OnContainerChanged += new CpRootContainer.Delegate_OnContainerChanged(OnAllRootsContainerChanged);
			this.m_Browser.OnRefreshComplete += new MediaBrowser.Delegate_ContentFound(this.Sink_RefreshComplete);

			this.PopulateContextDropDown();
			this.comboBoxContext.SelectedIndex = 0;
		}

		private void ClearAndFillMediaList()
		{
			if (this.m_Browser == null) return;

			lock (this.listViewMediaList)
			{
				this.listViewMediaList.BeginUpdate();
				foreach (ListViewItem lvi in this.listViewMediaList.Items)
				{
					lvi.Tag = null;
				}
				this.listViewMediaList.Items.Clear();

				IList cl = this.m_Browser.CurrentContext.ContainerContext.CompleteList;
				foreach (IUPnPMedia media in cl)
				{
					ListViewItem lvi;
					if (media.IsContainer)
					{
						if (media.Parent.GetType() == typeof(CpRootCollectionContainer)) 
						{
							lvi = new ListViewItem(new string[2] { media.Title, media.Creator}, 1);
						} 
						else 
						{
							lvi = new ListViewItem(new string[2] { media.Title, media.Creator}, 2);
						}
					}
					else
					{
						int icon = 4;
						string classtype = media.Class.ToString();
						switch (classtype) 
						{
							case "object.item":
								icon = 8;
								break;
							case "object.item.imageItem":
							case "object.item.imageItem.photo":
								icon = 7;
								break;
							case "object.item.videoItem":
							case "object.item.videoItem.movie":
								icon = 6;
								break;
							case "object.item.audioItem":
							case "object.item.audioItem.musicTrack":
								icon = 5;
								break;
						}
						lvi = new ListViewItem(new string[2] { media.Title, media.Creator}, icon);
					}
					lvi.Tag = media;
					this.listViewMediaList.Items.Add(lvi);
				}
				this.listViewMediaList.EndUpdate();
			}
		}

		private void IncrementalFillMediaList()
		{
			if (this.m_PendingUpdate == false)
			{
				this.m_PendingUpdate = true;

				int i=0;
				lock (this.listViewMediaList)
				{
					this.listViewMediaList.BeginUpdate();
					IList cl = this.m_Browser.CurrentContext.ContainerContext.CompleteList;

					// remove entries 
					ArrayList removeThese = new ArrayList();
					i=0;
					foreach (ListViewItem lvi in this.listViewMediaList.Items)
					{
						bool found = false;
						foreach (IUPnPMedia media in cl)
						{
							if (media == lvi.Tag)
							{
								found = true;
								break;
							}
						}

						if (found == false)
						{
							removeThese.Add(i);
							i--;
						}

						i++;
					}
					foreach (int rt in removeThese)
					{
						this.listViewMediaList.Items.RemoveAt(rt);
					}

					// add entries
					ArrayList addThese = new ArrayList();
					foreach (IUPnPMedia media in cl)
					{
						bool found = false;
						foreach (ListViewItem lvi in this.listViewMediaList.Items)
						{
							if (lvi.Tag == media)
							{
								found = true;
								break;
							}
						}

						if (found == false)
						{
							ListViewItem lvi;
							if (media.IsContainer)
							{
								if (media.Parent.GetType() == typeof(CpRootCollectionContainer)) 
								{
									lvi = new ListViewItem(new string[2] { media.Title, media.Creator}, 1);
								} 
								else 
								{
									lvi = new ListViewItem(new string[2] { media.Title, media.Creator}, 2);
								}
							}
							else
							{
								int icon = 4;
								string classtype = media.Class.ToString();
								switch (classtype) 
								{
									case "object.item":
										icon = 8;
										break;
									case "object.item.imageItem":
									case "object.item.imageItem.photo":
										icon = 7;
										break;
									case "object.item.videoItem":
									case "object.item.videoItem.movie":
										icon = 6;
										break;
									case "object.item.audioItem":
									case "object.item.audioItem.musicTrack":
										icon = 5;
										break;
								}
								lvi = new ListViewItem(new string[2] { media.Title, media.Creator}, icon);
							}
							lvi.Tag = media;
							this.listViewMediaList.Items.Add(lvi);
						}
					}
					this.listViewMediaList.EndUpdate();
				}

				this.m_PendingUpdate = false;
			}
		}

		private void OnAllRootsContainerChanged(CpRootContainer sender, CpMediaContainer thisChanged)
		{
			IncrementalFillMediaList();
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if(components != null)
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.components = new System.ComponentModel.Container();
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(CdsBrowserForm));
			this.comboBoxContext = new System.Windows.Forms.ComboBox();
			this.buttonUp = new System.Windows.Forms.Button();
			this.listViewMediaList = new System.Windows.Forms.ListView();
			this.TitleColumn = new System.Windows.Forms.ColumnHeader();
			this.CreatorColumn = new System.Windows.Forms.ColumnHeader();
			this.treeImageList = new System.Windows.Forms.ImageList(this.components);
			this.buttonOk = new System.Windows.Forms.Button();
			this.buttonCancel = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// comboBoxContext
			// 
			this.comboBoxContext.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right)));
			this.comboBoxContext.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.comboBoxContext.Location = new System.Drawing.Point(8, 8);
			this.comboBoxContext.Name = "comboBoxContext";
			this.comboBoxContext.Size = new System.Drawing.Size(384, 21);
			this.comboBoxContext.TabIndex = 0;
			this.comboBoxContext.SelectedIndexChanged += new System.EventHandler(this.comboBoxContext_SelectedIndexChanged);
			// 
			// buttonUp
			// 
			this.buttonUp.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.buttonUp.Location = new System.Drawing.Point(392, 8);
			this.buttonUp.Name = "buttonUp";
			this.buttonUp.Size = new System.Drawing.Size(40, 20);
			this.buttonUp.TabIndex = 2;
			this.buttonUp.Text = "Up";
			this.buttonUp.Click += new System.EventHandler(this.buttonUp_Click);
			// 
			// listViewMediaList
			// 
			this.listViewMediaList.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
				| System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right)));
			this.listViewMediaList.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
																								this.TitleColumn,
																								this.CreatorColumn});
			this.listViewMediaList.FullRowSelect = true;
			this.listViewMediaList.GridLines = true;
			this.listViewMediaList.Location = new System.Drawing.Point(8, 32);
			this.listViewMediaList.Name = "listViewMediaList";
			this.listViewMediaList.Size = new System.Drawing.Size(424, 312);
			this.listViewMediaList.SmallImageList = this.treeImageList;
			this.listViewMediaList.Sorting = System.Windows.Forms.SortOrder.Ascending;
			this.listViewMediaList.TabIndex = 4;
			this.listViewMediaList.View = System.Windows.Forms.View.Details;
			this.listViewMediaList.DoubleClick += new System.EventHandler(this.listViewMediaList_DoubleClick);
			this.listViewMediaList.SelectedIndexChanged += new System.EventHandler(this.listViewMediaList_SelectedIndexChanged);
			// 
			// TitleColumn
			// 
			this.TitleColumn.Text = "Title";
			this.TitleColumn.Width = 231;
			// 
			// CreatorColumn
			// 
			this.CreatorColumn.Text = "Creator";
			this.CreatorColumn.Width = 172;
			// 
			// treeImageList
			// 
			this.treeImageList.ColorDepth = System.Windows.Forms.ColorDepth.Depth16Bit;
			this.treeImageList.ImageSize = new System.Drawing.Size(16, 16);
			this.treeImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("treeImageList.ImageStream")));
			this.treeImageList.TransparentColor = System.Drawing.Color.Transparent;
			// 
			// buttonOk
			// 
			this.buttonOk.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.buttonOk.Enabled = false;
			this.buttonOk.Location = new System.Drawing.Point(352, 352);
			this.buttonOk.Name = "buttonOk";
			this.buttonOk.Size = new System.Drawing.Size(80, 23);
			this.buttonOk.TabIndex = 5;
			this.buttonOk.Text = "OK";
			// 
			// buttonCancel
			// 
			this.buttonCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.buttonCancel.Location = new System.Drawing.Point(264, 352);
			this.buttonCancel.Name = "buttonCancel";
			this.buttonCancel.Size = new System.Drawing.Size(80, 23);
			this.buttonCancel.TabIndex = 6;
			this.buttonCancel.Text = "Cancel";
			// 
			// CdsBrowserForm
			// 
			this.AcceptButton = this.buttonOk;
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.CancelButton = this.buttonCancel;
			this.ClientSize = new System.Drawing.Size(440, 382);
			this.Controls.Add(this.buttonCancel);
			this.Controls.Add(this.buttonOk);
			this.Controls.Add(this.listViewMediaList);
			this.Controls.Add(this.buttonUp);
			this.Controls.Add(this.comboBoxContext);
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.Name = "CdsBrowserForm";
			this.Text = "AV Media Wizard - Media Browser";
			this.ResumeLayout(false);

		}
		#endregion

		private void PopulateContextDropDown()
		{
			// update context drop down
			lock (this.comboBoxContext)
			{
				string spaces = "";
				this.comboBoxContext.BeginUpdate();
				IMediaContainer[] ec = this.m_Browser.CurrentContext.EntireContext;
				int i;
				for (i=0; i < ec.Length; i++)
				{
					if (i != comboBoxContext.SelectedIndex)
					{
						if (i < comboBoxContext.Items.Count)
						{
							comboBoxContext.Items[i] = spaces + ec[ec.Length-i-1].Title;
						}
						else
						{
							comboBoxContext.Items.Add(spaces + ec[ec.Length-i-1].Title);
						}
					}
					spaces += "  ";
				}
				while (comboBoxContext.Items.Count > i)
				{
					comboBoxContext.Items.RemoveAt(i);
				}
				if (comboBoxContext.Items.Count > 0) comboBoxContext.SelectedIndex = comboBoxContext.Items.Count - 1;
				comboBoxContext.EndUpdate();
			}
		}

		/// <summary>
		/// Updates the visual elements, based on the new context.
		/// </summary>
		private void HandleContextChange()
		{
			this.PopulateContextDropDown();

			// update media listing
			this.ClearAndFillMediaList();
		}

		private void Sink_RefreshComplete(MediaBrowser sender, IUPnPMedia[] added)
		{
			this.ClearAndFillMediaList();
		}

		private void listViewMediaList_DoubleClick(object sender, System.EventArgs e)
		{
			IMediaContainer mc = null;
			lock (this.comboBoxContext)
			{
				// change the context on the browser
				mc = this.listViewMediaList.SelectedItems[0].Tag as IMediaContainer;
				if (mc != null)
				{
					this.m_Browser.SetContainerContext(mc);
				}
			}

			// update UI
			if (mc != null)
			{
				this.HandleContextChange();
			}
		}

		private void comboBoxContext_SelectedIndexChanged(object sender, System.EventArgs e)
		{
			int target;
			int popThisMany;
			
			lock (this.comboBoxContext)
			{
				// change the context on the browser 
				target = this.comboBoxContext.SelectedIndex;
				popThisMany = this.comboBoxContext.Items.Count - target - 1;
				if (popThisMany > 0) this.m_Browser.Back(popThisMany);
			}

			// update context dropdown
			this.HandleContextChange();
			buttonOk.Enabled = false;
		}

		private void buttonUp_Click(object sender, System.EventArgs e)
		{
			if (m_Browser == null) return;

			lock (this.comboBoxContext)
			{
				// change the context on the browser 
				this.m_Browser.Back();
			}

			// update context dropdown
			this.HandleContextChange();
		}

		private void listViewMediaList_SelectedIndexChanged(object sender, System.EventArgs e)
		{
			if (listViewMediaList.SelectedItems.Count != 1) return;
			buttonOk.Enabled = (listViewMediaList.SelectedItems[0].Tag.GetType() == typeof(CpMediaItem));
		}

		public IUPnPMedia[] SelectedMedia
		{
			get
			{
				return null;
			}
		}
	}
}
