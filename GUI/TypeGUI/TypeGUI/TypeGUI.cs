using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

using System.IO.Ports;
using System.Collections;


namespace TypeGUI
{
    public partial class TypeGUI : Form
    {
        private NotifyIcon notifyIcon;
        private ArrayList serialItems; 

        private string comPort;
        private bool exiting = false;

        private static string VERSION = "1.0";

        public TypeGUI()
        {
            InitializeComponent();
            lblAbout.Text += VERSION;

            notifyIcon = new NotifyIcon();
            notifyIcon.Icon = Properties.Resources.TrayIcon;
            notifyIcon.Text = "TypeGUI";
            notifyIcon.DoubleClick += new EventHandler(DoSendClipboard);

            ToolStripMenuItem sendClipboardMenu = new ToolStripMenuItem("Send clipboard");
            sendClipboardMenu.Font = new Font(sendClipboardMenu.Font, sendClipboardMenu.Font.Style | FontStyle.Bold);
            sendClipboardMenu.Click += new EventHandler(DoSendClipboard);

            ToolStripMenuItem comPortMenu = new ToolStripMenuItem("COM Port");
            string[] serialPorts = SerialPort.GetPortNames();
            serialItems = new ArrayList();
            if (serialPorts.Length == 0)
            {
                ToolStripMenuItem noPortsMenu = new ToolStripMenuItem("(no port available)");
                noPortsMenu.Enabled = false;
                comPortMenu.DropDown.Items.Add(noPortsMenu);
                comPort = null;
            }
            else for(int i = 0; i < serialPorts.Length; i++)
            {
                string actualComPort = serialPorts[i];
                ToolStripMenuItem comPortItem = new ToolStripMenuItem(actualComPort);
                comPortItem.Click += new EventHandler(DoClickComPort);
                comPortMenu.DropDown.Items.Add(comPortItem);
                serialItems.Add(comPortItem);

                if (i == 0)
                {
                    comPortItem.Checked = true;
                    comPort = actualComPort;
                }
            }

            ContextMenuStrip contextMenu = new ContextMenuStrip();
            contextMenu.Items.Add(sendClipboardMenu);
            contextMenu.Items.Add(comPortMenu);
            contextMenu.Items.Add("-");
            contextMenu.Items.Add("About", null, DoAbout);
            contextMenu.Items.Add("Exit", null, DoExit);

            notifyIcon.ContextMenuStrip = contextMenu;
            notifyIcon.Visible = true;
        }

        protected override void OnLoad(EventArgs e)
        {
            Visible = false;
            ShowInTaskbar = false;
            base.OnLoad(e);
        }

        private void DoSendClipboard(object sender, EventArgs e)
        {
            if (!Clipboard.ContainsText()) MessageBox.Show("Clipboard contains no text", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            else
            {
                SerialPort serialPort = new SerialPort(comPort, 9600, Parity.None, 8, StopBits.One);
                serialPort.ReadTimeout = 1000;
                try
                {
                    serialPort.Open();
                    serialPort.Write(Clipboard.GetText());
                    serialPort.Write("\r");
                    string response = serialPort.ReadLine();
                    response = serialPort.ReadLine();
                    if (!response.Equals("OK!\r")) throw new Exception("Incorrect answer from Type4me");
                    serialPort.Close();

                    notifyIcon.ShowBalloonTip(5000, "", "Clipboard text sent to Type4me", ToolTipIcon.Info);
                }
                catch (Exception ex)
                {
                    MessageBox.Show("Unable to send data: " + ex.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    if (serialPort.IsOpen) serialPort.Close();
                }
            }
        }

        private void DoClickComPort(object sender, EventArgs e)
        {            
            foreach (ToolStripMenuItem item in serialItems) item.Checked = false;

            ToolStripMenuItem menuItem = (ToolStripMenuItem)sender;
            comPort = menuItem.Text;
            menuItem.Checked = true;
        }

        private void DoAbout(object sender, EventArgs e)
        {
            Visible = true;
            ShowInTaskbar = true;
        }

        private void DoExit(object sender, EventArgs e) 
        {
            exiting = true;
            Application.Exit();
        }

        private void TypeGUI_FormClosing(object sender, FormClosingEventArgs e)
        {
            Visible = false;
            ShowInTaskbar = false;

            if(!exiting) e.Cancel = true;
            base.OnClosing(e);
        }
    }
}
