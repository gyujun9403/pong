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
            this.notifyDataList = new System.Windows.Forms.ListBox();
            this.WinButton = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // notifyDataList
            // 
            this.notifyDataList.FormattingEnabled = true;
            this.notifyDataList.ItemHeight = 12;
            this.notifyDataList.Location = new System.Drawing.Point(83, 30);
            this.notifyDataList.Name = "notifyDataList";
            this.notifyDataList.Size = new System.Drawing.Size(120, 28);
            this.notifyDataList.TabIndex = 0;
            this.notifyDataList.SelectedIndexChanged += new System.EventHandler(this.notifyDataList_selectDataexcange);
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
            // GameForm
            // 
            this.ClientSize = new System.Drawing.Size(284, 261);
            this.Controls.Add(this.WinButton);
            this.Controls.Add(this.notifyDataList);
            this.Name = "GameForm";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.GameForm_FormClosing);
            this.Load += new System.EventHandler(this.GameForm_Load);
            this.ResumeLayout(false);

        }

        private System.Windows.Forms.ListBox notifyDataList;
        private System.Windows.Forms.Button WinButton;
    }
}