namespace csharp_test_client
{
    partial class GameForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        private void InitializeComponent()
        {
            this.WinButton = new System.Windows.Forms.Button();
            this.ServerText = new System.Windows.Forms.TextBox();
            this.SuspendLayout();
            // 
            // WinButton
            // 
            this.WinButton.Location = new System.Drawing.Point(62, 100);
            this.WinButton.Name = "WinButton";
            this.WinButton.Size = new System.Drawing.Size(159, 105);
            this.WinButton.TabIndex = 1;
            this.WinButton.Text = "WinButton";
            this.WinButton.UseVisualStyleBackColor = true;
            this.WinButton.Click += new System.EventHandler(this.WinButton_Click);
            // 
            // ServerText
            // 
            this.ServerText.Location = new System.Drawing.Point(76, 24);
            this.ServerText.Name = "ServerText";
            this.ServerText.Size = new System.Drawing.Size(134, 25);
            this.ServerText.TabIndex = 2;
            // 
            // GameForm
            // 
            this.ClientSize = new System.Drawing.Size(284, 261);
            this.Controls.Add(this.ServerText);
            this.Controls.Add(this.WinButton);
            this.Name = "GameForm";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.GameForm_FormClosing);
            this.Load += new System.EventHandler(this.GameForm_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }
        private System.Windows.Forms.Button WinButton;
        private System.Windows.Forms.TextBox ServerText;
    }
}