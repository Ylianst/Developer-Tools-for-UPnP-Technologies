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
using System.Net;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace UPnPSniffer
{
    /// <summary>
    /// Summary description for AddressFilterForm.
    /// </summary>
    public class AddressFilterForm : System.Windows.Forms.Form
    {
        private System.Windows.Forms.Button okButton;
        private System.Windows.Forms.Button cancelButton;
        private System.Windows.Forms.TextBox addrTextBox1;
        private System.Windows.Forms.RadioButton ipAddrFilterRadioButton;
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.Container components = null;
        private System.Windows.Forms.RadioButton noFilterRadioButton;

        private IPAddress filteraddress;

        public IPAddress FilterAddress
        {
            get { return filteraddress; }
            set
            {
                if (value == null)
                {
                    noFilterRadioButton.Checked = true;
                    ipAddrFilterRadioButton.Checked = false;
                    addrTextBox1.Text = "";
                }
                else
                {
                    noFilterRadioButton.Checked = false;
                    ipAddrFilterRadioButton.Checked = true;
                    addrTextBox1.Text = value.ToString();
                }
                addrTextBox1.Enabled = ipAddrFilterRadioButton.Checked;
            }
        }

        public AddressFilterForm()
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(AddressFilterForm));
            this.okButton = new System.Windows.Forms.Button();
            this.cancelButton = new System.Windows.Forms.Button();
            this.noFilterRadioButton = new System.Windows.Forms.RadioButton();
            this.ipAddrFilterRadioButton = new System.Windows.Forms.RadioButton();
            this.addrTextBox1 = new System.Windows.Forms.TextBox();
            this.SuspendLayout();
            // 
            // okButton
            // 
            this.okButton.Location = new System.Drawing.Point(200, 96);
            this.okButton.Name = "okButton";
            this.okButton.Size = new System.Drawing.Size(80, 24);
            this.okButton.TabIndex = 0;
            this.okButton.Text = "OK";
            this.okButton.Click += new System.EventHandler(this.okButton_Click);
            // 
            // cancelButton
            // 
            this.cancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.cancelButton.Location = new System.Drawing.Point(112, 96);
            this.cancelButton.Name = "cancelButton";
            this.cancelButton.Size = new System.Drawing.Size(80, 24);
            this.cancelButton.TabIndex = 1;
            this.cancelButton.Text = "Cancel";
            // 
            // noFilterRadioButton
            // 
            this.noFilterRadioButton.Checked = true;
            this.noFilterRadioButton.Location = new System.Drawing.Point(16, 16);
            this.noFilterRadioButton.Name = "noFilterRadioButton";
            this.noFilterRadioButton.Size = new System.Drawing.Size(264, 16);
            this.noFilterRadioButton.TabIndex = 2;
            this.noFilterRadioButton.TabStop = true;
            this.noFilterRadioButton.Text = "No filter, capture all traffic";
            this.noFilterRadioButton.CheckedChanged += new System.EventHandler(this.noFilterRadioButton_CheckedChanged);
            // 
            // ipAddrFilterRadioButton
            // 
            this.ipAddrFilterRadioButton.Location = new System.Drawing.Point(16, 40);
            this.ipAddrFilterRadioButton.Name = "ipAddrFilterRadioButton";
            this.ipAddrFilterRadioButton.Size = new System.Drawing.Size(264, 16);
            this.ipAddrFilterRadioButton.TabIndex = 3;
            this.ipAddrFilterRadioButton.Text = "Use IP address filter";
            this.ipAddrFilterRadioButton.CheckedChanged += new System.EventHandler(this.ipAddrFilterRadioButton_CheckedChanged);
            // 
            // addrTextBox1
            // 
            this.addrTextBox1.Enabled = false;
            this.addrTextBox1.Location = new System.Drawing.Point(40, 64);
            this.addrTextBox1.Name = "addrTextBox1";
            this.addrTextBox1.Size = new System.Drawing.Size(240, 20);
            this.addrTextBox1.TabIndex = 4;
            this.addrTextBox1.TextChanged += new System.EventHandler(this.addrTextBox1_TextChanged);
            // 
            // AddressFilterForm
            // 
            this.AcceptButton = this.okButton;
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.CancelButton = this.cancelButton;
            this.ClientSize = new System.Drawing.Size(296, 126);
            this.Controls.Add(this.addrTextBox1);
            this.Controls.Add(this.ipAddrFilterRadioButton);
            this.Controls.Add(this.noFilterRadioButton);
            this.Controls.Add(this.cancelButton);
            this.Controls.Add(this.okButton);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "AddressFilterForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Address Filter";
            this.Load += new System.EventHandler(this.AddressFilterForm_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }
        #endregion

        private void okButton_Click(object sender, System.EventArgs e)
        {
            if (ipAddrFilterRadioButton.Checked == true)
            {
                filteraddress = IPAddress.Parse(addrTextBox1.Text);
            }
            else
            {
                filteraddress = null;
            }
            DialogResult = DialogResult.OK;
        }

        private void noFilterRadioButton_CheckedChanged(object sender, System.EventArgs e)
        {
            UpdateInfo();
        }

        private void ipAddrFilterRadioButton_CheckedChanged(object sender, System.EventArgs e)
        {
            UpdateInfo();
        }

        private void UpdateInfo()
        {
            noFilterRadioButton.Checked = !ipAddrFilterRadioButton.Checked;
            addrTextBox1.Enabled = ipAddrFilterRadioButton.Checked;
            if (noFilterRadioButton.Checked == false)
            {
                IPAddress x;
                okButton.Enabled = IPAddress.TryParse(addrTextBox1.Text, out x);
            }
            else
            {
                okButton.Enabled = true;
            }
        }

        private void addrTextBox1_TextChanged(object sender, EventArgs e)
        {
            UpdateInfo();
        }

        private void AddressFilterForm_Load(object sender, EventArgs e)
        {
            UpdateInfo();
        }

    }
}
