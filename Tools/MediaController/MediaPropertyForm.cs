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
using System.Drawing;
using System.Reflection;
using System.Diagnostics;
using System.Collections;
using System.Windows.Forms;
using System.ComponentModel;
using OpenSource.UPnP;
using OpenSource.UPnP.AV;
using OpenSource.UPnP.AV.CdsMetadata;
using OpenSource.UPnP.AV.MediaServer.CP;

namespace UPnpMediaController
{
    /// <summary>
    /// Summary description for MediaPropertyForm.
    /// </summary>
    public class MediaPropertyForm : System.Windows.Forms.Form
    {
        private IUPnPMedia m_Copy;
        private ICpMedia m_Original;
        private bool m_CopyIsDirty = false;
        private static Tags T = Tags.GetInstance();

        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.Windows.Forms.Button OkButton;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.Splitter splitter1;
        private System.Windows.Forms.TreeView propTreeView;
        private System.Windows.Forms.ListBox propListBox;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.ColumnHeader columnHeader2;
        private System.Windows.Forms.ListView valueListView;
        private System.Windows.Forms.PictureBox emptyDocPictureBox;
        private System.Windows.Forms.PictureBox unknownDocPictureBox;
        private System.Windows.Forms.PictureBox musicDocPictureBox;
        private System.Windows.Forms.PictureBox classTypePictureBox;
        private System.Windows.Forms.PictureBox videoDocPictureBox;
        private System.Windows.Forms.PictureBox gearsDocPictureBox;
        private System.Windows.Forms.PictureBox imageDocPictureBox;
        private System.Windows.Forms.ContextMenu valueListContextMenu;
        private System.Windows.Forms.MenuItem copyMenuItem;
        private System.Windows.Forms.MenuItem openMenuItem;
        private System.Windows.Forms.ContextMenu propListContextMenu;
        private System.Windows.Forms.MenuItem cmi_AddResource;
        private System.Windows.Forms.MenuItem cmi_RemoveResource;
        private System.Windows.Forms.MenuItem cmi_AddCustom;
        private System.Windows.Forms.MenuItem menuItem4;
        private System.Windows.Forms.MenuItem cmi_RemoveCustom;
        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.Label titleValue;
        private System.Windows.Forms.Label creatorValue;
        private System.Windows.Forms.MenuItem menuItem1;
        private System.Windows.Forms.MenuItem AddProperty;
        private System.Windows.Forms.MenuItem EditProperty;
        private System.Windows.Forms.MenuItem RemoveProperty;
        private System.Windows.Forms.ContextMenu formContextMenu;
        private System.Windows.Forms.MenuItem editXml;
        private System.ComponentModel.Container components = null;

        /// <summary>
        /// Returns a reference to the original object that was set.
        /// Sets up the form to make modifications to a deep copy of metadata.
        /// </summary>
        public ICpMedia MediaObj
        {
            get
            {
                return this.m_Original;
            }
            set
            {
                bool refresh = true;
                bool reconcile = false;
                if (this.m_Original != null)
                {
                    if (this.m_CopyIsDirty)
                    {
                        refresh = false;
                    }
                }

                if (refresh == false)
                {
                    DialogResult dlgres = MessageBox.Show("You have made changes to an object but this window wants to load another object. Requesting metadata changes with old metadata may fail.\r\nContinue editing? \r\nABORT: Abort current editing and load new metadata. \r\nRETRY: Attempt to reconcile metadata and continue editing. \r\nIGNORE: Keep old metadata and continue editing.", "Object metadata update.", MessageBoxButtons.AbortRetryIgnore, MessageBoxIcon.Question, MessageBoxDefaultButton.Button1);
                    switch (dlgres)
                    {
                        case DialogResult.Abort:
                            refresh = true;
                            break;
                        case DialogResult.Retry:
                            refresh = true;
                            reconcile = true;
                            break;
                        case DialogResult.Ignore:
                            break;
                    }
                }

                if (refresh)
                {
                    this.m_Original = value;
                    if (reconcile)
                    {
                        throw new ApplicationException("Need to implement feature to reconcile metadata.");
                    }
                    else
                    {
                        this.m_Copy = this.m_Original.MetadataCopy();
                    }
                    this.m_CopyIsDirty = false;
                    RefreshUserInterface();
                }
            }
        }

        public MediaPropertyForm()
        {
            InitializeComponent();
            RefreshUserInterface();
        }

        /// <summary>
        /// Updates the text UI elements at the top of the form.
        /// </summary>
        private void RefreshBaseProperties()
        {
            IMediaContainer mc = this.m_Copy as IMediaContainer;
            IMediaItem mi = this.m_Copy as IMediaItem;

            IUPnPMedia original = (IUPnPMedia)this.m_Original;
            IMediaContainer originalC = this.m_Original as IMediaContainer;

            this.titleValue.Text = this.m_Copy.Title;
            this.creatorValue.Text = this.m_Copy.Creator;
        }

        private static string PropertiesDelimitor = "Properties";
        private static string ResourceDelimitor = "Resource #";
        private static string CustomMetadataDelimitor = "Custom Metadata #";

        /// <summary>
        /// Updates the UI elements in the propListBox.
        /// </summary>
        private void UpdatePropListBox()
        {
            propListBox.Items.Clear();
            propListBox.Items.Add(MediaPropertyForm.PropertiesDelimitor);
            int rc = 0;
            foreach (MediaResource res in m_Copy.MergedResources)
            {
                rc++;
                propListBox.Items.Add(MediaPropertyForm.ResourceDelimitor + rc);
            }
            int n = 0;
            foreach (object node in m_Copy.DescNodes)
            {
                n++;
                propListBox.Items.Add(MediaPropertyForm.CustomMetadataDelimitor + n);
            }
        }

        /// <summary>
        /// Updates the icon at the top left corner of the window.
        /// </summary>
        private void UpdateIcon()
        {
            string classtype = "";
            if (m_Copy.MergedProperties[CommonPropertyNames.Class] != null && m_Copy.MergedProperties[CommonPropertyNames.Class].Count > 0)
            {
                classtype = m_Copy.MergedProperties[CommonPropertyNames.Class][0].ToString();
                switch (classtype)
                {
                    case "object.item":
                        classTypePictureBox.Image = gearsDocPictureBox.Image;
                        break;
                    case "object.item.imageItem":
                    case "object.item.imageItem.photo":
                        classTypePictureBox.Image = imageDocPictureBox.Image;
                        break;
                    case "object.item.videoItem":
                    case "object.item.videoItem.movie":
                        classTypePictureBox.Image = videoDocPictureBox.Image;
                        break;
                    case "object.item.audioItem":
                    case "object.item.audioItem.musicTrack":
                        classTypePictureBox.Image = musicDocPictureBox.Image;
                        break;
                }
            }
        }

        /// <summary>
        /// Causes UI base properties and propListBox elements to update.
        /// </summary>
        private void RefreshUserInterface()
        {
            if (m_Copy == null)
            {
                this.Text = "Media Property";
                titleValue.Text = "None";
            }
            else
            {
                this.Text = "Media Property - " + m_Copy.Title;

                // print base properties
                this.RefreshBaseProperties();

                // print left-hand-side entries, including all metadata blocks
                this.UpdatePropListBox();
                propListBox.SelectedIndex = 0;
                this.propListBox_SelectedIndexChanged(null, null);
            }
        }

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                if (components != null)
                {
                    components.Dispose();
                }
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code
        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(MediaPropertyForm));
            this.OkButton = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.classTypePictureBox = new System.Windows.Forms.PictureBox();
            this.panel1 = new System.Windows.Forms.Panel();
            this.valueListView = new System.Windows.Forms.ListView();
            this.columnHeader1 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader2 = new System.Windows.Forms.ColumnHeader();
            this.valueListContextMenu = new System.Windows.Forms.ContextMenu();
            this.copyMenuItem = new System.Windows.Forms.MenuItem();
            this.openMenuItem = new System.Windows.Forms.MenuItem();
            this.menuItem1 = new System.Windows.Forms.MenuItem();
            this.AddProperty = new System.Windows.Forms.MenuItem();
            this.EditProperty = new System.Windows.Forms.MenuItem();
            this.RemoveProperty = new System.Windows.Forms.MenuItem();
            this.splitter1 = new System.Windows.Forms.Splitter();
            this.propListBox = new System.Windows.Forms.ListBox();
            this.propListContextMenu = new System.Windows.Forms.ContextMenu();
            this.cmi_AddResource = new System.Windows.Forms.MenuItem();
            this.cmi_RemoveResource = new System.Windows.Forms.MenuItem();
            this.menuItem4 = new System.Windows.Forms.MenuItem();
            this.cmi_AddCustom = new System.Windows.Forms.MenuItem();
            this.cmi_RemoveCustom = new System.Windows.Forms.MenuItem();
            this.titleValue = new System.Windows.Forms.Label();
            this.creatorValue = new System.Windows.Forms.Label();
            this.propTreeView = new System.Windows.Forms.TreeView();
            this.emptyDocPictureBox = new System.Windows.Forms.PictureBox();
            this.unknownDocPictureBox = new System.Windows.Forms.PictureBox();
            this.videoDocPictureBox = new System.Windows.Forms.PictureBox();
            this.musicDocPictureBox = new System.Windows.Forms.PictureBox();
            this.imageDocPictureBox = new System.Windows.Forms.PictureBox();
            this.gearsDocPictureBox = new System.Windows.Forms.PictureBox();
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.formContextMenu = new System.Windows.Forms.ContextMenu();
            this.editXml = new System.Windows.Forms.MenuItem();
            this.panel1.SuspendLayout();
            this.SuspendLayout();
            // 
            // OkButton
            // 
            this.OkButton.Anchor = (System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right);
            this.OkButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.OkButton.Location = new System.Drawing.Point(398, 406);
            this.OkButton.Name = "OkButton";
            this.OkButton.Size = new System.Drawing.Size(80, 24);
            this.OkButton.TabIndex = 0;
            this.OkButton.Text = "OK";
            this.OkButton.Click += new System.EventHandler(this.OkButton_Click);
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(40, 8);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(72, 16);
            this.label1.TabIndex = 1;
            this.label1.Text = "Title";
            // 
            // label2
            // 
            this.label2.Location = new System.Drawing.Point(40, 24);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(72, 16);
            this.label2.TabIndex = 2;
            this.label2.Text = "Creator";
            // 
            // classTypePictureBox
            // 
            this.classTypePictureBox.Image = ((System.Drawing.Bitmap)(resources.GetObject("classTypePictureBox.Image")));
            this.classTypePictureBox.Location = new System.Drawing.Point(8, 8);
            this.classTypePictureBox.Name = "classTypePictureBox";
            this.classTypePictureBox.Size = new System.Drawing.Size(32, 32);
            this.classTypePictureBox.TabIndex = 3;
            this.classTypePictureBox.TabStop = false;
            this.classTypePictureBox.MouseDown += new System.Windows.Forms.MouseEventHandler(this.classTypePictureBox_MouseDown);
            // 
            // panel1
            // 
            this.panel1.Anchor = (((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                | System.Windows.Forms.AnchorStyles.Left)
                | System.Windows.Forms.AnchorStyles.Right);
            this.panel1.Controls.AddRange(new System.Windows.Forms.Control[] {
																				 this.valueListView,
																				 this.splitter1,
																				 this.propListBox});
            this.panel1.Location = new System.Drawing.Point(8, 56);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(470, 336);
            this.panel1.TabIndex = 8;
            // 
            // valueListView
            // 
            this.valueListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
																							this.columnHeader1,
																							this.columnHeader2});
            this.valueListView.ContextMenu = this.valueListContextMenu;
            this.valueListView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.valueListView.FullRowSelect = true;
            this.valueListView.Location = new System.Drawing.Point(115, 0);
            this.valueListView.MultiSelect = false;
            this.valueListView.Name = "valueListView";
            this.valueListView.Size = new System.Drawing.Size(355, 336);
            this.valueListView.TabIndex = 9;
            this.valueListView.View = System.Windows.Forms.View.Details;
            this.valueListView.MouseDown += new System.Windows.Forms.MouseEventHandler(this.valueListView_MouseDown);
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "Name";
            this.columnHeader1.Width = 100;
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "Value";
            this.columnHeader2.Width = 250;
            // 
            // valueListContextMenu
            // 
            this.valueListContextMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																								 this.copyMenuItem,
																								 this.openMenuItem,
																								 this.menuItem1,
																								 this.AddProperty,
																								 this.EditProperty,
																								 this.RemoveProperty});
            this.valueListContextMenu.Popup += new System.EventHandler(this.valueListContextMenu_Popup);
            // 
            // copyMenuItem
            // 
            this.copyMenuItem.Index = 0;
            this.copyMenuItem.Text = "&Copy";
            this.copyMenuItem.Click += new System.EventHandler(this.copyMenuItem_Click);
            // 
            // openMenuItem
            // 
            this.openMenuItem.Index = 1;
            this.openMenuItem.Text = "&Open URI";
            this.openMenuItem.Click += new System.EventHandler(this.openMenuItem_Click);
            // 
            // menuItem1
            // 
            this.menuItem1.Index = 2;
            this.menuItem1.Text = "-";
            // 
            // AddProperty
            // 
            this.AddProperty.Index = 3;
            this.AddProperty.Text = "Add Property";
            this.AddProperty.Click += new System.EventHandler(this.AddProperty_Click);
            // 
            // EditProperty
            // 
            this.EditProperty.Index = 4;
            this.EditProperty.Text = "Edit Property";
            this.EditProperty.Click += new System.EventHandler(this.EditProperty_Click);
            // 
            // RemoveProperty
            // 
            this.RemoveProperty.Index = 5;
            this.RemoveProperty.Text = "Remove Property";
            this.RemoveProperty.Click += new System.EventHandler(this.RemoveProperty_Click);
            // 
            // splitter1
            // 
            this.splitter1.Location = new System.Drawing.Point(112, 0);
            this.splitter1.Name = "splitter1";
            this.splitter1.Size = new System.Drawing.Size(3, 336);
            this.splitter1.TabIndex = 8;
            this.splitter1.TabStop = false;
            // 
            // propListBox
            // 
            this.propListBox.ContextMenu = this.propListContextMenu;
            this.propListBox.Dock = System.Windows.Forms.DockStyle.Left;
            this.propListBox.IntegralHeight = false;
            this.propListBox.Name = "propListBox";
            this.propListBox.Size = new System.Drawing.Size(112, 336);
            this.propListBox.TabIndex = 7;
            this.propListBox.MouseDown += new System.Windows.Forms.MouseEventHandler(this.propListBox_MouseDown);
            this.propListBox.SelectedIndexChanged += new System.EventHandler(this.propListBox_SelectedIndexChanged);
            // 
            // propListContextMenu
            // 
            this.propListContextMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																								this.cmi_AddResource,
																								this.cmi_RemoveResource,
																								this.menuItem4,
																								this.cmi_AddCustom,
																								this.cmi_RemoveCustom});
            this.propListContextMenu.Popup += new System.EventHandler(this.propListContextMenu_Popup);
            // 
            // cmi_AddResource
            // 
            this.cmi_AddResource.Index = 0;
            this.cmi_AddResource.Text = "Add Resource";
            this.cmi_AddResource.Click += new System.EventHandler(this.cmi_AddResource_Click);
            // 
            // cmi_RemoveResource
            // 
            this.cmi_RemoveResource.Index = 1;
            this.cmi_RemoveResource.Text = "Remove Resource";
            this.cmi_RemoveResource.Click += new System.EventHandler(this.cmi_RemoveResource_Click);
            // 
            // menuItem4
            // 
            this.menuItem4.Index = 2;
            this.menuItem4.Text = "-";
            // 
            // cmi_AddCustom
            // 
            this.cmi_AddCustom.Index = 3;
            this.cmi_AddCustom.Text = "Add Custom Metadata Block";
            this.cmi_AddCustom.Click += new System.EventHandler(this.cmi_AddCustom_Click);
            // 
            // cmi_RemoveCustom
            // 
            this.cmi_RemoveCustom.Index = 4;
            this.cmi_RemoveCustom.Text = "Remove Custom Metadata Block";
            this.cmi_RemoveCustom.Click += new System.EventHandler(this.cmi_RemoveCustom_Click);
            // 
            // titleValue
            // 
            this.titleValue.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                | System.Windows.Forms.AnchorStyles.Right);
            this.titleValue.Location = new System.Drawing.Point(112, 8);
            this.titleValue.Name = "titleValue";
            this.titleValue.Size = new System.Drawing.Size(366, 16);
            this.titleValue.TabIndex = 9;
            // 
            // creatorValue
            // 
            this.creatorValue.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                | System.Windows.Forms.AnchorStyles.Right);
            this.creatorValue.Location = new System.Drawing.Point(112, 24);
            this.creatorValue.Name = "creatorValue";
            this.creatorValue.Size = new System.Drawing.Size(366, 16);
            this.creatorValue.TabIndex = 10;
            // 
            // propTreeView
            // 
            this.propTreeView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.propTreeView.ImageIndex = -1;
            this.propTreeView.Name = "propTreeView";
            this.propTreeView.SelectedImageIndex = -1;
            this.propTreeView.Size = new System.Drawing.Size(312, 248);
            this.propTreeView.TabIndex = 9;
            // 
            // emptyDocPictureBox
            // 
            this.emptyDocPictureBox.Anchor = (System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left);
            this.emptyDocPictureBox.Image = ((System.Drawing.Bitmap)(resources.GetObject("emptyDocPictureBox.Image")));
            this.emptyDocPictureBox.Location = new System.Drawing.Point(8, 403);
            this.emptyDocPictureBox.Name = "emptyDocPictureBox";
            this.emptyDocPictureBox.Size = new System.Drawing.Size(32, 40);
            this.emptyDocPictureBox.TabIndex = 11;
            this.emptyDocPictureBox.TabStop = false;
            this.emptyDocPictureBox.Visible = false;
            // 
            // unknownDocPictureBox
            // 
            this.unknownDocPictureBox.Anchor = (System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left);
            this.unknownDocPictureBox.Image = ((System.Drawing.Bitmap)(resources.GetObject("unknownDocPictureBox.Image")));
            this.unknownDocPictureBox.Location = new System.Drawing.Point(40, 403);
            this.unknownDocPictureBox.Name = "unknownDocPictureBox";
            this.unknownDocPictureBox.Size = new System.Drawing.Size(32, 40);
            this.unknownDocPictureBox.TabIndex = 12;
            this.unknownDocPictureBox.TabStop = false;
            this.unknownDocPictureBox.Visible = false;
            // 
            // videoDocPictureBox
            // 
            this.videoDocPictureBox.Anchor = (System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left);
            this.videoDocPictureBox.Image = ((System.Drawing.Bitmap)(resources.GetObject("videoDocPictureBox.Image")));
            this.videoDocPictureBox.Location = new System.Drawing.Point(72, 403);
            this.videoDocPictureBox.Name = "videoDocPictureBox";
            this.videoDocPictureBox.Size = new System.Drawing.Size(32, 40);
            this.videoDocPictureBox.TabIndex = 13;
            this.videoDocPictureBox.TabStop = false;
            this.videoDocPictureBox.Visible = false;
            // 
            // musicDocPictureBox
            // 
            this.musicDocPictureBox.Anchor = (System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left);
            this.musicDocPictureBox.Image = ((System.Drawing.Bitmap)(resources.GetObject("musicDocPictureBox.Image")));
            this.musicDocPictureBox.Location = new System.Drawing.Point(104, 403);
            this.musicDocPictureBox.Name = "musicDocPictureBox";
            this.musicDocPictureBox.Size = new System.Drawing.Size(32, 40);
            this.musicDocPictureBox.TabIndex = 14;
            this.musicDocPictureBox.TabStop = false;
            this.musicDocPictureBox.Visible = false;
            // 
            // imageDocPictureBox
            // 
            this.imageDocPictureBox.Anchor = (System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left);
            this.imageDocPictureBox.Image = ((System.Drawing.Bitmap)(resources.GetObject("imageDocPictureBox.Image")));
            this.imageDocPictureBox.Location = new System.Drawing.Point(136, 403);
            this.imageDocPictureBox.Name = "imageDocPictureBox";
            this.imageDocPictureBox.Size = new System.Drawing.Size(32, 40);
            this.imageDocPictureBox.TabIndex = 15;
            this.imageDocPictureBox.TabStop = false;
            this.imageDocPictureBox.Visible = false;
            // 
            // gearsDocPictureBox
            // 
            this.gearsDocPictureBox.Anchor = (System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left);
            this.gearsDocPictureBox.Image = ((System.Drawing.Bitmap)(resources.GetObject("gearsDocPictureBox.Image")));
            this.gearsDocPictureBox.Location = new System.Drawing.Point(168, 403);
            this.gearsDocPictureBox.Name = "gearsDocPictureBox";
            this.gearsDocPictureBox.Size = new System.Drawing.Size(32, 40);
            this.gearsDocPictureBox.TabIndex = 16;
            this.gearsDocPictureBox.TabStop = false;
            this.gearsDocPictureBox.Visible = false;
            // 
            // pictureBox1
            // 
            this.pictureBox1.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                | System.Windows.Forms.AnchorStyles.Right);
            this.pictureBox1.BackColor = System.Drawing.Color.Gray;
            this.pictureBox1.Location = new System.Drawing.Point(8, 48);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(470, 3);
            this.pictureBox1.TabIndex = 26;
            this.pictureBox1.TabStop = false;
            // 
            // formContextMenu
            // 
            this.formContextMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																							this.editXml});
            // 
            // editXml
            // 
            this.editXml.Index = 0;
            this.editXml.Text = "Edit XML";
            this.editXml.Click += new System.EventHandler(this.editXml_Click);
            // 
            // MediaPropertyForm
            // 
            this.AcceptButton = this.OkButton;
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.CancelButton = this.OkButton;
            this.ClientSize = new System.Drawing.Size(488, 438);
            this.ContextMenu = this.formContextMenu;
            this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.pictureBox1,
																		  this.gearsDocPictureBox,
																		  this.imageDocPictureBox,
																		  this.musicDocPictureBox,
																		  this.videoDocPictureBox,
																		  this.unknownDocPictureBox,
																		  this.emptyDocPictureBox,
																		  this.creatorValue,
																		  this.titleValue,
																		  this.panel1,
																		  this.label2,
																		  this.label1,
																		  this.OkButton,
																		  this.classTypePictureBox});
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "MediaPropertyForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Media Property";
            this.panel1.ResumeLayout(false);
            this.ResumeLayout(false);

        }
        #endregion

        private void OkButton_Click(object sender, System.EventArgs e)
        {
            DialogResult = DialogResult.OK;
            Close();
        }

        /// <summary>
        /// Updates the value list with property data.
        /// </summary>
        private void UpdateValueListWithProperties(IMediaProperties properties)
        {
            valueListView.Items.Clear();
            foreach (string propertyName in properties.PropertyNames)
            {
                int rc = 0;
                IList propertyValues = m_Copy.MergedProperties[propertyName];

                foreach (ICdsElement propertyvalue in propertyValues)
                {
                    rc++;
                    if (rc == 1)
                    {
                        valueListView.Items.Add(new ListViewItem(new string[] { propertyName, propertyvalue.StringValue }));
                    }
                    else
                    {
                        valueListView.Items.Add(new ListViewItem(new string[] { propertyName + "(" + rc + ")", propertyvalue.StringValue }));
                    }
                }
            }

            valueListView.Items.Add(new ListViewItem(new string[] { "Restricted", m_Copy.IsRestricted.ToString() }));
            valueListView.Items.Add(new ListViewItem(new string[] { "Media Class", m_Copy.Class.ToString() }));
            valueListView.Items.Add(new ListViewItem(new string[] { "Object ID", m_Original.ID }));

            IMediaContainer originalC = this.m_Original as IMediaContainer;
            IMediaContainer mc = this.m_Copy as IMediaContainer;
            if (mc != null && originalC.IsRootContainer)
            {
                valueListView.Items.Add(new ListViewItem(new string[] { "Server UDN", ((CpRootContainer)m_Original).UDN }));
            }
            else
            {
                valueListView.Items.Add(new ListViewItem(new string[] { "Parent ID", m_Original.ParentID }));
            }

            if (mc != null) valueListView.Items.Add(new ListViewItem(new string[] { "Searchable", mc.IsSearchable.ToString() }));
        }

        /// <summary>
        /// Updates the valueList window with information about a resource.
        /// </summary>
        /// <param name="res"></param>
        private void UpdateValueListWithResource(IMediaResource res)
        {
            valueListView.Items.Clear();
            valueListView.Items.Add(new ListViewItem(new string[] { "contentUri", res.ContentUri }));

            ICollection ic = res.ValidAttributes;
            SortedList sl = new SortedList(ic.Count);

            foreach (string attrib in res.ValidAttributes)
            {
                sl.Add(attrib, attrib);
            }

            foreach (string attrib in sl.Values)
            {
                valueListView.Items.Add(new ListViewItem(new string[] { attrib, res[attrib].ToString() }));
            }
        }

        /// <summary>
        /// Updates the valueList portion of the window to update
        /// with the appropriate type of information.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void propListBox_SelectedIndexChanged(object sender, System.EventArgs e)
        {
            PropListBoxSelected propType = this.GetPropListBoxSelectedType();
            switch (propType)
            {
                case PropListBoxSelected.NothingSelected:
                    return;

                case PropListBoxSelected.Properties:
                    valueListView.Items.Clear();
                    this.UpdateValueListWithProperties(this.m_Copy.Properties);
                    break;
                case PropListBoxSelected.Resource:
                    IMediaResource res = (IMediaResource)m_Copy.MergedResources[propListBox.SelectedIndex - 1];
                    valueListView.Items.Clear();
                    this.UpdateValueListWithResource(res);
                    break;
                case PropListBoxSelected.CustomMetadata:
                    throw new ApplicationException("Need to write handler for propListBox custom metadata entry.");
                case PropListBoxSelected.Unknown:
                default:
                    throw new ApplicationException("Need to write handler for propListBox entry.");
            }
        }

        private void classTypePictureBox_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            if (m_Copy != null)
            {
                IUPnPMedia[] items = new IUPnPMedia[1];
                items[0] = m_Copy;
                classTypePictureBox.DoDragDrop(items, DragDropEffects.Copy | DragDropEffects.Move);
            }
        }

        private void valueListContextMenu_Popup(object sender, System.EventArgs e)
        {
            copyMenuItem.Visible = (valueListView.SelectedItems.Count > 0);
            openMenuItem.Visible = (valueListView.SelectedItems.Count > 0 && valueListView.SelectedItems[0].SubItems[1].Text.ToLower().StartsWith("http://"));

            AddProperty.Visible = MODIFY_ENABLED;
            EditProperty.Visible = ((valueListView.SelectedItems.Count > 0) && MODIFY_ENABLED);
            RemoveProperty.Visible = ((valueListView.SelectedItems.Count > 0) && MODIFY_ENABLED);
        }

        private void copyMenuItem_Click(object sender, System.EventArgs e)
        {
            if (valueListView.SelectedItems.Count == 0) return;
            Clipboard.SetDataObject(valueListView.SelectedItems[0].SubItems[1].Text);
        }

        private void openMenuItem_Click(object sender, System.EventArgs e)
        {
            if (valueListView.SelectedItems.Count == 0) return;
            if (valueListView.SelectedItems[0].SubItems[1].Text.ToLower().StartsWith("http://") == false) return;
            try
            {
                System.Diagnostics.Process.Start("\"" + valueListView.SelectedItems[0].SubItems[1].Text + "\"");
            }
            catch (System.ComponentModel.Win32Exception) { }
        }

        /// <summary>
        /// Draws the context menu when you right-click on propListBox item.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void propListContextMenu_Popup(object sender, System.EventArgs e)
        {
            // figure out what object to select
            //this.propListBox.SetSelected

            this.cmi_AddCustom.Visible = MODIFY_ENABLED;
            this.cmi_AddResource.Visible = MODIFY_ENABLED;

            PropListBoxSelected propType = this.GetPropListBoxSelectedType();

            switch (propType)
            {
                case PropListBoxSelected.Properties:
                    this.cmi_RemoveResource.Visible = false;
                    this.cmi_RemoveCustom.Visible = false;
                    break;

                case PropListBoxSelected.Resource:
                    this.cmi_RemoveResource.Visible = MODIFY_ENABLED;
                    this.cmi_RemoveCustom.Visible = false;
                    break;

                case PropListBoxSelected.CustomMetadata:
                    this.cmi_RemoveResource.Visible = false;
                    this.cmi_RemoveCustom.Visible = MODIFY_ENABLED;
                    break;
            }
        }

        private bool MODIFY_ENABLED = false;

        /// <summary>
        /// Adds a new resource to the object.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cmi_AddResource_Click(object sender, System.EventArgs e)
        {
            MessageBox.Show("Not supported in this version. Right-click on above text area and do Edit XML instead.");
            /*
            this.m_CopyIsDirty = true;
            IMediaResource newRes = new MediaResource();
            this.m_Copy.AddResource(newRes);
            this.UpdatePropListBox();
            */
        }

        /// <summary>
        /// Removes the selected resource.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cmi_RemoveResource_Click(object sender, System.EventArgs e)
        {
            MessageBox.Show("Not supported in this version. Right-click on above text area and do Edit XML instead.");
            /*
            this.m_CopyIsDirty = true;

            PropListBoxSelected propType = this.GetPropListBoxSelectedType();

            if (propType == PropListBoxSelected.Resource)
            {
                IList resources = this.m_Copy.Resources;
                int index = this.propListBox.SelectedIndex;

                if (index > 0)
                {
                    IMediaResource res = (IMediaResource) resources[index-1];
                    this.m_Copy.RemoveResource(res);
				
                    System.Diagnostics.Debug.Assert(resources.Count != this.m_Copy.Resources.Count, "cmi_RemoveResource_Click failed.");

                    this.propListBox.Items.RemoveAt(index);
                }
            }
            */
        }

        /// <summary>
        /// Adds a custom metadata block
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cmi_AddCustom_Click(object sender, System.EventArgs e)
        {
            //this.m_CopyIsDirty = true;
            MessageBox.Show("Not supported in this version. Right-click on above text area and do Edit XML instead.");
        }

        /// <summary>
        /// Removes the selected custom metadata block.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cmi_RemoveCustom_Click(object sender, System.EventArgs e)
        {
            //this.m_CopyIsDirty = true;
            MessageBox.Show("Not supported in this version. Right-click on above text area and do Edit XML instead.");
        }

        /// <summary>
        /// This method makes it so that a right click will cause the appropriate
        /// item in propListBox to get selected before doing the context menu.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void propListBox_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Right)
            {
                ListBox listbox = sender as ListBox;
                if (listbox == this.propListBox)
                {
                    for (int i = 0; i < listbox.Items.Count; i++)
                    {
                        Rectangle r = listbox.GetItemRectangle(i);
                        if (r.Contains(e.X, e.Y))
                        {
                            listbox.SelectedIndex = i;
                            break;
                        }
                    }
                    this.propListContextMenu_Popup(null, null);
                }
            }
        }

        /// <summary>
        /// Enumerates through the types of entries in the propListBox.
        /// </summary>
        private enum PropListBoxSelected
        {
            Unknown,
            NothingSelected,
            Properties,
            Resource,
            CustomMetadata
        }

        /// <summary>
        /// Examines the currently selected entry in propListBox and returns
        /// a value indicating the type of thingie that's selected.
        /// </summary>
        /// <returns></returns>
        private PropListBoxSelected GetPropListBoxSelectedType()
        {
            if (this.propListBox.SelectedItem != null)
            {
                if (this.propListBox.SelectedItem.ToString().StartsWith(MediaPropertyForm.PropertiesDelimitor))
                {
                    return PropListBoxSelected.Properties;
                }
                else if (this.propListBox.SelectedItem.ToString().StartsWith(MediaPropertyForm.ResourceDelimitor))
                {
                    return PropListBoxSelected.Resource;
                }
                else if (this.propListBox.SelectedItem.ToString().StartsWith(MediaPropertyForm.CustomMetadataDelimitor))
                {
                    return PropListBoxSelected.CustomMetadata;
                }

                return PropListBoxSelected.Unknown;
            }

            return PropListBoxSelected.NothingSelected;
        }

        /// <summary>
        /// Generic sorter to keep arraylist objects sorted by their
        /// standard IComparable values.
        /// </summary>
        private static _SortedList Sorter = new _SortedList(null, false);

        /// <summary>
        /// Keeps a listing of all known types of media classes.
        /// </summary>
        private static ArrayList MediaClasses = new ArrayList();

        /// <summary>
        /// Nice easy way to wrap up information about a media class
        /// and the associated metadata fields that go with it.
        /// </summary>
        private struct MediaClassInfo : IComparable
        {
            /// <summary>
            /// The .NET class/type derived from MediaBuilder.CoreMetadata
            /// that would give information about the metadata properties
            /// for a given media class.
            /// </summary>
            public Type _Type;
            /// <summary>
            /// An actual instantiation of the MediaClass with 
            /// a full class name and a default friendly name.
            /// </summary>
            public MediaClass _Class;
            /// <summary>
            /// Returns the full class name.
            /// </summary>
            /// <returns></returns>
            public override string ToString() { return _Class.FullClassName; }
            /// <summary>
            /// Provides the magic for the static Sorter, so that
            /// all entries in MediaClasses are sorted by full class name.
            /// </summary>
            /// <param name="obj"></param>
            /// <returns></returns>
            public int CompareTo(object obj)
            {
                return string.Compare(this.ToString(), obj.ToString());
            }
        }

        /// <summary>
        /// Populates the MediaClasses arraylist.
        /// </summary>
        private static void GetMediaClasses()
        {
            lock (MediaClasses.SyncRoot)
            {
                if (MediaClasses.Count == 0)
                {
                    // Iterate through assemblies
                    Assembly[] assemblies = AppDomain.CurrentDomain.GetAssemblies();
                    foreach (Assembly asm in assemblies)
                    {
                        // Iterate through class types of each assembly
                        Type[] types = asm.GetTypes();
                        foreach (Type type in types)
                        {
                            // Only classes derived from MediaBuilder.CoreMetadata
                            // are considered for defining a media classes.
                            if (type.IsSubclassOf(typeof(MediaBuilder.CoreMetadata)))
                            {
                                // Recurse up the class's base types and derive
                                // a string that reveals the full media class
                                // that this type represents.

                                Stack baseTypes = new Stack();
                                Type baseType = type;

                                while (baseType != typeof(MediaBuilder.CoreMetadata))
                                {
                                    baseTypes.Push(baseType);
                                    baseType = baseType.BaseType;
                                }

                                StringBuilder sb = new StringBuilder(baseTypes.Count * 8 + 1);
                                sb.Append("object.");

                                while (baseTypes.Count > 0)
                                {
                                    baseType = (Type)baseTypes.Pop();
                                    sb.Append(baseType.Name);
                                    if (baseTypes.Count > 0)
                                    {
                                        sb.Append(".");
                                    }
                                }

                                string mediaClassString = sb.ToString();

                                // Introspect the type for a friendly name.
                                FieldInfo fi = type.GetField("CdsFriendlyName", BindingFlags.Instance | BindingFlags.NonPublic | BindingFlags.Public);

                                string friendly = "";
                                if (fi != null)
                                {
                                    ConstructorInfo ci = type.GetConstructor(new Type[0]);
                                    if (ci != null)
                                    {
                                        object obj = ci.Invoke(new Object[0]);
                                        friendly = (string)fi.GetValue(obj);
                                    }
                                }

                                // Instantiate a media class and store it in the array
                                MediaClass newClass = new MediaClass(mediaClassString, friendly);

                                MediaClassInfo mci = new MediaClassInfo();
                                mci._Type = type;
                                mci._Class = newClass;
                                Sorter.Set(MediaClasses, mci, false); ;
                            }
                        }
                    }
                }
            }
        }

        /// <summary>
        /// Iterates through all possible values in the
        /// <see cref="OpenSource.UPnP.AV.CdsMetadata.CommonPropertyNames"/>
        /// enumerator and returns a list of keys in string form.
        /// </summary>
        /// <returns></returns>
        private ArrayList GetAllPropertyNames()
        {
            ArrayList al = new ArrayList(T.DC.Count + T.UPNP.Count);
            al.AddRange(T.DC);
            al.AddRange(T.UPNP);
            return al;
        }

        /*
        /// <summary>
        /// Given an IMediaProperties, this method returns a list of keys that 
        /// a programmer could choose to add. 
        /// </summary>
        /// <param name="properties">
        /// An IMediaProperties instance that holds information about what metadata
        /// exists in a properties group.
        /// </param>
        /// <param name="onlyForClassType">If the onlyForClassType boolean
        /// is true, then the returned list only contains a further subset
        /// of keys that apply to the media class stored in the properties argument.
        /// </param>
        /// <returns>
        /// </returns>
        private IList GetAvailablePropertyFields(IMediaProperties properties, bool onlyForClassType)
        {
            ArrayList allKeys = this.GetAllPropertyNames();
            ICollection usedKeys = properties.PropertyNames;
            ArrayList retVal = new ArrayList();

            foreach (string key in usedKeys)
            {
                // only add the keys that allow multiple values
                bool allowsMultiple;
                Type dataType = PropertyMappings.PropertyNameToType(key, out allowsMultiple);
                if (allowsMultiple == true)
                {
                    NewPropertyForm.PropertyInfo pi = new NewPropertyForm.PropertyInfo();
                    pi._Name = key;
                    pi._Type = dataType;
                    pi._MultipleOk = allowsMultiple;
                    retVal.Add(pi);
                }
                else
                {
                    allKeys.Remove(key);
                }
            }

            if (onlyForClassType)
            {
                // Further restrict the listing of keys to those keys
                // that apply to a particular media class
                IList values = properties[CommonPropertyNames.Class];
                MediaClass mclass = values[0] as MediaClass;
                if (mclass != null)
                {
                    MediaPropertyForm.GetMediaClasses();

                    // Iterate through the possible list of media classes.
                    foreach (MediaClassInfo mci in MediaPropertyForm.MediaClasses)
                    {
                        // If a class name matches, then we introspect
                        // the type associated with that class and figure
                        // out what fields apply to that media class.
                        if (mci._Class.FullClassNameMatches(mclass))
                        {
                            FieldInfo[] fields = mci._Type.GetFields();

                            // Iterate through the remaining list of 
                            // possible properties that we intend to send back.
                            // If the property name is not present in the list
                            // of fields, then remove it from the list.
                            ArrayList removeThese = new ArrayList();
                            int i=0;
                            foreach (string key in allKeys)
                            {
                                bool found = false;
                                string key2 = key.Substring(key.IndexOf(":")+1);
                                foreach (FieldInfo fi in fields)
                                {
                                    if (fi.Name == key2)
                                    {
                                        found = true;
                                        break;
                                    }
                                }

                                if (!found)
                                {
                                    removeThese.Add(i);
                                }
                                i++;
                            }

                            int j = 0;
                            foreach (int index in removeThese)
                            {
                                allKeys.RemoveAt(index-j);
                                j++;
                            }

                            // Don't bother searching the rest of the classes
                            break;
                        }
                    }

                    //allKeys has the remaining keys that need to be added
                    foreach (string key in allKeys)
                    {
                        bool allowsMultiple;
                        Type dataType = PropertyMappings.PropertyNameToType(key, out allowsMultiple);
                        NewPropertyForm.PropertyInfo pi = new NewPropertyForm.PropertyInfo();
                        pi._Name = key;
                        pi._Type = dataType;
                        pi._MultipleOk = allowsMultiple;
                        retVal.Add(pi);
                    }
                }
            }
			
            return retVal;
        }

        /// <summary>
        /// When adding a new property to the properties group or to a resource, 
        /// we call this method and provide a list of possible metadata fields.
        /// User will then choose a new desired metadata field.
        /// User is also responsible for entering one or more valid 
        /// type-checked values for that field.
        /// </summary>
        /// <param name="availableFields"></param>
        /// <param name="selectedField"></param>
        /// <param name="fieldValues"></param>
        private void ShowDialog_NewProperty(IList availableFields, out object selectedField, out IList fieldValues)
        {
            selectedField = null;
            fieldValues = null;

            NewPropertyForm npf = new NewPropertyForm();
            npf.SetAvailableProperties(availableFields);
            DialogResult dlgresult = npf.ShowDialog(this);

            if (dlgresult == DialogResult.OK)
            {
            }
        }
        */

        /// <summary>
        /// Adds a metadata field for a resource or properties group.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void AddProperty_Click(object sender, System.EventArgs e)
        {
            MessageBox.Show("Not supported in this version. Right-click on above text area and do Edit XML instead.");
            /*
            PropListBoxSelected propType = this.GetPropListBoxSelectedType();

            switch (propType)
            {
                case PropListBoxSelected.Properties:
                    IList availableFields = this.GetAvailablePropertyFields(this.m_Copy.Properties, true);
                    object selectedField;
                    IList newValues;
                    this.ShowDialog_NewProperty(availableFields, out selectedField, out newValues);
                    break;

                case PropListBoxSelected.Resource:
                    break;
            }
            */
        }

        /// <summary>
        /// Edits the metadata values for a resource or properties group.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void EditProperty_Click(object sender, System.EventArgs e)
        {
            MessageBox.Show("Not supported in this version. Right-click on above text area and do Edit XML instead.");
            /*
            PropListBoxSelected propType = this.GetPropListBoxSelectedType();

            switch (propType)
            {
                case PropListBoxSelected.Properties:
                    break;

                case PropListBoxSelected.Resource:
                    break;
            }		
            */
        }

        /// <summary>
        /// Removes a metadata field for a resource or properties group.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void RemoveProperty_Click(object sender, System.EventArgs e)
        {
            MessageBox.Show("Not supported in this version. Right-click on above text area and do Edit XML instead.");
            /*
            PropListBoxSelected propType = this.GetPropListBoxSelectedType();

            switch (propType)
            {
                case PropListBoxSelected.Properties:
                    break;

                case PropListBoxSelected.Resource:
                    break;
            }		
            */
        }

        /// <summary>
        /// Handles right-click on valueListView so that an appropriate item is selected
        /// and then the pop menu shows.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void valueListView_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Right)
            {
                ListView listview = sender as ListView;
                if (listview == this.valueListView)
                {
                    ListViewItem lvi = listview.GetItemAt(e.X, e.Y);
                    if (lvi != null)
                    {
                        lvi.Selected = true;
                    }
                    this.valueListContextMenu_Popup(null, null);
                }
            }
        }

        /// <summary>
        /// Pops up a window for editing a media object's Xml.
        /// If user commits changes, the object sends a request
        /// to the remote media server to change metadata.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void editXml_Click(object sender, System.EventArgs e)
        {
            EditXmlForm editForm = new EditXmlForm();
            editForm.EditThis = this.m_Copy;
            DialogResult res = editForm.ShowDialog(this);

            if (res == DialogResult.OK)
            {
                DateTime now;
                lock (MediaPropertyForm.MetadataRequests.SyncRoot)
                {
                    now = DateTime.Now;
                    MediaPropertyForm.MetadataRequests[now] = this;
                }
                this.m_Original.RequestUpdateObject(editForm.EditThis, now, new CpMediaDelegates.Delegate_ResultUpdateObject(MediaPropertyForm.OnResult_RequestForUpdateObject));
            }
        }

        private static void OnResult_RequestForUpdateObject(ICpMedia attemptChangeOnThis, IUPnPMedia usedThisMetadata, object Tag, UPnPInvokeException error)
        {
            MediaPropertyForm form = null;
            lock (MediaPropertyForm.MetadataRequests.SyncRoot)
            {
                form = (MediaPropertyForm)MediaPropertyForm.MetadataRequests[Tag];
                MediaPropertyForm.MetadataRequests.Remove(Tag);
            }

            if (error != null)
            {
                MessageBox.Show(error.UPNP.Message, "An error occurred while trying to update an object.");
            }
            if (form != null)
            {
            }
        }

        private static Hashtable MetadataRequests = new Hashtable();
    }
}
