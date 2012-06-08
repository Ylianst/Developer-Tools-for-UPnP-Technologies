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
using System.Windows.Forms;
using System.ComponentModel;
using OpenSource.UPnP;

namespace ServiceAuthor
{
    /// <summary>
    /// Summary description for ComplexTypeProperty.
    /// </summary>
    public class ComplexTypeProperty : System.Windows.Forms.Form
    {
        public string LocalName
        {
            get
            {
                return (LocalNameTextBox.Text);
            }
        }
        public string Namespace
        {
            get
            {
                return (NamespaceTextBox.Text);
            }
        }
        private System.Windows.Forms.TextBox textBox3;
        private System.Windows.Forms.TextBox NamespaceTextBox;
        private System.Windows.Forms.TextBox textBox1;
        private System.Windows.Forms.TextBox LocalNameTextBox;
        private System.Windows.Forms.Button OKButton;
        private System.Windows.Forms.Button CancelButton;
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.Container components = null;

        public ComplexTypeProperty()
            : this(null)
        {
        }
        public ComplexTypeProperty(UPnPComplexType c)
        {
            //
            // Required for Windows Form Designer support
            //
            InitializeComponent();
            if (c != null)
            {
                NamespaceTextBox.Text = c.Name_NAMESPACE;
                LocalNameTextBox.Text = c.Name_LOCAL;
            }

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
            this.textBox3 = new System.Windows.Forms.TextBox();
            this.NamespaceTextBox = new System.Windows.Forms.TextBox();
            this.textBox1 = new System.Windows.Forms.TextBox();
            this.LocalNameTextBox = new System.Windows.Forms.TextBox();
            this.OKButton = new System.Windows.Forms.Button();
            this.CancelButton = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // textBox3
            // 
            this.textBox3.Location = new System.Drawing.Point(8, 32);
            this.textBox3.Name = "textBox3";
            this.textBox3.ReadOnly = true;
            this.textBox3.Size = new System.Drawing.Size(72, 20);
            this.textBox3.TabIndex = 5;
            this.textBox3.TabStop = false;
            this.textBox3.Text = "Local Name";
            // 
            // NamespaceTextBox
            // 
            this.NamespaceTextBox.Location = new System.Drawing.Point(80, 8);
            this.NamespaceTextBox.Name = "NamespaceTextBox";
            this.NamespaceTextBox.Size = new System.Drawing.Size(312, 20);
            this.NamespaceTextBox.TabIndex = 4;
            this.NamespaceTextBox.Text = "http://www.vendor.org/Schemas";
            this.NamespaceTextBox.TextChanged += new System.EventHandler(this.OnTextBoxChanged);
            // 
            // textBox1
            // 
            this.textBox1.Location = new System.Drawing.Point(8, 8);
            this.textBox1.Name = "textBox1";
            this.textBox1.ReadOnly = true;
            this.textBox1.Size = new System.Drawing.Size(72, 20);
            this.textBox1.TabIndex = 3;
            this.textBox1.TabStop = false;
            this.textBox1.Text = "Namespace";
            // 
            // LocalNameTextBox
            // 
            this.LocalNameTextBox.Location = new System.Drawing.Point(80, 32);
            this.LocalNameTextBox.Name = "LocalNameTextBox";
            this.LocalNameTextBox.Size = new System.Drawing.Size(312, 20);
            this.LocalNameTextBox.TabIndex = 6;
            this.LocalNameTextBox.TextChanged += new System.EventHandler(this.OnTextBoxChanged);
            // 
            // OKButton
            // 
            this.OKButton.Enabled = false;
            this.OKButton.Location = new System.Drawing.Point(312, 64);
            this.OKButton.Name = "OKButton";
            this.OKButton.Size = new System.Drawing.Size(75, 23);
            this.OKButton.TabIndex = 7;
            this.OKButton.Text = "OK";
            this.OKButton.Click += new System.EventHandler(this.OKButton_Click);
            // 
            // CancelButton
            // 
            this.CancelButton.Location = new System.Drawing.Point(232, 64);
            this.CancelButton.Name = "CancelButton";
            this.CancelButton.Size = new System.Drawing.Size(75, 23);
            this.CancelButton.TabIndex = 8;
            this.CancelButton.Text = "Cancel";
            this.CancelButton.Click += new System.EventHandler(this.CancelButton_Click);
            // 
            // ComplexTypeProperty
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(400, 94);
            this.Controls.Add(this.CancelButton);
            this.Controls.Add(this.OKButton);
            this.Controls.Add(this.LocalNameTextBox);
            this.Controls.Add(this.textBox3);
            this.Controls.Add(this.NamespaceTextBox);
            this.Controls.Add(this.textBox1);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "ComplexTypeProperty";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Complex Type Property";
            this.ResumeLayout(false);
            this.PerformLayout();

        }
        #endregion


        private void OnTextBoxChanged(object sender, System.EventArgs e)
        {
            if (NamespaceTextBox.Text != "" && LocalNameTextBox.Text != "")
            {
                OKButton.Enabled = true;
            }
            else
            {
                OKButton.Enabled = false;
            }
        }

        private void OKButton_Click(object sender, System.EventArgs e)
        {
            DialogResult = DialogResult.OK;
        }

        private void CancelButton_Click(object sender, EventArgs e)
        {
            DialogResult = DialogResult.Cancel;
        }
    }
}
