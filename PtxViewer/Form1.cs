using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace PtxViewer
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private void exitMenu_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void openMenu_Click(object sender, EventArgs e)
        {
            using (OpenFileDialog openFileDialog = new OpenFileDialog())
            {
                openFileDialog.Filter = "PTX File(*.ptx)|*.ptx|All file(*.*)|*.*";
                if (openFileDialog.ShowDialog() == DialogResult.OK)
                {
                    MessageBox.Show(openFileDialog.FileName, "開発中", MessageBoxButtons.OK, MessageBoxIcon.Information);
                }
            }
        }

        private void mainView_Paint(object sender, PaintEventArgs e)
        {
            using (var impl = new Tngn.Class1())
            {
                impl.DrawToWindow(mainView.Handle);
            }
            var panelSize = mainView.Size;
            int margin = 2;
            e.Graphics.DrawRectangle(System.Drawing.Pens.Red, margin, margin, panelSize.Width - margin * 2 - 1, panelSize.Height - margin * 2 - 1);
        }
    }
}
