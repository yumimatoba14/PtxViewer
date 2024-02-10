
namespace PtxViewer
{
    partial class Form1
    {
        /// <summary>
        /// 必要なデザイナー変数です。
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 使用中のリソースをすべてクリーンアップします。
        /// </summary>
        /// <param name="disposing">マネージド リソースを破棄する場合は true を指定し、その他の場合は false を指定します。</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows フォーム デザイナーで生成されたコード

        /// <summary>
        /// デザイナー サポートに必要なメソッドです。このメソッドの内容を
        /// コード エディターで変更しないでください。
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.mainMenuStrip = new System.Windows.Forms.MenuStrip();
            this.fileMenu = new System.Windows.Forms.ToolStripMenuItem();
            this.openMenu = new System.Windows.Forms.ToolStripMenuItem();
            this.exitMenu = new System.Windows.Forms.ToolStripMenuItem();
            this.mainView = new Tngn.FormView();
            this.testMenu = new System.Windows.Forms.ToolStripMenuItem();
            this.drawWithScannerPosMenu = new System.Windows.Forms.ToolStripMenuItem();
            this.enablePickMenu = new System.Windows.Forms.ToolStripMenuItem();
            this.viewTimer = new System.Windows.Forms.Timer(this.components);
            this.saveViewImageMenu = new System.Windows.Forms.ToolStripMenuItem();
            this.lengthDimensionMenu = new System.Windows.Forms.ToolStripMenuItem();
            this.viewMenu = new System.Windows.Forms.ToolStripMenuItem();
            this.orthographicViewMenu = new System.Windows.Forms.ToolStripMenuItem();
            this.mainMenuStrip.SuspendLayout();
            this.SuspendLayout();
            // 
            // mainMenuStrip
            // 
            this.mainMenuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileMenu,
            this.viewMenu,
            this.testMenu});
            this.mainMenuStrip.Location = new System.Drawing.Point(0, 0);
            this.mainMenuStrip.Name = "mainMenuStrip";
            this.mainMenuStrip.Size = new System.Drawing.Size(800, 24);
            this.mainMenuStrip.TabIndex = 0;
            this.mainMenuStrip.Text = "menuStrip1";
            // 
            // fileMenu
            // 
            this.fileMenu.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.openMenu,
            this.exitMenu});
            this.fileMenu.Name = "fileMenu";
            this.fileMenu.Size = new System.Drawing.Size(37, 20);
            this.fileMenu.Text = "&File";
            // 
            // openMenu
            // 
            this.openMenu.Name = "openMenu";
            this.openMenu.Size = new System.Drawing.Size(101, 22);
            this.openMenu.Text = "&open";
            this.openMenu.Click += new System.EventHandler(this.openMenu_Click);
            // 
            // exitMenu
            // 
            this.exitMenu.Name = "exitMenu";
            this.exitMenu.Size = new System.Drawing.Size(101, 22);
            this.exitMenu.Text = "&exit";
            this.exitMenu.Click += new System.EventHandler(this.exitMenu_Click);
            // 
            // mainView
            // 
            this.mainView.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.mainView.Location = new System.Drawing.Point(0, 27);
            this.mainView.Name = "mainView";
            this.mainView.Size = new System.Drawing.Size(800, 424);
            this.mainView.TabIndex = 1;
            this.mainView.Text = "formView1";
            this.mainView.Paint += new System.Windows.Forms.PaintEventHandler(this.mainView_Paint);
            this.mainView.MouseDown += new System.Windows.Forms.MouseEventHandler(this.mainView_MouseDown);
            this.mainView.MouseMove += new System.Windows.Forms.MouseEventHandler(this.mainView_MouseMove);
            this.mainView.MouseUp += new System.Windows.Forms.MouseEventHandler(this.mainView_MouseUp);
            this.mainView.MouseWheel += new System.Windows.Forms.MouseEventHandler(this.mainView_MouseWheel);
            this.mainView.Resize += new System.EventHandler(this.mainView_Resize);
            // 
            // testMenu
            // 
            this.testMenu.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.drawWithScannerPosMenu,
            this.enablePickMenu,
            this.saveViewImageMenu,
            this.lengthDimensionMenu});
            this.testMenu.Name = "testMenu";
            this.testMenu.Size = new System.Drawing.Size(39, 20);
            this.testMenu.Text = "&Test";
            // 
            // drawWithScannerPosMenu
            // 
            this.drawWithScannerPosMenu.CheckOnClick = true;
            this.drawWithScannerPosMenu.Name = "drawWithScannerPosMenu";
            this.drawWithScannerPosMenu.Size = new System.Drawing.Size(196, 22);
            this.drawWithScannerPosMenu.Text = "Draw with scanner pos.";
            this.drawWithScannerPosMenu.Click += new System.EventHandler(this.drawWithScannerPosMenu_Click);
            // 
            // enablePickMenu
            // 
            this.enablePickMenu.CheckOnClick = true;
            this.enablePickMenu.Name = "enablePickMenu";
            this.enablePickMenu.Size = new System.Drawing.Size(196, 22);
            this.enablePickMenu.Text = "Enable &pick";
            this.enablePickMenu.Click += new System.EventHandler(this.enablePickMenu_Click);
            // 
            // viewTimer
            // 
            this.viewTimer.Interval = 1;
            this.viewTimer.Tick += new System.EventHandler(this.viewTimer_Tick);
            // 
            // saveViewImageMenu
            // 
            this.saveViewImageMenu.Name = "saveViewImageMenu";
            this.saveViewImageMenu.Size = new System.Drawing.Size(196, 22);
            this.saveViewImageMenu.Text = "Save view &image...";
            this.saveViewImageMenu.ToolTipText = "Save view to image file.";
            this.saveViewImageMenu.Click += new System.EventHandler(this.saveViewImageMenu_Click);
            // 
            // lengthDimensionMenu
            // 
            this.lengthDimensionMenu.CheckOnClick = true;
            this.lengthDimensionMenu.Name = "lengthDimensionMenu";
            this.lengthDimensionMenu.Size = new System.Drawing.Size(196, 22);
            this.lengthDimensionMenu.Text = "&Measure distance";
            this.lengthDimensionMenu.Click += new System.EventHandler(this.lengthDimensionMenu_Click);
            // 
            // viewMenu
            // 
            this.viewMenu.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.orthographicViewMenu});
            this.viewMenu.Name = "viewMenu";
            this.viewMenu.Size = new System.Drawing.Size(44, 20);
            this.viewMenu.Text = "&View";
            // 
            // orthographicViewMenu
            // 
            this.orthographicViewMenu.CheckOnClick = true;
            this.orthographicViewMenu.Name = "orthographicViewMenu";
            this.orthographicViewMenu.Size = new System.Drawing.Size(180, 22);
            this.orthographicViewMenu.Text = "&Orthographic view";
            this.orthographicViewMenu.ToolTipText = "Switch perspective view and orthographic view.";
            this.orthographicViewMenu.Click += new System.EventHandler(this.orthographicViewMenu_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(800, 450);
            this.Controls.Add(this.mainView);
            this.Controls.Add(this.mainMenuStrip);
            this.MainMenuStrip = this.mainMenuStrip;
            this.Name = "Form1";
            this.Text = "Form1";
            this.mainMenuStrip.ResumeLayout(false);
            this.mainMenuStrip.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.MenuStrip mainMenuStrip;
        private System.Windows.Forms.ToolStripMenuItem fileMenu;
        private System.Windows.Forms.ToolStripMenuItem openMenu;
        private System.Windows.Forms.ToolStripMenuItem exitMenu;
        private Tngn.FormView mainView;
        private System.Windows.Forms.Timer viewTimer;
        private System.Windows.Forms.ToolStripMenuItem testMenu;
        private System.Windows.Forms.ToolStripMenuItem enablePickMenu;
        private System.Windows.Forms.ToolStripMenuItem drawWithScannerPosMenu;
        private System.Windows.Forms.ToolStripMenuItem saveViewImageMenu;
        private System.Windows.Forms.ToolStripMenuItem lengthDimensionMenu;
        private System.Windows.Forms.ToolStripMenuItem viewMenu;
        private System.Windows.Forms.ToolStripMenuItem orthographicViewMenu;
    }
}

