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
using OpenSource.UPnP.AV.RENDERER.Device;

namespace UPnPRenderer
{
	/// <summary>
	/// Summary description for ImageVideoForm.
	/// </summary>
	public class ImageVideoForm : System.Windows.Forms.Form
	{
		private AVConnection Connection;
		private new RendererForm Parent;

		private System.Windows.Forms.GroupBox groupBox1;
		private System.Windows.Forms.TrackBar BlueBlack;
		private System.Windows.Forms.TrackBar GreenBlack;
		private System.Windows.Forms.TrackBar RedBlack;
		private System.Windows.Forms.GroupBox groupBox2;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.Label label4;
		private System.Windows.Forms.Label label5;
		private System.Windows.Forms.Label label6;
		private System.Windows.Forms.TrackBar BlueGain;
		private System.Windows.Forms.TrackBar GreenGain;
		private System.Windows.Forms.TrackBar RedGain;
		private System.Windows.Forms.TrackBar Sharpness;
		private System.Windows.Forms.TrackBar Contrast;
		private System.Windows.Forms.TrackBar Brightness;
		private System.Windows.Forms.Label label7;
		private System.Windows.Forms.Label label8;
		private System.Windows.Forms.Label label9;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public ImageVideoForm(RendererForm referer, AVConnection c)
		{
			Connection = c;
			Parent = referer;
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();
			this.Text = "Image/Video Controls  [" + c.ID.ToString() + "]";
			RedBlack.Value = (int)c.RedVideoBlackLevel;
			RedGain.Value = (int)c.RedVideoGain;
			GreenBlack.Value = (int)c.GreenVideoBlackLevel;
			GreenGain.Value = (int)c.GreenVideoGain;
			BlueBlack.Value = (int)c.BlueVideoBlackLevel;
			BlueGain.Value = (int)c.BlueVideoGain;

			c.OnBlueVideoBlackLevelChanged += new AVConnection.VariableChangedHandler(BlueBlackSink);
			c.OnBlueVideoGainChanged += new AVConnection.VariableChangedHandler(BlueGainSink);
			c.OnGreenVideoBlackLevelChanged += new AVConnection.VariableChangedHandler(GreenBlackSink);
			c.OnGreenVideoGainChanged += new AVConnection.VariableChangedHandler(GreenGainSink);
			c.OnRedVideoBlackLevelChanged += new AVConnection.VariableChangedHandler(RedBlackSink);
			c.OnRedVideoGainChanged += new AVConnection.VariableChangedHandler(RedGainSink);
		
			c.OnSharpnessChanged += new AVConnection.VariableChangedHandler(SharpnessSink);
			c.OnBrightnessChanged += new AVConnection.VariableChangedHandler(BrightnessSink);
			c.OnContrastChanged += new AVConnection.VariableChangedHandler(ContrastSink);
		
		}


		protected void SharpnessSink(AVConnection sender)
		{
			Sharpness.Value = (int)sender.Sharpness;
		}
		protected void BrightnessSink(AVConnection sender)
		{
			Brightness.Value = (int)sender.Brightness;
		}
		protected void ContrastSink(AVConnection sender)
		{
			Contrast.Value = (int)sender.Contrast;
		}

		protected void RedBlackSink(AVConnection sender)
		{
			RedBlack.Value = (int)sender.RedVideoBlackLevel;
		}
		protected void RedGainSink(AVConnection sender)
		{
			RedGain.Value = (int)sender.RedVideoGain;
		}
		protected void GreenBlackSink(AVConnection sender)
		{
			GreenBlack.Value = (int)sender.GreenVideoBlackLevel;
		}
		protected void GreenGainSink(AVConnection sender)
		{
			GreenGain.Value = (int)sender.GreenVideoGain;
		}
		protected void BlueBlackSink(AVConnection sender)
		{
			BlueBlack.Value = (int)sender.BlueVideoBlackLevel;
		}
		protected void BlueGainSink(AVConnection sender)
		{
			BlueGain.Value = (int)sender.BlueVideoGain;
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
			this.groupBox1 = new System.Windows.Forms.GroupBox();
			this.BlueBlack = new System.Windows.Forms.TrackBar();
			this.GreenBlack = new System.Windows.Forms.TrackBar();
			this.RedBlack = new System.Windows.Forms.TrackBar();
			this.groupBox2 = new System.Windows.Forms.GroupBox();
			this.label1 = new System.Windows.Forms.Label();
			this.label2 = new System.Windows.Forms.Label();
			this.label3 = new System.Windows.Forms.Label();
			this.label4 = new System.Windows.Forms.Label();
			this.label5 = new System.Windows.Forms.Label();
			this.label6 = new System.Windows.Forms.Label();
			this.BlueGain = new System.Windows.Forms.TrackBar();
			this.GreenGain = new System.Windows.Forms.TrackBar();
			this.RedGain = new System.Windows.Forms.TrackBar();
			this.Sharpness = new System.Windows.Forms.TrackBar();
			this.Contrast = new System.Windows.Forms.TrackBar();
			this.Brightness = new System.Windows.Forms.TrackBar();
			this.label7 = new System.Windows.Forms.Label();
			this.label8 = new System.Windows.Forms.Label();
			this.label9 = new System.Windows.Forms.Label();
			this.groupBox1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.BlueBlack)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.GreenBlack)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.RedBlack)).BeginInit();
			this.groupBox2.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.BlueGain)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.GreenGain)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.RedGain)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.Sharpness)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.Contrast)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.Brightness)).BeginInit();
			this.SuspendLayout();
			// 
			// groupBox1
			// 
			this.groupBox1.Controls.AddRange(new System.Windows.Forms.Control[] {
																					this.label3,
																					this.label2,
																					this.label1,
																					this.BlueBlack,
																					this.GreenBlack,
																					this.RedBlack});
			this.groupBox1.Location = new System.Drawing.Point(16, 16);
			this.groupBox1.Name = "groupBox1";
			this.groupBox1.Size = new System.Drawing.Size(184, 192);
			this.groupBox1.TabIndex = 0;
			this.groupBox1.TabStop = false;
			this.groupBox1.Text = "Black Level";
			// 
			// BlueBlack
			// 
			this.BlueBlack.Location = new System.Drawing.Point(120, 24);
			this.BlueBlack.Maximum = 100;
			this.BlueBlack.Name = "BlueBlack";
			this.BlueBlack.Orientation = System.Windows.Forms.Orientation.Vertical;
			this.BlueBlack.Size = new System.Drawing.Size(45, 128);
			this.BlueBlack.TabIndex = 5;
			this.BlueBlack.TickStyle = System.Windows.Forms.TickStyle.None;
			this.BlueBlack.Scroll += new System.EventHandler(this.ChangeBlueBlack);
			// 
			// GreenBlack
			// 
			this.GreenBlack.Location = new System.Drawing.Point(72, 24);
			this.GreenBlack.Maximum = 100;
			this.GreenBlack.Name = "GreenBlack";
			this.GreenBlack.Orientation = System.Windows.Forms.Orientation.Vertical;
			this.GreenBlack.Size = new System.Drawing.Size(45, 128);
			this.GreenBlack.TabIndex = 4;
			this.GreenBlack.TickStyle = System.Windows.Forms.TickStyle.None;
			this.GreenBlack.Scroll += new System.EventHandler(this.ChangeGreenBlack);
			// 
			// RedBlack
			// 
			this.RedBlack.Location = new System.Drawing.Point(24, 24);
			this.RedBlack.Maximum = 100;
			this.RedBlack.Name = "RedBlack";
			this.RedBlack.Orientation = System.Windows.Forms.Orientation.Vertical;
			this.RedBlack.Size = new System.Drawing.Size(45, 128);
			this.RedBlack.TabIndex = 3;
			this.RedBlack.TickStyle = System.Windows.Forms.TickStyle.None;
			this.RedBlack.Scroll += new System.EventHandler(this.ChangeRedBlack);
			// 
			// groupBox2
			// 
			this.groupBox2.Controls.AddRange(new System.Windows.Forms.Control[] {
																					this.label4,
																					this.label5,
																					this.label6,
																					this.BlueGain,
																					this.GreenGain,
																					this.RedGain});
			this.groupBox2.Location = new System.Drawing.Point(216, 16);
			this.groupBox2.Name = "groupBox2";
			this.groupBox2.Size = new System.Drawing.Size(184, 192);
			this.groupBox2.TabIndex = 1;
			this.groupBox2.TabStop = false;
			this.groupBox2.Text = "Gain Level";
			// 
			// label1
			// 
			this.label1.Font = new System.Drawing.Font("Arial", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.label1.ForeColor = System.Drawing.Color.Red;
			this.label1.Location = new System.Drawing.Point(27, 160);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(16, 16);
			this.label1.TabIndex = 6;
			this.label1.Text = "R";
			// 
			// label2
			// 
			this.label2.Font = new System.Drawing.Font("Arial", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.label2.ForeColor = System.Drawing.Color.Green;
			this.label2.Location = new System.Drawing.Point(74, 160);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(24, 24);
			this.label2.TabIndex = 7;
			this.label2.Text = "G";
			// 
			// label3
			// 
			this.label3.Font = new System.Drawing.Font("Arial", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.label3.ForeColor = System.Drawing.Color.Blue;
			this.label3.Location = new System.Drawing.Point(122, 160);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(24, 24);
			this.label3.TabIndex = 8;
			this.label3.Text = "B";
			// 
			// label4
			// 
			this.label4.Font = new System.Drawing.Font("Arial", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.label4.ForeColor = System.Drawing.Color.Blue;
			this.label4.Location = new System.Drawing.Point(124, 156);
			this.label4.Name = "label4";
			this.label4.Size = new System.Drawing.Size(24, 24);
			this.label4.TabIndex = 14;
			this.label4.Text = "B";
			// 
			// label5
			// 
			this.label5.Font = new System.Drawing.Font("Arial", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.label5.ForeColor = System.Drawing.Color.Green;
			this.label5.Location = new System.Drawing.Point(76, 156);
			this.label5.Name = "label5";
			this.label5.Size = new System.Drawing.Size(24, 24);
			this.label5.TabIndex = 13;
			this.label5.Text = "G";
			// 
			// label6
			// 
			this.label6.Font = new System.Drawing.Font("Arial", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.label6.ForeColor = System.Drawing.Color.Red;
			this.label6.Location = new System.Drawing.Point(29, 156);
			this.label6.Name = "label6";
			this.label6.Size = new System.Drawing.Size(16, 16);
			this.label6.TabIndex = 12;
			this.label6.Text = "R";
			// 
			// BlueGain
			// 
			this.BlueGain.Location = new System.Drawing.Point(122, 20);
			this.BlueGain.Maximum = 100;
			this.BlueGain.Name = "BlueGain";
			this.BlueGain.Orientation = System.Windows.Forms.Orientation.Vertical;
			this.BlueGain.Size = new System.Drawing.Size(45, 128);
			this.BlueGain.TabIndex = 11;
			this.BlueGain.TickStyle = System.Windows.Forms.TickStyle.None;
			this.BlueGain.Scroll += new System.EventHandler(this.ChangeBlueGain);
			// 
			// GreenGain
			// 
			this.GreenGain.Location = new System.Drawing.Point(74, 20);
			this.GreenGain.Maximum = 100;
			this.GreenGain.Name = "GreenGain";
			this.GreenGain.Orientation = System.Windows.Forms.Orientation.Vertical;
			this.GreenGain.Size = new System.Drawing.Size(45, 128);
			this.GreenGain.TabIndex = 10;
			this.GreenGain.TickStyle = System.Windows.Forms.TickStyle.None;
			this.GreenGain.Scroll += new System.EventHandler(this.ChangeGreenGain);
			// 
			// RedGain
			// 
			this.RedGain.Location = new System.Drawing.Point(26, 20);
			this.RedGain.Maximum = 100;
			this.RedGain.Name = "RedGain";
			this.RedGain.Orientation = System.Windows.Forms.Orientation.Vertical;
			this.RedGain.Size = new System.Drawing.Size(45, 128);
			this.RedGain.TabIndex = 9;
			this.RedGain.TickStyle = System.Windows.Forms.TickStyle.None;
			this.RedGain.Scroll += new System.EventHandler(this.ChangeRedGain);
			// 
			// Sharpness
			// 
			this.Sharpness.Location = new System.Drawing.Point(280, 248);
			this.Sharpness.Maximum = 100;
			this.Sharpness.Name = "Sharpness";
			this.Sharpness.Size = new System.Drawing.Size(128, 45);
			this.Sharpness.TabIndex = 8;
			this.Sharpness.TickStyle = System.Windows.Forms.TickStyle.None;
			this.Sharpness.Scroll += new System.EventHandler(this.ChangeSharpness);
			// 
			// Contrast
			// 
			this.Contrast.Location = new System.Drawing.Point(144, 248);
			this.Contrast.Maximum = 100;
			this.Contrast.Name = "Contrast";
			this.Contrast.Size = new System.Drawing.Size(128, 45);
			this.Contrast.TabIndex = 7;
			this.Contrast.TickStyle = System.Windows.Forms.TickStyle.None;
			this.Contrast.Scroll += new System.EventHandler(this.ChangeContrast);
			// 
			// Brightness
			// 
			this.Brightness.Location = new System.Drawing.Point(16, 248);
			this.Brightness.Maximum = 100;
			this.Brightness.Name = "Brightness";
			this.Brightness.Size = new System.Drawing.Size(128, 45);
			this.Brightness.TabIndex = 6;
			this.Brightness.TickStyle = System.Windows.Forms.TickStyle.None;
			this.Brightness.Scroll += new System.EventHandler(this.ChangeBrightness);
			// 
			// label7
			// 
			this.label7.Location = new System.Drawing.Point(24, 224);
			this.label7.Name = "label7";
			this.label7.Size = new System.Drawing.Size(112, 23);
			this.label7.TabIndex = 9;
			this.label7.Text = "Brightness";
			this.label7.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// label8
			// 
			this.label8.Location = new System.Drawing.Point(152, 224);
			this.label8.Name = "label8";
			this.label8.Size = new System.Drawing.Size(112, 23);
			this.label8.TabIndex = 10;
			this.label8.Text = "Contrast";
			this.label8.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// label9
			// 
			this.label9.Location = new System.Drawing.Point(288, 224);
			this.label9.Name = "label9";
			this.label9.Size = new System.Drawing.Size(112, 23);
			this.label9.TabIndex = 11;
			this.label9.Text = "Sharpness";
			this.label9.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// ImageVideoForm
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(426, 312);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.label9,
																		  this.label8,
																		  this.label7,
																		  this.Sharpness,
																		  this.Contrast,
																		  this.Brightness,
																		  this.groupBox2,
																		  this.groupBox1});
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
			this.Name = "ImageVideoForm";
			this.Text = "Image/Video Controls";
			this.Closed += new System.EventHandler(this.OnClosed);
			this.groupBox1.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)(this.BlueBlack)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.GreenBlack)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.RedBlack)).EndInit();
			this.groupBox2.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)(this.BlueGain)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.GreenGain)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.RedGain)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.Sharpness)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.Contrast)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.Brightness)).EndInit();
			this.ResumeLayout(false);

		}
		#endregion

		private void OnClosed(object sender, System.EventArgs e)
		{
			Connection.OnBlueVideoBlackLevelChanged -= new AVConnection.VariableChangedHandler(BlueBlackSink);
			Connection.OnBlueVideoGainChanged -= new AVConnection.VariableChangedHandler(BlueGainSink);
			Connection.OnGreenVideoBlackLevelChanged -= new AVConnection.VariableChangedHandler(GreenBlackSink);
			Connection.OnGreenVideoGainChanged -= new AVConnection.VariableChangedHandler(GreenGainSink);
			Connection.OnRedVideoBlackLevelChanged -= new AVConnection.VariableChangedHandler(RedBlackSink);
			Connection.OnRedVideoGainChanged -= new AVConnection.VariableChangedHandler(RedGainSink);
			Connection.OnSharpnessChanged -= new AVConnection.VariableChangedHandler(SharpnessSink);
			Connection.OnBrightnessChanged -= new AVConnection.VariableChangedHandler(BrightnessSink);
			Connection.OnContrastChanged -= new AVConnection.VariableChangedHandler(ContrastSink);


			Parent.RemoveMe(this);
		}

		private void ChangeRedBlack(object sender, System.EventArgs e)
		{
			if(Connection.RedVideoBlackLevel!= (ushort)RedBlack.Value)
				Connection.RedVideoBlackLevel = (ushort)RedBlack.Value;
		}

		private void ChangeGreenBlack(object sender, System.EventArgs e)
		{
			if(Connection.GreenVideoBlackLevel!=(ushort)GreenBlack.Value)
				Connection.GreenVideoBlackLevel = (ushort)GreenBlack.Value;
		}

		private void ChangeBlueBlack(object sender, System.EventArgs e)
		{
			if(Connection.BlueVideoBlackLevel!=(ushort)BlueBlack.Value)
				Connection.BlueVideoBlackLevel = (ushort)BlueBlack.Value;
		}

		private void ChangeRedGain(object sender, System.EventArgs e)
		{
			if(Connection.RedVideoGain!=(ushort)RedGain.Value)
				Connection.RedVideoGain = (ushort)RedGain.Value;
		}

		private void ChangeGreenGain(object sender, System.EventArgs e)
		{
			if(Connection.GreenVideoGain!=(ushort)GreenGain.Value)
				Connection.GreenVideoGain = (ushort)GreenGain.Value;
		}

		private void ChangeBlueGain(object sender, System.EventArgs e)
		{
			if(Connection.BlueVideoGain!=(ushort)BlueGain.Value)
				Connection.BlueVideoGain = (ushort)BlueGain.Value;
		}

		private void ChangeBrightness(object sender, System.EventArgs e)
		{
			if(Connection.Brightness!=(ushort)Brightness.Value)
				Connection.Brightness = (ushort)Brightness.Value;
		}

		private void ChangeContrast(object sender, System.EventArgs e)
		{
			if(Connection.Contrast!=(ushort)Contrast.Value)
				Connection.Contrast = (ushort)Contrast.Value;
		}

		private void ChangeSharpness(object sender, System.EventArgs e)
		{
			if(Connection.Sharpness!=(ushort)Sharpness.Value)
				Connection.Sharpness = (ushort)Sharpness.Value;
		}
	}
}
