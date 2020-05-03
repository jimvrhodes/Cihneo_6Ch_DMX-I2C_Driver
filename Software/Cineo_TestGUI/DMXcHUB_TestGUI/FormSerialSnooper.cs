using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO.Ports;
using System.Threading;

namespace DMXcHUB_TestGUI
{
    public partial class FormSerialSnooper : Form
    {
        SerialPort myPort;
        static ListBox log;
        static string message = null;

        public FormSerialSnooper(SerialPort port)
        {
            InitializeComponent();

            myPort = port;
            myPort.DataReceived += new SerialDataReceivedEventHandler(DataReceivedHandler);

            log = listBoxSerialLog;

            Thread thread = new Thread(new ThreadStart(Thread_SerialChecker));
            thread.IsBackground = true;
            thread.Name = "Serial Stream Thread";

            thread.Start();
        }

        private void buttonClose_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private static void DataReceivedHandler(object sender, SerialDataReceivedEventArgs e)
        {
            DateTime currentime;

            SerialPort sp = (SerialPort)sender;
            string indata = sp.ReadExisting();

            currentime = DateTime.Now;
            message = (String.Format("{0:00}", currentime.Hour) + ":" + String.Format("{0:00}", currentime.Minute) + ":" + String.Format("{0:00}", currentime.Second) + "  " + indata);
        }

        private static void Thread_SerialChecker()
        {
            while (true)
            {
                if (!String.IsNullOrEmpty(message))
                {
                    log.Items.Add(message);
                    message = null;
                }
                Thread.Sleep(10);
            }
        }
    }
}
