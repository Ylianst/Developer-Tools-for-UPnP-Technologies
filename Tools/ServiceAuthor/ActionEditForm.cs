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

namespace UPnPAuthor
{
	/// <summary>
	/// Summary description for ActionEditForm.
	/// </summary>
	public class ActionEditForm : System.Windows.Forms.Form
	{
		private OpenSource.UPnP.UPnPAction action = null;

		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Button okButton;
		private System.Windows.Forms.Button cancelButton;
		private System.Windows.Forms.TextBox actionNameTextBox;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Panel argPanel;
		private System.Windows.Forms.Button addArgButton;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public UPnPAction Action 
		{
			get
			{
				return action;
			}
			set 
			{
				action = value;
				SetAction(action);
			}
		}

		public ActionEditForm(UPnPService parentService)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			action = new UPnPAction();
			action.ParentService = parentService;
		}

		private void SetAction(UPnPAction act) 
		{
			actionNameTextBox.Text = act.Name;

			UPnPArgument[] args = act.ArgumentList;
			foreach (UPnPArgument arg in args) 
			{
				UPnpArgumentEditControl argEdit = new UPnpArgumentEditControl(this,act,arg);
				argEdit.Dock = DockStyle.Top;
				argPanel.Controls.Add(argEdit);
				argPanel.Controls.SetChildIndex(argEdit,0);
			}
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
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(ActionEditForm));
			this.actionNameTextBox = new System.Windows.Forms.TextBox();
			this.label1 = new System.Windows.Forms.Label();
			this.argPanel = new System.Windows.Forms.Panel();
			this.okButton = new System.Windows.Forms.Button();
			this.cancelButton = new System.Windows.Forms.Button();
			this.addArgButton = new System.Windows.Forms.Button();
			this.label2 = new System.Windows.Forms.Label();
			this.SuspendLayout();
			// 
			// actionNameTextBox
			// 
			this.actionNameTextBox.AccessibleDescription = ((string)(resources.GetObject("actionNameTextBox.AccessibleDescription")));
			this.actionNameTextBox.AccessibleName = ((string)(resources.GetObject("actionNameTextBox.AccessibleName")));
			this.actionNameTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(resources.GetObject("actionNameTextBox.Anchor")));
			this.actionNameTextBox.AutoSize = ((bool)(resources.GetObject("actionNameTextBox.AutoSize")));
			this.actionNameTextBox.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("actionNameTextBox.BackgroundImage")));
			this.actionNameTextBox.Dock = ((System.Windows.Forms.DockStyle)(resources.GetObject("actionNameTextBox.Dock")));
			this.actionNameTextBox.Enabled = ((bool)(resources.GetObject("actionNameTextBox.Enabled")));
			this.actionNameTextBox.Font = ((System.Drawing.Font)(resources.GetObject("actionNameTextBox.Font")));
			this.actionNameTextBox.ImeMode = ((System.Windows.Forms.ImeMode)(resources.GetObject("actionNameTextBox.ImeMode")));
			this.actionNameTextBox.Location = ((System.Drawing.Point)(resources.GetObject("actionNameTextBox.Location")));
			this.actionNameTextBox.MaxLength = ((int)(resources.GetObject("actionNameTextBox.MaxLength")));
			this.actionNameTextBox.Multiline = ((bool)(resources.GetObject("actionNameTextBox.Multiline")));
			this.actionNameTextBox.Name = "actionNameTextBox";
			this.actionNameTextBox.PasswordChar = ((char)(resources.GetObject("actionNameTextBox.PasswordChar")));
			this.actionNameTextBox.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("actionNameTextBox.RightToLeft")));
			this.actionNameTextBox.ScrollBars = ((System.Windows.Forms.ScrollBars)(resources.GetObject("actionNameTextBox.ScrollBars")));
			this.actionNameTextBox.Size = ((System.Drawing.Size)(resources.GetObject("actionNameTextBox.Size")));
			this.actionNameTextBox.TabIndex = ((int)(resources.GetObject("actionNameTextBox.TabIndex")));
			this.actionNameTextBox.Text = resources.GetString("actionNameTextBox.Text");
			this.actionNameTextBox.TextAlign = ((System.Windows.Forms.HorizontalAlignment)(resources.GetObject("actionNameTextBox.TextAlign")));
			this.actionNameTextBox.Visible = ((bool)(resources.GetObject("actionNameTextBox.Visible")));
			this.actionNameTextBox.WordWrap = ((bool)(resources.GetObject("actionNameTextBox.WordWrap")));
			// 
			// label1
			// 
			this.label1.AccessibleDescription = ((string)(resources.GetObject("label1.AccessibleDescription")));
			this.label1.AccessibleName = ((string)(resources.GetObject("label1.AccessibleName")));
			this.label1.Anchor = ((System.Windows.Forms.AnchorStyles)(resources.GetObject("label1.Anchor")));
			this.label1.AutoSize = ((bool)(resources.GetObject("label1.AutoSize")));
			this.label1.Dock = ((System.Windows.Forms.DockStyle)(resources.GetObject("label1.Dock")));
			this.label1.Enabled = ((bool)(resources.GetObject("label1.Enabled")));
			this.label1.Font = ((System.Drawing.Font)(resources.GetObject("label1.Font")));
			this.label1.Image = ((System.Drawing.Image)(resources.GetObject("label1.Image")));
			this.label1.ImageAlign = ((System.Drawing.ContentAlignment)(resources.GetObject("label1.ImageAlign")));
			this.label1.ImageIndex = ((int)(resources.GetObject("label1.ImageIndex")));
			this.label1.ImeMode = ((System.Windows.Forms.ImeMode)(resources.GetObject("label1.ImeMode")));
			this.label1.Location = ((System.Drawing.Point)(resources.GetObject("label1.Location")));
			this.label1.Name = "label1";
			this.label1.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("label1.RightToLeft")));
			this.label1.Size = ((System.Drawing.Size)(resources.GetObject("label1.Size")));
			this.label1.TabIndex = ((int)(resources.GetObject("label1.TabIndex")));
			this.label1.Text = resources.GetString("label1.Text");
			this.label1.TextAlign = ((System.Drawing.ContentAlignment)(resources.GetObject("label1.TextAlign")));
			this.label1.Visible = ((bool)(resources.GetObject("label1.Visible")));
			// 
			// argPanel
			// 
			this.argPanel.AccessibleDescription = ((string)(resources.GetObject("argPanel.AccessibleDescription")));
			this.argPanel.AccessibleName = ((string)(resources.GetObject("argPanel.AccessibleName")));
			this.argPanel.Anchor = ((System.Windows.Forms.AnchorStyles)(resources.GetObject("argPanel.Anchor")));
			this.argPanel.AutoScroll = ((bool)(resources.GetObject("argPanel.AutoScroll")));
			this.argPanel.AutoScrollMargin = ((System.Drawing.Size)(resources.GetObject("argPanel.AutoScrollMargin")));
			this.argPanel.AutoScrollMinSize = ((System.Drawing.Size)(resources.GetObject("argPanel.AutoScrollMinSize")));
			this.argPanel.BackColor = System.Drawing.SystemColors.AppWorkspace;
			this.argPanel.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("argPanel.BackgroundImage")));
			this.argPanel.Dock = ((System.Windows.Forms.DockStyle)(resources.GetObject("argPanel.Dock")));
			this.argPanel.Enabled = ((bool)(resources.GetObject("argPanel.Enabled")));
			this.argPanel.Font = ((System.Drawing.Font)(resources.GetObject("argPanel.Font")));
			this.argPanel.ImeMode = ((System.Windows.Forms.ImeMode)(resources.GetObject("argPanel.ImeMode")));
			this.argPanel.Location = ((System.Drawing.Point)(resources.GetObject("argPanel.Location")));
			this.argPanel.Name = "argPanel";
			this.argPanel.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("argPanel.RightToLeft")));
			this.argPanel.Size = ((System.Drawing.Size)(resources.GetObject("argPanel.Size")));
			this.argPanel.TabIndex = ((int)(resources.GetObject("argPanel.TabIndex")));
			this.argPanel.Text = resources.GetString("argPanel.Text");
			this.argPanel.Visible = ((bool)(resources.GetObject("argPanel.Visible")));
			// 
			// okButton
			// 
			this.okButton.AccessibleDescription = ((string)(resources.GetObject("okButton.AccessibleDescription")));
			this.okButton.AccessibleName = ((string)(resources.GetObject("okButton.AccessibleName")));
			this.okButton.Anchor = ((System.Windows.Forms.AnchorStyles)(resources.GetObject("okButton.Anchor")));
			this.okButton.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("okButton.BackgroundImage")));
			this.okButton.Dock = ((System.Windows.Forms.DockStyle)(resources.GetObject("okButton.Dock")));
			this.okButton.Enabled = ((bool)(resources.GetObject("okButton.Enabled")));
			this.okButton.FlatStyle = ((System.Windows.Forms.FlatStyle)(resources.GetObject("okButton.FlatStyle")));
			this.okButton.Font = ((System.Drawing.Font)(resources.GetObject("okButton.Font")));
			this.okButton.Image = ((System.Drawing.Image)(resources.GetObject("okButton.Image")));
			this.okButton.ImageAlign = ((System.Drawing.ContentAlignment)(resources.GetObject("okButton.ImageAlign")));
			this.okButton.ImageIndex = ((int)(resources.GetObject("okButton.ImageIndex")));
			this.okButton.ImeMode = ((System.Windows.Forms.ImeMode)(resources.GetObject("okButton.ImeMode")));
			this.okButton.Location = ((System.Drawing.Point)(resources.GetObject("okButton.Location")));
			this.okButton.Name = "okButton";
			this.okButton.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("okButton.RightToLeft")));
			this.okButton.Size = ((System.Drawing.Size)(resources.GetObject("okButton.Size")));
			this.okButton.TabIndex = ((int)(resources.GetObject("okButton.TabIndex")));
			this.okButton.Text = resources.GetString("okButton.Text");
			this.okButton.TextAlign = ((System.Drawing.ContentAlignment)(resources.GetObject("okButton.TextAlign")));
			this.okButton.Visible = ((bool)(resources.GetObject("okButton.Visible")));
			this.okButton.Click += new System.EventHandler(this.okButton_Click);
			// 
			// cancelButton
			// 
			this.cancelButton.AccessibleDescription = ((string)(resources.GetObject("cancelButton.AccessibleDescription")));
			this.cancelButton.AccessibleName = ((string)(resources.GetObject("cancelButton.AccessibleName")));
			this.cancelButton.Anchor = ((System.Windows.Forms.AnchorStyles)(resources.GetObject("cancelButton.Anchor")));
			this.cancelButton.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("cancelButton.BackgroundImage")));
			this.cancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.cancelButton.Dock = ((System.Windows.Forms.DockStyle)(resources.GetObject("cancelButton.Dock")));
			this.cancelButton.Enabled = ((bool)(resources.GetObject("cancelButton.Enabled")));
			this.cancelButton.FlatStyle = ((System.Windows.Forms.FlatStyle)(resources.GetObject("cancelButton.FlatStyle")));
			this.cancelButton.Font = ((System.Drawing.Font)(resources.GetObject("cancelButton.Font")));
			this.cancelButton.Image = ((System.Drawing.Image)(resources.GetObject("cancelButton.Image")));
			this.cancelButton.ImageAlign = ((System.Drawing.ContentAlignment)(resources.GetObject("cancelButton.ImageAlign")));
			this.cancelButton.ImageIndex = ((int)(resources.GetObject("cancelButton.ImageIndex")));
			this.cancelButton.ImeMode = ((System.Windows.Forms.ImeMode)(resources.GetObject("cancelButton.ImeMode")));
			this.cancelButton.Location = ((System.Drawing.Point)(resources.GetObject("cancelButton.Location")));
			this.cancelButton.Name = "cancelButton";
			this.cancelButton.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("cancelButton.RightToLeft")));
			this.cancelButton.Size = ((System.Drawing.Size)(resources.GetObject("cancelButton.Size")));
			this.cancelButton.TabIndex = ((int)(resources.GetObject("cancelButton.TabIndex")));
			this.cancelButton.Text = resources.GetString("cancelButton.Text");
			this.cancelButton.TextAlign = ((System.Drawing.ContentAlignment)(resources.GetObject("cancelButton.TextAlign")));
			this.cancelButton.Visible = ((bool)(resources.GetObject("cancelButton.Visible")));
			this.cancelButton.Click += new System.EventHandler(this.cancelButton_Click);
			// 
			// addArgButton
			// 
			this.addArgButton.AccessibleDescription = ((string)(resources.GetObject("addArgButton.AccessibleDescription")));
			this.addArgButton.AccessibleName = ((string)(resources.GetObject("addArgButton.AccessibleName")));
			this.addArgButton.Anchor = ((System.Windows.Forms.AnchorStyles)(resources.GetObject("addArgButton.Anchor")));
			this.addArgButton.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("addArgButton.BackgroundImage")));
			this.addArgButton.Dock = ((System.Windows.Forms.DockStyle)(resources.GetObject("addArgButton.Dock")));
			this.addArgButton.Enabled = ((bool)(resources.GetObject("addArgButton.Enabled")));
			this.addArgButton.FlatStyle = ((System.Windows.Forms.FlatStyle)(resources.GetObject("addArgButton.FlatStyle")));
			this.addArgButton.Font = ((System.Drawing.Font)(resources.GetObject("addArgButton.Font")));
			this.addArgButton.Image = ((System.Drawing.Image)(resources.GetObject("addArgButton.Image")));
			this.addArgButton.ImageAlign = ((System.Drawing.ContentAlignment)(resources.GetObject("addArgButton.ImageAlign")));
			this.addArgButton.ImageIndex = ((int)(resources.GetObject("addArgButton.ImageIndex")));
			this.addArgButton.ImeMode = ((System.Windows.Forms.ImeMode)(resources.GetObject("addArgButton.ImeMode")));
			this.addArgButton.Location = ((System.Drawing.Point)(resources.GetObject("addArgButton.Location")));
			this.addArgButton.Name = "addArgButton";
			this.addArgButton.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("addArgButton.RightToLeft")));
			this.addArgButton.Size = ((System.Drawing.Size)(resources.GetObject("addArgButton.Size")));
			this.addArgButton.TabIndex = ((int)(resources.GetObject("addArgButton.TabIndex")));
			this.addArgButton.Text = resources.GetString("addArgButton.Text");
			this.addArgButton.TextAlign = ((System.Drawing.ContentAlignment)(resources.GetObject("addArgButton.TextAlign")));
			this.addArgButton.Visible = ((bool)(resources.GetObject("addArgButton.Visible")));
			this.addArgButton.Click += new System.EventHandler(this.addArgButton_Click);
			// 
			// label2
			// 
			this.label2.AccessibleDescription = ((string)(resources.GetObject("label2.AccessibleDescription")));
			this.label2.AccessibleName = ((string)(resources.GetObject("label2.AccessibleName")));
			this.label2.Anchor = ((System.Windows.Forms.AnchorStyles)(resources.GetObject("label2.Anchor")));
			this.label2.AutoSize = ((bool)(resources.GetObject("label2.AutoSize")));
			this.label2.Dock = ((System.Windows.Forms.DockStyle)(resources.GetObject("label2.Dock")));
			this.label2.Enabled = ((bool)(resources.GetObject("label2.Enabled")));
			this.label2.Font = ((System.Drawing.Font)(resources.GetObject("label2.Font")));
			this.label2.Image = ((System.Drawing.Image)(resources.GetObject("label2.Image")));
			this.label2.ImageAlign = ((System.Drawing.ContentAlignment)(resources.GetObject("label2.ImageAlign")));
			this.label2.ImageIndex = ((int)(resources.GetObject("label2.ImageIndex")));
			this.label2.ImeMode = ((System.Windows.Forms.ImeMode)(resources.GetObject("label2.ImeMode")));
			this.label2.Location = ((System.Drawing.Point)(resources.GetObject("label2.Location")));
			this.label2.Name = "label2";
			this.label2.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("label2.RightToLeft")));
			this.label2.Size = ((System.Drawing.Size)(resources.GetObject("label2.Size")));
			this.label2.TabIndex = ((int)(resources.GetObject("label2.TabIndex")));
			this.label2.Text = resources.GetString("label2.Text");
			this.label2.TextAlign = ((System.Drawing.ContentAlignment)(resources.GetObject("label2.TextAlign")));
			this.label2.Visible = ((bool)(resources.GetObject("label2.Visible")));
			// 
			// ActionEditForm
			// 
			this.AcceptButton = this.okButton;
			this.AccessibleDescription = ((string)(resources.GetObject("$this.AccessibleDescription")));
			this.AccessibleName = ((string)(resources.GetObject("$this.AccessibleName")));
			this.Anchor = ((System.Windows.Forms.AnchorStyles)(resources.GetObject("$this.Anchor")));
			this.AutoScaleBaseSize = ((System.Drawing.Size)(resources.GetObject("$this.AutoScaleBaseSize")));
			this.AutoScroll = ((bool)(resources.GetObject("$this.AutoScroll")));
			this.AutoScrollMargin = ((System.Drawing.Size)(resources.GetObject("$this.AutoScrollMargin")));
			this.AutoScrollMinSize = ((System.Drawing.Size)(resources.GetObject("$this.AutoScrollMinSize")));
			this.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("$this.BackgroundImage")));
			this.CancelButton = this.cancelButton;
			this.ClientSize = ((System.Drawing.Size)(resources.GetObject("$this.ClientSize")));
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.argPanel,
																		  this.label2,
																		  this.addArgButton,
																		  this.cancelButton,
																		  this.okButton,
																		  this.label1,
																		  this.actionNameTextBox});
			this.Dock = ((System.Windows.Forms.DockStyle)(resources.GetObject("$this.Dock")));
			this.Enabled = ((bool)(resources.GetObject("$this.Enabled")));
			this.Font = ((System.Drawing.Font)(resources.GetObject("$this.Font")));
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.ImeMode = ((System.Windows.Forms.ImeMode)(resources.GetObject("$this.ImeMode")));
			this.Location = ((System.Drawing.Point)(resources.GetObject("$this.Location")));
			this.MaximumSize = ((System.Drawing.Size)(resources.GetObject("$this.MaximumSize")));
			this.MinimumSize = ((System.Drawing.Size)(resources.GetObject("$this.MinimumSize")));
			this.Name = "ActionEditForm";
			this.RightToLeft = ((System.Windows.Forms.RightToLeft)(resources.GetObject("$this.RightToLeft")));
			this.StartPosition = ((System.Windows.Forms.FormStartPosition)(resources.GetObject("$this.StartPosition")));
			this.Text = resources.GetString("$this.Text");
			this.Visible = ((bool)(resources.GetObject("$this.Visible")));
			this.ResumeLayout(false);

		}
		#endregion

		private void okButton_Click(object sender, System.EventArgs e)
		{
			if (actionNameTextBox.Text.Length == 0) 
			{
				MessageBox.Show(this,"Actions must have a name","Action",MessageBoxButtons.OK,MessageBoxIcon.Asterisk);
				return;
			}
			int returnArg = 0;
			foreach (UPnpArgumentEditControl argEdit in argPanel.Controls) 
			{
				if (argEdit.UPnPArgument.IsReturnValue == true) {returnArg++;}
				if (argEdit.UPnPArgument.Name.Length == 0) 
				{
					MessageBox.Show(this,"All arguments must have names","Action",MessageBoxButtons.OK,MessageBoxIcon.Asterisk);
					return;
				}
			}
			if (returnArg > 1) 
			{
				MessageBox.Show(this,"Only a single return argument is allowed","Action",MessageBoxButtons.OK,MessageBoxIcon.Asterisk);
				return;
			}

			// Re-serialize
			action.Name = actionNameTextBox.Text;

			ArrayList argList = new ArrayList();
			foreach (UPnpArgumentEditControl argEdit in argPanel.Controls) 
			{
				argList.Insert(0,argEdit.UPnPArgument);
			}
			action.Arguments = argList;

			this.DialogResult = DialogResult.OK;
		}

		private void cancelButton_Click(object sender, System.EventArgs e)
		{
			this.DialogResult = DialogResult.Cancel;		
		}

		private void addArgButton_Click(object sender, System.EventArgs e)
		{
			if (action == null) return;

			if (action.ParentService.GetStateVariables().Length == 0) 
			{
				MessageBox.Show(this,"No state variables available","Argument Creation",MessageBoxButtons.OK,MessageBoxIcon.Asterisk);
				return;
			}

			UPnPArgument arg = new UPnPArgument("",null);
			arg.Direction = "in";
			UPnpArgumentEditControl argEdit = new UPnpArgumentEditControl(this,action,arg);
			argEdit.Dock = DockStyle.Top;
			ArrayList list = new ArrayList(argPanel.Controls);
			list.Insert(0, argEdit);
			argPanel.Controls.Clear();
			argPanel.Controls.AddRange(list.ToArray(typeof(Control)) as Control[]);
		}

		public void moveArgUp(UPnpArgumentEditControl ctrl) 
		{
			int pos = argPanel.Controls.GetChildIndex(ctrl,false);
			if (pos > 0) 
			{
				argPanel.Controls.SetChildIndex(ctrl,pos-1);
			}
		}

		public void moveArgDown(UPnpArgumentEditControl ctrl) 
		{
			int pos = argPanel.Controls.GetChildIndex(ctrl,false);
			if (pos >= 0) 
			{
				argPanel.Controls.SetChildIndex(ctrl,pos+1);
			}
		}

		public void RemoveArg(UPnpArgumentEditControl ctrl) 
		{
			argPanel.Controls.Remove(ctrl);
		}

		public void moveArgTop(UPnpArgumentEditControl ctrl) 
		{
			int pos = argPanel.Controls.GetChildIndex(ctrl,false);
			argPanel.Controls.SetChildIndex(ctrl,0);
		}

		public void moveArgBottom(UPnpArgumentEditControl ctrl) 
		{
			int pos = argPanel.Controls.GetChildIndex(ctrl,false);
			argPanel.Controls.SetChildIndex(ctrl,999);
		}

	}
}
