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
        class LocalCursorManager : IDisposable
        {
            public LocalCursorManager(Cursor newCursor)
            {
                orgCursor = Cursor.Current;
                Cursor.Current = newCursor;
            }

            protected virtual void Dispose(bool disposing)
            {
                if (!disposedValue)
                {
                    if (disposing)
                    {
                        // TODO: マネージド状態を破棄します (マネージド オブジェクト)
                        Cursor.Current = orgCursor;
                    }

                    // TODO: アンマネージド リソース (アンマネージド オブジェクト) を解放し、ファイナライザーをオーバーライドします
                    // TODO: 大きなフィールドを null に設定します
                    disposedValue = true;
                }
            }

            // // TODO: 'Dispose(bool disposing)' にアンマネージド リソースを解放するコードが含まれる場合にのみ、ファイナライザーをオーバーライドします
            // ~LocalCursorManager()
            // {
            //     // このコードを変更しないでください。クリーンアップ コードを 'Dispose(bool disposing)' メソッドに記述します
            //     Dispose(disposing: false);
            // }

            public void Dispose()
            {
                // このコードを変更しないでください。クリーンアップ コードを 'Dispose(bool disposing)' メソッドに記述します
                Dispose(disposing: true);
                GC.SuppressFinalize(this);
            }

            private Cursor orgCursor = null;
            private bool disposedValue = false;
        }

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
            if (viewModel == null)
            {
                return;
            }

            using (OpenFileDialog openFileDialog = new OpenFileDialog())
            {
                openFileDialog.Filter = "PTX File(*.ptx)|*.ptx|All file(*.*)|*.*";
                openFileDialog.Multiselect = true;
                if (openFileDialog.ShowDialog() == DialogResult.OK)
                {
                    using (var localCursor = new LocalCursorManager(Cursors.WaitCursor))
                    {
                        if (openFileDialog.Multiselect)
                        {
                            foreach (var filePath in openFileDialog.FileNames)
                            {
                                bool isOk = viewModel.OpenPtxFile(filePath);
                                if (!isOk)
                                {
                                    MessageBox.Show("Failed to open file:" + filePath, "File open", MessageBoxButtons.OK, MessageBoxIcon.Error);
                                }
                            }

                        }
                        else
                        {
                            bool isOk = viewModel.OpenPtxFile(openFileDialog.FileName);
                            if (!isOk)
                            {
                                MessageBox.Show("Failed to open file:" + openFileDialog.FileName, "File open", MessageBoxButtons.OK, MessageBoxIcon.Error);
                            }
                        }
                    }
                    drawWithScannerPosMenu.Checked = viewModel.IsDrawWithScannerPosition();
                    mainView.Invalidate();
                }
            }
        }

        private bool isProgressiveViewMode = true;
        private bool afterTimer = false;
        private Tngn.ViewModel viewModel = null;

        private void mainView_Paint(object sender, PaintEventArgs e)
        {
            if (viewModel == null)
            {
                viewModel = new Tngn.ViewModel(this.components, mainView.Handle);
            }
            viewModel.SetProgressiveViewMode(isProgressiveViewMode, afterTimer);
            viewModel.Draw();
            if (isProgressiveViewMode && viewModel.IsViewContentUpdated())
            {
                viewTimer.Enabled = true;
            }
            else
            {
                viewTimer.Enabled = false;
            }
            afterTimer = false;
        }

        private void mainView_Resize(object sender, EventArgs e)
        {
            mainView.Invalidate();
            if (viewModel != null)
            {
                viewModel.OnSize(mainView.Size);
            }
        }

        private void mainView_MouseWheel(object sender, MouseEventArgs e)
        {
            if (viewModel != null)
            {
                viewModel.OnMouseWheel(e);
                if (viewModel.IsNeedDraw())
                {
                    mainView.Invalidate();
                }
            }
        }

        private void mainView_MouseDown(object sender, MouseEventArgs e)
        {
            if (viewModel != null)
            {
                viewModel.OnMouseButtonDown(e);
                if (viewModel.IsNeedDraw())
                {
                    mainView.Invalidate();
                }
            }
        }

        private void mainView_MouseUp(object sender, MouseEventArgs e)
        {
            if (viewModel != null)
            {
                viewModel.OnMouseButtonUp(e);
                if (viewModel.IsNeedDraw())
                {
                    mainView.Invalidate();
                }
            }
        }

        private void mainView_MouseMove(object sender, MouseEventArgs e)
        {
            if (viewModel != null)
            {
                viewModel.OnMouseMove(e);
                if (viewModel.IsNeedDraw())
                {
                    mainView.Invalidate();
                }
            }
        }

        private void viewTimer_Tick(object sender, EventArgs e)
        {
            afterTimer = true;
            mainView.Invalidate();
        }

        private void enablePickMenu_Click(object sender, EventArgs e)
        {
            if (enablePickMenu.Checked)
            {
                viewModel.SetViewEventListener(new PickMode(() => enablePickMenu.Checked = false));
            } else
            {
                viewModel.SetViewEventListener(null);
            }
        }

        private void drawWithScannerPosMenu_Click(object sender, EventArgs e)
        {
            if (viewModel != null)
            {
                if (drawWithScannerPosMenu.Checked != viewModel.IsDrawWithScannerPosition())
                {
                    viewModel.SetDrawWithScannerPosition(drawWithScannerPosMenu.Checked);
                    if (drawWithScannerPosMenu.Checked != viewModel.IsDrawWithScannerPosition())
                    {
                        drawWithScannerPosMenu.Checked = viewModel.IsDrawWithScannerPosition();
                    }
                }
                mainView.Invalidate();
            }
        }

        private void saveViewImageMenu_Click(object sender, EventArgs e)
        {
            if (viewModel == null)
            {
                return;
            }
            using (SaveFileDialog fileDialog = new SaveFileDialog())
            {
                fileDialog.Filter = "PNG File(*.png)|*.png|"
                    + "JPEG file(*.jpg, *.jpeg)|*.jpg;*.jpeg|"
                    + "Bitmap file(*.bmp)|*.bmp|"
                    + "All file(*.*)|*.*";
                if (fileDialog.ShowDialog() == DialogResult.OK)
                {
                    try
                    {
                        viewModel.SaveViewToFile(fileDialog.FileName);
                    }
                    catch (Exception)
                    {
                        MessageBox.Show(
                            "Failed to save image file : " + fileDialog.FileName,
                            "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    }
                }
            }
        }
    }
}
