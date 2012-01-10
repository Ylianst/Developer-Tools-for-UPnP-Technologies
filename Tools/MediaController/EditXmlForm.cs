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
using System.Xml;
using System.Text;
using System.Drawing;
using System.Collections;
using System.Windows.Forms;
using System.ComponentModel;
using OpenSource.UPnP;
using OpenSource.UPnP.AV.CdsMetadata;

namespace UPnpMediaController
{
    /// <summary>
    /// Summary description for EditXmlForm.
    /// </summary>
    public class EditXmlForm : System.Windows.Forms.Form
    {
        private System.Windows.Forms.TextBox txtbox;
        private System.Windows.Forms.Button btnCommit;
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.Container components = null;

        public EditXmlForm()
        {
            //
            // Required for Windows Form Designer support
            //
            InitializeComponent();

            //
            // TODO: Add any constructor code after InitializeComponent call
            //
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
            this.txtbox = new System.Windows.Forms.TextBox();
            this.btnCommit = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // txtbox
            // 
            this.txtbox.Anchor = (((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                | System.Windows.Forms.AnchorStyles.Left)
                | System.Windows.Forms.AnchorStyles.Right);
            this.txtbox.Location = new System.Drawing.Point(8, 8);
            this.txtbox.Multiline = true;
            this.txtbox.Name = "txtbox";
            this.txtbox.Size = new System.Drawing.Size(376, 224);
            this.txtbox.TabIndex = 0;
            this.txtbox.Text = "";
            // 
            // btnCommit
            // 
            this.btnCommit.Anchor = ((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                | System.Windows.Forms.AnchorStyles.Right);
            this.btnCommit.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.btnCommit.Location = new System.Drawing.Point(8, 232);
            this.btnCommit.Name = "btnCommit";
            this.btnCommit.Size = new System.Drawing.Size(376, 23);
            this.btnCommit.TabIndex = 1;
            this.btnCommit.Text = "Commit Changes";
            this.btnCommit.Click += new System.EventHandler(this.btnCommit_Click);
            // 
            // EditXmlForm
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(392, 262);
            this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.btnCommit,
																		  this.txtbox});
            this.Name = "EditXmlForm";
            this.Text = "Edit XML";
            this.ResumeLayout(false);

        }
        #endregion


        public IUPnPMedia EditThis
        {
            get
            {
                return this.MediaObj;
            }
            set
            {
                this.MediaObj = value;
                this.SetTxtBox();
            }
        }

        public void SetTxtBox()
        {
            if (this.MediaObj != null)
            {
                StringBuilder sbXml = null;
                sbXml = new StringBuilder(MediaObject.XML_BUFFER_SIZE);
                StringWriter sw = new StringWriter(sbXml);
                XmlTextWriter xmlWriter = new XmlTextWriter(sw);
                xmlWriter.Formatting = System.Xml.Formatting.Indented;
                xmlWriter.Namespaces = true;

                ToXmlFormatter _f = new ToXmlFormatter();
                ToXmlData _d = new ToXmlData();
                _d.DesiredProperties = new ArrayList(0);
                _d.IsRecursive = false;
                this.MediaObj.ToXml(_f, _d, xmlWriter);
                //this.MediaObj.ToXml(false, new ArrayList(0), xmlWriter);

                this.txtbox.Text = sbXml.ToString();
            }
        }

        private IUPnPMedia MediaObj;

        private void btnCommit_Click(object sender, System.EventArgs e)
        {
            try
            {
                Tags T = Tags.GetInstance();
                StringBuilder sb = new StringBuilder(this.txtbox.Text.Length + 300);
                sb.AppendFormat("<{0} {1}=\"{2}\" {3}=\"{4}\" {5}=\"{6}\">{7}</{0}>", T[_DIDL.DIDL_Lite], Tags.XMLNS, Tags.XMLNSDIDL_VALUE, Tags.XMLNS_DC, Tags.XMLNSDC_VALUE, Tags.XMLNS_UPNP, Tags.XMLNSUPNP_VALUE, this.txtbox.Text);
                this.MediaObj.UpdateObject(sb.ToString());
            }
            catch
            {
                MessageBox.Show("Your XML is not XML or UPNP-AV ContentDirectory compliant.");
            }
        }
    }
}
