using System;
//using System.Collections.Generic;
//using System.ComponentModel;
//using System.Data;
using System.Drawing;
//using System.Linq;
//using System.Text;
//using System.Threading.Tasks;
using System.Windows.Forms;
//using System.IO;
using System.IO.Ports;
//using System.Text.RegularExpressions;
//using CyUSB;
//using Bootloader;
//using System.Threading.Tasks;
using System.Diagnostics;


//
//
// this supports version 1.00 of the I2C Test cHUB firmware
//
namespace DMXcHUB_TestGUI
{
    public partial class FormMain : Form
    {
        public unsafe struct structInfo
        {
            public bool bACKOnOff;
            public bool bAutoSaveOnOff;
            //
            public ushort uiAuditNumber;
        };

        // this is for our serial use
        public byte[] byArray = new byte[600];
        public int z;
        public byte byAuditCode;

        public bool bWeGotData = true;
        public bool bTickAlive = false;

        // use this to force the next serial command output
        public static int byForceSerial = 0;
        public static int byReadSerial = 0;
        public static byte byI2CAddress = 22;

        // constants
        public const byte PINGRESPONSE = 0x55;
        //        public const byte ACKGOODRESPONSE = 0xAA;
        public const byte ACKGOODRESPONSE = 0x00;
        public const byte ACKBADRESPONSE = 0xBB;
        public const byte ACKRDMALREADYSELECTEDRESPONSE = 0xBC;
        public const byte ACKBADCHECKSUMRESPONSE = 0xBD;
        //
        // packet offsets to where the data is
        public const byte A_PACKETLENGTH = 7;
        public const byte B_PACKETLENGTH = 8;
        public const byte C_PACKETLENGTH = 9;
        public const byte D_PACKETLENGTH = 8;
        public const byte E_PACKETLENGTH = 10;
        // read and write commands
        public const byte READ = 0;
        public const byte WRITE = 1;
        // command string lengths
        public const byte ACKLENGTH = A_PACKETLENGTH;
        public const byte PINGLENGTH = A_PACKETLENGTH;
        public const byte AUDITCODELENGTH = B_PACKETLENGTH;
        public const byte READCHUBVERSIONLENGTH = E_PACKETLENGTH;
        public const byte SENDTEMPERATURELENGTH = D_PACKETLENGTH;
        public const byte AUTOSAVELENGTH = A_PACKETLENGTH;
        public const byte RESETLENGTH = A_PACKETLENGTH;
        public const byte SLIDERLENGTH = D_PACKETLENGTH;
        public const byte TEMPLENGTH = A_PACKETLENGTH;
        public const byte VOLTSLENGTH = A_PACKETLENGTH;
        public const byte READDRVVERSIONLENGTH = E_PACKETLENGTH;
        public const byte SETI2CADDRESSLENGTH = A_PACKETLENGTH;

        public FormMain()
        {
            InitializeComponent();

            // see what COM ports are on the system
            // Get a list of serial port names.
            string[] ports = SerialPort.GetPortNames();

            // Display each port name to the console.
            foreach (string port in ports)
                comboBoxPortNumber.Items.Add(port);
            if (comboBoxPortNumber.Items.Count != 0)
            {
                comboBoxPortNumber.SelectedIndex = 0;
                UpdateLogMessage("COM Ports scanned...select a port from the drop down list...");
            }
            else
            {
                comboBoxPortNumber.Text = "No ports!";
                UpdateLogMessage("COM Ports scanned...no ports found!");
            }

            labelStatusMessage.Text = "Ports scanned...select a COM port to connect to...";

            // show what the user can do here
            comboBoxCommand.Items.Clear();
            comboBoxCommand.Items.Add("Ping (0x10)");                                                                               // done
            comboBoxCommand.Items.Add("ACK command (0x11)");                                                                        // done
            comboBoxCommand.Items.Add("AutoSave ON/OFF (0x12)");                                                                    // done
            comboBoxCommand.Items.Add("Read Driver Temp (0x2A)");                                                                   // done
            comboBoxCommand.Items.Add("Read Driver Input Voltage (0x2B)");                                                          // done
            comboBoxCommand.Items.Add("Read Driver version (XXYYZZ) (0x2C)");                                                       // done
            comboBoxCommand.Items.Add("Set Driver I2C Address (byte val) (0x2D)");                                                  // done
            comboBoxCommand.Items.Add("Read cHUB version (XXYYZZ) (0x30)");                                                         // done
            comboBoxCommand.Items.Add("Read cHUB NTC ADC count (0-1023) (0x31)");                                                   // done
            comboBoxCommand.Items.Add("Set Audit Code (0xAA)");                                                                     // done
            comboBoxCommand.Items.Add("Reset DMXcHUB (0xBB)");                                                                      // done
            comboBoxCommand.SelectedIndex = 0;

            // enable these as we read them in after connection
            trackBarCH1.Enabled = false;
            trackBarCH2.Enabled = false;
            trackBarCH3.Enabled = false;
            trackBarCH4.Enabled = false;
            trackBarCH5.Enabled = false;
            trackBarCH6.Enabled = false;

            buttonBootload.Enabled = false;
            buttonHammer.Enabled = false;
            buttonBootload.Visible = false;
            buttonHammer.Visible = false;
        }

        private void buttonOpenUSBPort_Click(object sender, EventArgs e)
        {
            if (buttonOpenUSBPort.Text == "Open USB")
            {
                try
                {
                    SerialPort.PortName = (string)comboBoxPortNumber.SelectedItem;
                    SerialPort.Open();
                    if (SerialPort.IsOpen)
                        labelStatusMessage.Text = comboBoxPortNumber.SelectedItem + " opened...ready to go!";

                    buttonOpenUSBPort.BackColor = Color.Lime;
                    buttonOpenUSBPort.Text = "Close USB";

                    // now allow user to start using app
                    comboBoxCommand.Enabled = true;
                    comboBoxCommand.Text = "Make command selection here first...";

                    buttonBootload.Enabled = false;
                    buttonHammer.Enabled = false;
                    buttonBootload.Visible = false;
                    buttonHammer.Visible = false;

                    buttonAllOff.Enabled = true;
                    buttonComTest.Enabled = true;
                    buttonSend.Enabled = true;

                    textBoxI2CAddress.Enabled = false;
                    textBoxI2CAddress.Text = "22";

                    byAuditCode = 0;

                    // set to read as default
                    comboBoxReadWrite.Items.Clear();
                    comboBoxReadWrite.Items.Add("Write");
                    comboBoxReadWrite.Items.Add("Read");
                    comboBoxReadWrite.Enabled = true;
                    comboBoxReadWrite.SelectedIndex = 1;

                    // to get the initial values
                    timerGetInitialSettings.Enabled = true;
                    bWeGotData = true;
                    bTickAlive = false;

                    byForceSerial = byReadSerial = (byte)CONSOLECOMMANDS.PING;
                    timerSerialPush.Enabled = true;
                }
                catch
                {
                    labelStatusMessage.Text = "Error opening " + comboBoxPortNumber.SelectedItem + ".  Please try another port or check connection";
                    buttonOpenUSBPort.BackColor = Color.Red;
                }
            }
            else
            {
                try
                {
                    SerialPort.Close();
                    if (!SerialPort.IsOpen)
                        labelStatusMessage.Text = comboBoxPortNumber.SelectedItem + " closed...be sure to Open port again!";

                    buttonOpenUSBPort.BackColor = Color.Yellow;
                    buttonOpenUSBPort.Text = "Open USB";
                    buttonComTest.BackColor = Color.LightGray;

                    // now take control away from user
                    comboBoxCommand.Enabled = false;
                    comboBoxCommand.Text = "Select command here...";
                    comboBoxParameters.Enabled = false;
                    textBoxCommandString.Enabled = false;
                    comboBoxReadWrite.Enabled = false;

                    buttonSend.Enabled = false;
                    textBoxI2CAddress.Enabled = false;
                    textBoxI2CAddress.Text = "  ";

                    buttonBootload.Enabled = false;
                    buttonHammer.Enabled = false;

                    buttonAllOff.Enabled = false;
                    buttonComTest.Enabled = false;

                    trackBarCH1.Enabled = false;
                    trackBarCH1.Value = 0;
                    CH1UpDown.Value = 0;

                    trackBarCH2.Enabled = false;
                    trackBarCH2.Value = 0;
                    CH2UpDown.Value = 0;

                    trackBarCH3.Enabled = false;
                    trackBarCH3.Value = 0;
                    CH3UpDown.Value = 0;

                    trackBarCH4.Enabled = false;
                    trackBarCH4.Value = 0;
                    CH4UpDown.Value = 0;

                    trackBarCH5.Enabled = false;
                    trackBarCH5.Value = 0;
                    CH5UpDown.Value = 0;

                    trackBarCH6.Enabled = false;
                    trackBarCH6.Value = 0;
                    CH6UpDown.Value = 0;

                    // to get the initial values
                    timerGetInitialSettings.Enabled = false;
                    bWeGotData = false;
                    bTickAlive = false;
                }
                catch
                {
                    labelStatusMessage.Text = "Error closing " + comboBoxPortNumber.SelectedItem + ".  Was it even open?";
                }
            }
        }

        private void buttonSend_Click(object sender, EventArgs e)
        {
            // this is the main activity switch
            switch (comboBoxCommand.SelectedIndex)
            {
                // this is the PING command
                case (byte)SWITCHINDEX.PING:

                    timerSerialPush.Enabled = true;
                    byForceSerial = byReadSerial = (byte)CONSOLECOMMANDS.PING;

                    labelStatusMessage.Text = "PING command selected...";
                    break;

                // ACK On/Off command
                case (byte)SWITCHINDEX.ACK:

                    timerSerialPush.Enabled = true;
                    byForceSerial = byReadSerial = (byte)CONSOLECOMMANDS.ACK;

                    labelStatusMessage.Text = "ACK On/Off command selected...";
                    break;

                // Autosave On/Off command
                case (byte)SWITCHINDEX.AUTOSAVE:

                    timerSerialPush.Enabled = true;
                    byForceSerial = byReadSerial = (byte)CONSOLECOMMANDS.AUTOSAVE;

                    labelStatusMessage.Text = "Autosave On/Off command selected...";
                    break;

                // Read cHUB version
                case (byte)SWITCHINDEX.READCHUBVERSION:

                    timerSerialPush.Enabled = true;
                    byForceSerial = byReadSerial = (byte)CONSOLECOMMANDS.READCHUBVERSION;

                    labelStatusMessage.Text = "Reading cHUB version command selected...";
                    break;
                    
                    // Read cHUB temp
                case (byte)SWITCHINDEX.SENDTEMPERATURE:

                    timerSerialPush.Enabled = true;
                    byForceSerial = byReadSerial = (byte)CONSOLECOMMANDS.SENDTEMPERATURE;

                    labelStatusMessage.Text = "Reading cHUB temperature command selected...";
                    break;

                // Set Audit code command
                case (byte)SWITCHINDEX.SETAUDITCODE:

                    timerSerialPush.Enabled = true;
                    byForceSerial = byReadSerial = (byte)CONSOLECOMMANDS.SETAUDITCODE;

                    labelStatusMessage.Text = "Set Audit Code command selected...";
                    break;

                // Set get Driver Temp command
                case (byte)SWITCHINDEX.TEMP:

                    timerSerialPush.Enabled = true;
                    byForceSerial = byReadSerial = (byte)CONSOLECOMMANDS.TEMP;

                    labelStatusMessage.Text = "Set Get Driver Temp code command selected...";
                    break;

                // Set get Driver Volts command
                case (byte)SWITCHINDEX.VOLTS:

                    timerSerialPush.Enabled = true;
                    byForceSerial = byReadSerial = (byte)CONSOLECOMMANDS.VOLTS;

                    labelStatusMessage.Text = "Set Get Driver Temp code command selected...";
                    break;

                // Read Driver version
                case (byte)SWITCHINDEX.READDRVVERSION:

                    timerSerialPush.Enabled = true;
                    byForceSerial = byReadSerial = (byte)CONSOLECOMMANDS.READDRVVERSION;

                    labelStatusMessage.Text = "Reading Driver version command selected...";
                    break;

                // Set I2C address
                case (byte)SWITCHINDEX.SETI2CADDRESS:

                    timerSerialPush.Enabled = true;
                    byForceSerial = byReadSerial = (byte)CONSOLECOMMANDS.SETI2CADDRESS;

                    labelStatusMessage.Text = "Setting Driver I2C address command selected...";
                    break;

                // Set Reset command
                case (byte)SWITCHINDEX.RESET:

                    timerSerialPush.Enabled = true;
                    byForceSerial = byReadSerial = (byte)CONSOLECOMMANDS.RESET;

                    labelStatusMessage.Text = "Set Reset selected...";
                    break;


                default:
                    break;
            }
        }

        private void comboBoxCommand_SelectedIndexChanged(object sender, EventArgs e)
        {
            // clean em up here, we enable them if needed in the specific case
            comboBoxParameters.Enabled = false;
            comboBoxParameters.Items.Clear();
            comboBoxParameters.Text = "";
            textBoxCommandString.Enabled = false;
            textBoxCommandString.Text = "";
            comboBoxReadWrite.Enabled = false;
            comboBoxReadWrite.Items.Clear();
            comboBoxReadWrite.Text = "";
            textBoxCommandString.Enabled = false;

            // this is the main activity switch
            switch (comboBoxCommand.SelectedIndex)
            {
                // ping handler
                case (byte)SWITCHINDEX.PING:
                    {
                        // set to read as default
                        comboBoxReadWrite.Items.Clear();
                        comboBoxReadWrite.Items.Add("Write");
                        comboBoxReadWrite.Enabled = true;
                        comboBoxReadWrite.SelectedIndex = 0;
                    }
                    break;

                // ACK On/Off command
                case (byte)SWITCHINDEX.ACK:
                    {
                        comboBoxParameters.Enabled = true;
                        comboBoxParameters.Items.Clear();
                        comboBoxParameters.Items.Add("ACK OFF");
                        comboBoxParameters.Items.Add("ACK ON");
                        comboBoxParameters.SelectedIndex = 1;

                        // set to read as default
                        comboBoxReadWrite.Items.Clear();
                        comboBoxReadWrite.Items.Add("Write");
                        comboBoxReadWrite.Items.Add("Read");
                        comboBoxReadWrite.Enabled = true;
                        comboBoxReadWrite.SelectedIndex = 1;
                    }
                    break;

                // Autosave On/Off command
                case (byte)SWITCHINDEX.AUTOSAVE:
                    {
                        comboBoxParameters.Enabled = true;
                        comboBoxParameters.Items.Clear();
                        comboBoxParameters.Items.Add("AutoSave Off");
                        comboBoxParameters.Items.Add("AutoSave On");
                        comboBoxParameters.Items.Add("Save Now!");
                        comboBoxParameters.SelectedIndex = 1;

                        // set to Write as default
                        comboBoxReadWrite.Items.Clear();
                        comboBoxReadWrite.Items.Add("Write");
                        comboBoxReadWrite.Items.Add("Read");
                        comboBoxReadWrite.Enabled = true;
                        comboBoxReadWrite.SelectedIndex = 1;
                    }
                    break;

                case (byte)SWITCHINDEX.READCHUBVERSION:
                    {
                        // show version
                        textBoxCommandString.Enabled = true;
                        textBoxCommandString.Text = "00.0000.00";

                        // set to read as default
                        comboBoxReadWrite.Items.Clear();
                        comboBoxReadWrite.Items.Add("Write");
                        comboBoxReadWrite.Items.Add("Read");
                        comboBoxReadWrite.Enabled = false;
                        comboBoxReadWrite.SelectedIndex = 1;
                    }
                    break;

                case (byte)SWITCHINDEX.SENDTEMPERATURE:
                    {
                        // show version
                        textBoxCommandString.Enabled = true;
                        textBoxCommandString.Text = "0000";

                        // set to read as default
                        comboBoxReadWrite.Items.Clear();
                        comboBoxReadWrite.Items.Add("NO Write!");
                        comboBoxReadWrite.Items.Add("Read");
                        comboBoxReadWrite.Enabled = false;
                        comboBoxReadWrite.SelectedIndex = 1;
                    }
                    break;

                case (byte)SWITCHINDEX.SETAUDITCODE:
                    {
                        comboBoxParameters.Enabled = true;
                        comboBoxParameters.Items.Clear();
                        labelStatusMessage.Text = "Please wait...generating audit numbers...";
                        UpdateLogMessage(" Please wait...generating audit numbers...");
                        this.Update();
                        for (UInt16 n = 0; n < 65535; n++)
                            comboBoxParameters.Items.Add(n);
                        comboBoxParameters.SelectedIndex = 0;

                        // set to read as default
                        comboBoxReadWrite.Items.Clear();
                        comboBoxReadWrite.Items.Add("Write");
                        comboBoxReadWrite.Items.Add("Read");
                        comboBoxReadWrite.Enabled = true;
                        comboBoxReadWrite.SelectedIndex = 1;
                    }
                    break;

                case (byte)SWITCHINDEX.TEMP:
                    {
                        // show version
                        textBoxCommandString.Enabled = true;
                        textBoxCommandString.Text = "000 C";

                        // set to read as default
                        comboBoxReadWrite.Items.Clear();
                        comboBoxReadWrite.Items.Add("NO Write!");
                        comboBoxReadWrite.Items.Add("Read");
                        comboBoxReadWrite.Enabled = false;
                        comboBoxReadWrite.SelectedIndex = 1;
                    }
                    break;

                case (byte)SWITCHINDEX.VOLTS:
                    {
                        // show version
                        textBoxCommandString.Enabled = true;
                        textBoxCommandString.Text = "00 VDC";

                        // set to read as default
                        comboBoxReadWrite.Items.Clear();
                        comboBoxReadWrite.Items.Add("NO Write!");
                        comboBoxReadWrite.Items.Add("Read");
                        comboBoxReadWrite.Enabled = false;
                        comboBoxReadWrite.SelectedIndex = 1;
                    }
                    break;

                case (byte)SWITCHINDEX.READDRVVERSION:
                    {
                        // show version
                        textBoxCommandString.Enabled = true;
                        textBoxCommandString.Text = "00.0000.0";

                        // set to read as default
                        comboBoxReadWrite.Items.Clear();
                        comboBoxReadWrite.Items.Add("Write");
                        comboBoxReadWrite.Items.Add("Read");
                        comboBoxReadWrite.Enabled = false;
                        comboBoxReadWrite.SelectedIndex = 1;
                    }
                    break;

                case (byte)SWITCHINDEX.SETI2CADDRESS:
                    {
                        // show I2C address
                        textBoxI2CAddress.Enabled = true;
                        textBoxI2CAddress.Text = Convert.ToString(byI2CAddress);

                        // set to read as default
                        comboBoxReadWrite.Items.Clear();
                        comboBoxReadWrite.Items.Add("Write");
                        comboBoxReadWrite.Items.Add("Read");
                        comboBoxReadWrite.Enabled = true;
                        comboBoxReadWrite.SelectedIndex = 1;
                    }
                    break;

                case (byte)SWITCHINDEX.RESET:
                    {
                        comboBoxParameters.Enabled = true;
                        comboBoxParameters.Items.Clear();
                        comboBoxParameters.Items.Add("Soft Reset (no ACK)");
                        comboBoxParameters.Items.Add("Load Defaults from internal settings");
                        comboBoxParameters.Items.Add("Load last saved FLASH values");
                        comboBoxParameters.SelectedIndex = 0;

                        // can only write this one
                        comboBoxReadWrite.Items.Clear();
                        comboBoxReadWrite.Items.Add("Write");
                        comboBoxReadWrite.Enabled = true;
                    }
                    break;

                default:
                    break;
            }
        }

        // USB send routine
        private void timerSerialPush_Tick(object sender, EventArgs e)
        {
            // our indexer into the array as we build it
            z = 0;

            try
            {
                if (SerialPort.IsOpen)
                {
                    // this is to see if we pushed a button and need to do a serial send stream
                    if (byForceSerial != 0)
                    {
                        switch (byForceSerial)
                        {
                            case (byte)CONSOLECOMMANDS.PING:
                                {
                                    byArray[z++] = (byte)CONSOLECOMMANDS.PING;
                                    byArray[z++] = (byte)PINGLENGTH;
                                    byArray[z++] = (byte)byAuditCode++;
                                    byArray[z++] = (byte)comboBoxReadWrite.SelectedIndex;
                                    byArray[z++] = (byte)ACKGOODRESPONSE;
                                    byArray[z++] = (byte)comboBoxParameters.SelectedIndex;
                                    SerialPort.Write(byArray, 0, z);

                                    // wait for the reponse
                                    timerComRead.Enabled = true;
                                    labelStatusMessage.Text = "Sent PING message...waiting for reply";
                                    UpdateLogMessage("Sent " + ToHex((byte)CONSOLECOMMANDS.PING) + " PING message...waiting for reply...");
                                }
                                break;

                            case (byte)CONSOLECOMMANDS.ACK:
                                {
                                    byArray[z++] = (byte)CONSOLECOMMANDS.ACK;
                                    byArray[z++] = (byte)ACKLENGTH;
                                    byArray[z++] = (byte)byAuditCode++;
                                    byArray[z++] = (byte)comboBoxReadWrite.SelectedIndex;
                                    byArray[z++] = (byte)ACKGOODRESPONSE;
                                    byArray[z++] = (byte)comboBoxParameters.SelectedIndex;
                                    GenerateChecksum();
                                    SerialPort.Write(byArray, 0, z);

                                    // wait for the reponse
                                    timerComRead.Enabled = true;

                                    labelStatusMessage.Text = "Sent ACK message...waiting for reply";
                                    UpdateLogMessage("Sent " + ToHex((byte)CONSOLECOMMANDS.ACK) + " ACK message...waiting for reply...");
                                }
                                break;

                            case (byte)CONSOLECOMMANDS.AUTOSAVE:
                                {
                                    byArray[z++] = (byte)CONSOLECOMMANDS.AUTOSAVE;
                                    byArray[z++] = (byte)AUTOSAVELENGTH;
                                    byArray[z++] = (byte)byAuditCode++;
                                    byArray[z++] = (byte)comboBoxReadWrite.SelectedIndex;
                                    byArray[z++] = (byte)ACKGOODRESPONSE;
                                    byArray[z++] = (byte)comboBoxParameters.SelectedIndex;
                                    GenerateChecksum();
                                    SerialPort.Write(byArray, 0, z);

                                    // wait for the reponse
                                    timerComRead.Enabled = true;

                                    labelStatusMessage.Text = "Sent AUTOSAVE message...waiting for reply";
                                    UpdateLogMessage("Sent " + ToHex((byte)CONSOLECOMMANDS.AUTOSAVE) + " AUTOSAVE message...waiting for reply...");
                                }
                                break;

                            case (byte)CONSOLECOMMANDS.READCHUBVERSION:
                                {
                                    byArray[z++] = (byte)CONSOLECOMMANDS.READCHUBVERSION;
                                    byArray[z++] = (byte)READCHUBVERSIONLENGTH;
                                    byArray[z++] = (byte)byAuditCode++;
                                    byArray[z++] = (byte)READWRITE.READ;        // read only!!
                                    byArray[z++] = (byte)ACKGOODRESPONSE;
                                    byArray[z++] = (byte)0x00;                  // dummy bytes
                                    byArray[z++] = (byte)0x00;
                                    byArray[z++] = (byte)0x00;
                                    byArray[z++] = (byte)0x00;
                                    GenerateChecksum();
                                    SerialPort.Write(byArray, 0, z);

                                    // wait for the reponse
                                    timerComRead.Enabled = true;

                                    labelStatusMessage.Text = "Sent Read cHUB version message...waiting for reply";
                                    UpdateLogMessage("Sent " + ToHex((byte)CONSOLECOMMANDS.READCHUBVERSION) + " read cHUB version message...waiting for reply...");
                                }
                                break;

                            case (byte)CONSOLECOMMANDS.SENDTEMPERATURE:
                                {
                                    byArray[z++] = (byte)CONSOLECOMMANDS.SENDTEMPERATURE;
                                    byArray[z++] = (byte)SENDTEMPERATURELENGTH;
                                    byArray[z++] = (byte)byAuditCode++;
                                    byArray[z++] = (byte)READWRITE.READ;        // read only!!
                                    byArray[z++] = (byte)ACKGOODRESPONSE;
                                    byArray[z++] = (byte)0x00;                  // dummy bytes
                                    byArray[z++] = (byte)0x00;
                                    GenerateChecksum();
                                    SerialPort.Write(byArray, 0, z);

                                    // wait for the reponse
                                    timerComRead.Enabled = true;

                                    labelStatusMessage.Text = "Sent Read cHUB temperature message...waiting for reply";
                                    UpdateLogMessage("Sent " + ToHex((byte)CONSOLECOMMANDS.SENDTEMPERATURE) + " read cHUB temperature message...waiting for reply...");
                                }
                                break;

                            case (byte)CONSOLECOMMANDS.TEMP:
                                {
                                    byArray[z++] = (byte)CONSOLECOMMANDS.TEMP;
                                    byArray[z++] = (byte)TEMPLENGTH;
                                    byArray[z++] = (byte)byAuditCode++;
                                    byArray[z++] = (byte)READWRITE.READ;        // read only!!
                                    byArray[z++] = (byte)ACKGOODRESPONSE;
                                    byArray[z++] = (byte)0x00;
                                    GenerateChecksum();
                                    SerialPort.Write(byArray, 0, z);

                                    // wait for the reponse
                                    timerComRead.Enabled = true;

                                    labelStatusMessage.Text = "Sent Read Driver Temp message...waiting for reply";
                                    UpdateLogMessage("Sent " + ToHex((byte)CONSOLECOMMANDS.TEMP) + " Driver Temp message...waiting for reply...");
                                }
                                break;

                            case (byte)CONSOLECOMMANDS.VOLTS:
                                {
                                    byArray[z++] = (byte)CONSOLECOMMANDS.VOLTS;
                                    byArray[z++] = (byte)TEMPLENGTH;
                                    byArray[z++] = (byte)byAuditCode++;
                                    byArray[z++] = (byte)READWRITE.READ;        // read only!!
                                    byArray[z++] = (byte)ACKGOODRESPONSE;
                                    byArray[z++] = (byte)0x00;
                                    GenerateChecksum();
                                    SerialPort.Write(byArray, 0, z);

                                    // wait for the reponse
                                    timerComRead.Enabled = true;

                                    labelStatusMessage.Text = "Sent Read Driver Volts message...waiting for reply";
                                    UpdateLogMessage("Sent " + ToHex((byte)CONSOLECOMMANDS.VOLTS) + " Driver Volts message...waiting for reply...");
                                }
                                break;

                            case (byte)CONSOLECOMMANDS.READDRVVERSION:
                                {
                                    byArray[z++] = (byte)CONSOLECOMMANDS.READDRVVERSION;
                                    byArray[z++] = (byte)READDRVVERSIONLENGTH;
                                    byArray[z++] = (byte)byAuditCode++;
                                    byArray[z++] = (byte)READWRITE.READ;        // read only!!
                                    byArray[z++] = (byte)ACKGOODRESPONSE;
                                    byArray[z++] = (byte)0x00;                  // dummy bytes
                                    byArray[z++] = (byte)0x00;
                                    byArray[z++] = (byte)0x00;
                                    byArray[z++] = (byte)0x00;
                                    GenerateChecksum();
                                    SerialPort.Write(byArray, 0, z);

                                    // wait for the reponse
                                    timerComRead.Enabled = true;

                                    labelStatusMessage.Text = "Sent Read Driver version message...waiting for reply";
                                    UpdateLogMessage("Sent " + ToHex((byte)CONSOLECOMMANDS.READDRVVERSION) + " read Driver version message...waiting for reply...");
                                }
                                break;

                            case (byte)CONSOLECOMMANDS.SETI2CADDRESS:
                                {
                                    byte I2CAddressHere = byI2CAddress;

                                    byArray[z++] = (byte)CONSOLECOMMANDS.SETI2CADDRESS;
                                    byArray[z++] = (byte)SETI2CADDRESSLENGTH;
                                    byArray[z++] = (byte)byAuditCode++;
                                    byArray[z++] = (byte)comboBoxReadWrite.SelectedIndex;
                                    byArray[z++] = (byte)ACKGOODRESPONSE;

                                    // make sure we have a number there
                                    if (comboBoxReadWrite.SelectedIndex == (byte)READWRITE.WRITE)
                                    {
                                        try
                                        {
                                            I2CAddressHere = Convert.ToByte(textBoxI2CAddress.Text);
                                            byArray[z++] = (byte)I2CAddressHere;
                                        }
                                        catch
                                        {
                                            labelStatusMessage.Text = "Need a good number for the I2C address...";
                                            UpdateLogMessage("Need a good number for the I2C address...");
                                        }
                                    }
                                    else
                                        byArray[z++] = 0;

                                    // show data value sent
                                    textBoxCommandString.Text = Convert.ToString(I2CAddressHere) + " I2C Address";
                                    byI2CAddress = I2CAddressHere;

                                    GenerateChecksum();
                                    SerialPort.Write(byArray, 0, z);

                                    // wait for the reponse
                                    timerComRead.Enabled = true;

                                    labelStatusMessage.Text = "Sent Driver I2C address...waiting for reply";
                                    UpdateLogMessage("Sent " + ToHex((byte)CONSOLECOMMANDS.SETI2CADDRESS) + " Driver I2C address message...waiting for reply...");
                                }
                                break;

                            case (byte)CONSOLECOMMANDS.SETAUDITCODE:
                                {
                                    byArray[z++] = (byte)CONSOLECOMMANDS.SETAUDITCODE;
                                    byArray[z++] = (byte)AUDITCODELENGTH;
                                    byArray[z++] = (byte)byAuditCode++;
                                    byArray[z++] = (byte)comboBoxReadWrite.SelectedIndex;
                                    byArray[z++] = (byte)ACKGOODRESPONSE;
                                    // get the UINT16 and break it into two bytes
                                    UInt16 uiAuditCode = (UInt16)comboBoxParameters.SelectedIndex;
                                    byArray[z++] = (byte)(uiAuditCode >> 8);
                                    byArray[z++] = (byte)uiAuditCode;
                                    GenerateChecksum();
                                    SerialPort.Write(byArray, 0, z);

                                    labelStatusMessage.Text = "Sent Set Audit Code message...waiting for reply";
                                    UpdateLogMessage("Sent " + ToHex((byte)CONSOLECOMMANDS.SETAUDITCODE) + " Audit Code message...waiting for reply...");

                                    // wait for the reponse
                                    timerComRead.Enabled = true;
                                }
                                break;

                            case (byte)CONSOLECOMMANDS.RESET:
                                {
                                    byArray[z++] = (byte)CONSOLECOMMANDS.RESET;
                                    labelStatusMessage.Text = "Sent Reset message...waiting for reply";
                                    UpdateLogMessage("Sent " + ToHex((byte)CONSOLECOMMANDS.RESET) + " Sent Reset message...waiting for reply...");

                                    byArray[z++] = (byte)RESETLENGTH;
                                    byArray[z++] = (byte)byAuditCode++;
                                    byArray[z++] = (byte)READWRITE.WRITE;
                                    byArray[z++] = (byte)ACKGOODRESPONSE;
                                    byArray[z++] = (byte)comboBoxParameters.SelectedIndex;
                                    GenerateChecksum();
                                    SerialPort.Write(byArray, 0, z);

                                    // wait for the reponse
                                    timerComRead.Enabled = true;
                                }
                                break;

                            case (byte)CONSOLECOMMANDS.SLIDERCH1:
                                {
                                    byArray[z++] = (byte)CONSOLECOMMANDS.SLIDERCH1;
                                    byArray[z++] = (byte)SLIDERLENGTH;
                                    byArray[z++] = (byte)byAuditCode++;
                                    byArray[z++] = (byte)comboBoxReadWrite.SelectedIndex;
                                    byArray[z++] = (byte)ACKGOODRESPONSE;
                                    byArray[z++] = (byte)0x00;                  // dummy bytes
                                    byArray[z++] = (byte)0x00;
                                    GenerateChecksum();
                                    SerialPort.Write(byArray, 0, z);

                                    // wait for the reponse
                                    timerComRead.Enabled = true;

                                    labelStatusMessage.Text = "Sent Read CH1 message...waiting for reply";
                                    UpdateLogMessage("Sent " + ToHex((byte)CONSOLECOMMANDS.SLIDERCH1) + " read CH1 message...waiting for reply...");
                                }
                                break;

                            case (byte)CONSOLECOMMANDS.SLIDERCH2:
                                {
                                    byArray[z++] = (byte)CONSOLECOMMANDS.SLIDERCH2;
                                    byArray[z++] = (byte)SLIDERLENGTH;
                                    byArray[z++] = (byte)byAuditCode++;
                                    byArray[z++] = (byte)comboBoxReadWrite.SelectedIndex;
                                    byArray[z++] = (byte)ACKGOODRESPONSE;
                                    byArray[z++] = (byte)0x00;                  // dummy bytes
                                    byArray[z++] = (byte)0x00;
                                    GenerateChecksum();
                                    SerialPort.Write(byArray, 0, z);

                                    // wait for the reponse
                                    timerComRead.Enabled = true;

                                    labelStatusMessage.Text = "Sent Read CH2 message...waiting for reply";
                                    UpdateLogMessage("Sent " + ToHex((byte)CONSOLECOMMANDS.SLIDERCH2) + " read CH2 message...waiting for reply...");
                                }
                                break;

                            case (byte)CONSOLECOMMANDS.SLIDERCH3:
                                {
                                    byArray[z++] = (byte)CONSOLECOMMANDS.SLIDERCH3;
                                    byArray[z++] = (byte)SLIDERLENGTH;
                                    byArray[z++] = (byte)byAuditCode++;
                                    byArray[z++] = (byte)comboBoxReadWrite.SelectedIndex;
                                    byArray[z++] = (byte)ACKGOODRESPONSE;
                                    byArray[z++] = (byte)0x00;                  // dummy bytes
                                    byArray[z++] = (byte)0x00;
                                    GenerateChecksum();
                                    SerialPort.Write(byArray, 0, z);

                                    // wait for the reponse
                                    timerComRead.Enabled = true;

                                    labelStatusMessage.Text = "Sent Read CH3 message...waiting for reply";
                                    UpdateLogMessage("Sent " + ToHex((byte)CONSOLECOMMANDS.SLIDERCH3) + " read CH3 message...waiting for reply...");
                                }
                                break;

                            case (byte)CONSOLECOMMANDS.SLIDERCH4:
                                {
                                    byArray[z++] = (byte)CONSOLECOMMANDS.SLIDERCH4;
                                    byArray[z++] = (byte)SLIDERLENGTH;
                                    byArray[z++] = (byte)byAuditCode++;
                                    byArray[z++] = (byte)comboBoxReadWrite.SelectedIndex;
                                    byArray[z++] = (byte)ACKGOODRESPONSE;
                                    byArray[z++] = (byte)0x00;                  // dummy bytes
                                    byArray[z++] = (byte)0x00;
                                    GenerateChecksum();
                                    SerialPort.Write(byArray, 0, z);

                                    // wait for the reponse
                                    timerComRead.Enabled = true;

                                    labelStatusMessage.Text = "Sent Read CH4 message...waiting for reply";
                                    UpdateLogMessage("Sent " + ToHex((byte)CONSOLECOMMANDS.SLIDERCH4) + " read CH4 message...waiting for reply...");
                                }
                                break;

                            case (byte)CONSOLECOMMANDS.SLIDERCH5:
                                {
                                    byArray[z++] = (byte)CONSOLECOMMANDS.SLIDERCH5;
                                    byArray[z++] = (byte)SLIDERLENGTH;
                                    byArray[z++] = (byte)byAuditCode++;
                                    byArray[z++] = (byte)comboBoxReadWrite.SelectedIndex;
                                    byArray[z++] = (byte)ACKGOODRESPONSE;
                                    byArray[z++] = (byte)0x00;                  // dummy bytes
                                    byArray[z++] = (byte)0x00;
                                    GenerateChecksum();
                                    SerialPort.Write(byArray, 0, z);

                                    // wait for the reponse
                                    timerComRead.Enabled = true;

                                    labelStatusMessage.Text = "Sent Read CH5 message...waiting for reply";
                                    UpdateLogMessage("Sent " + ToHex((byte)CONSOLECOMMANDS.SLIDERCH5) + " read CH5 message...waiting for reply...");
                                }
                                break;

                            case (byte)CONSOLECOMMANDS.SLIDERCH6:
                                {
                                    byArray[z++] = (byte)CONSOLECOMMANDS.SLIDERCH6;
                                    byArray[z++] = (byte)SLIDERLENGTH;
                                    byArray[z++] = (byte)byAuditCode++;
                                    byArray[z++] = (byte)comboBoxReadWrite.SelectedIndex;
                                    byArray[z++] = (byte)ACKGOODRESPONSE;
                                    byArray[z++] = (byte)0x00;                  // dummy bytes
                                    byArray[z++] = (byte)0x00;
                                    GenerateChecksum();
                                    SerialPort.Write(byArray, 0, z);

                                    // wait for the reponse
                                    timerComRead.Enabled = true;

                                    labelStatusMessage.Text = "Sent Read CH6 message...waiting for reply";
                                    UpdateLogMessage("Sent " + ToHex((byte)CONSOLECOMMANDS.SLIDERCH6) + " read CH6 message...waiting for reply...");
                                }
                                break;


                            default:
                                break;
                        }

                        // reset the status to be idle
                        byForceSerial = 0;
                        timerSerialPush.Enabled = false;
                    }
                }
            }
            catch
            {
                labelStatusMessage.Text = "Something happened to the serial port during a WRITE, stopping sample timer...";
                UpdateLogMessage("Something happened to the serial port during a WRITE, stopping sample timer...");

                timerSerialPush.Enabled = false;
                timerComRead.Enabled = false;

                buttonOpenUSBPort.BackColor = Color.Yellow;
                buttonOpenUSBPort.Text = "Open USB";
                buttonComTest.BackColor = Color.LightGray;

                byForceSerial = 0;
            }
        }

        // this uses byArray and z for it's parameters
        private void GenerateChecksum()
        {
            byte byData = 0;
            byte n;

            // logical AND of all the data bytes. less the checksum
            for (n = 0; n < byArray[1] - 1; n++)
                byData += (byte)byArray[n];

            // load this up into array, no z increment!
            byArray[z++] = byData;
        }

        // USB receive routine
        private void timerComRead_Tick(object sender, EventArgs e)
        {
            byte command, auditcode, ackcode;
            int numbytes;
            byte[] data = new byte[25000];

            try
            {
                if (SerialPort.IsOpen && SerialPort.BytesToRead != 0)
                {
                    // give  10msec to get them all
                    //                    sleep(5);
                    int iPortBytes = SerialPort.BytesToRead;
                    SerialPort.Read(data, 0, SerialPort.BytesToRead);

                    // parse out the command and data
                    command = data[(byte)PACKETA.COMMAND];
                    numbytes = (int)data[(byte)PACKETA.LENGTH];                // this is the number of bytes
                    auditcode = data[(byte)PACKETA.AUDIT];
                    ackcode = data[(byte)PACKETA.ACK];

                    // write return is a simple ACK if that is enabled
                    if (comboBoxReadWrite.SelectedIndex == (byte)READWRITE.WRITE)
                    {
                        if (command == byReadSerial)
                        {
                            if (command >= (byte)CONSOLECOMMANDS.SLIDERCH1 && command <= (byte)CONSOLECOMMANDS.SLIDERCH6)
                            {
                                UpdateLogMessage("Good SLIDER response received...");
                                labelStatusMessage.Text = "Good SLIDER response received...";
                            }
                            else if (auditcode == (byAuditCode - 1))
                            {
                                // this is was a write command, the ACK command is in the 3rd byte position, which is the LENGTH
                                if (ackcode == ACKGOODRESPONSE)
                                {
                                    UpdateLogMessage("Good " + ToHex(ackcode) + " ACK response received...");
                                    labelStatusMessage.Text = "Good " + ToHex(ackcode) + " ACK response received...";
                                }
                                else if (ackcode == ACKBADRESPONSE)
                                {
                                    UpdateLogMessage("Bad " + ToHex(ackcode) + " ACK response received...");
                                    labelStatusMessage.Text = "Bad " + ToHex(ackcode) + " ACK response received...";
                                }
                                else if (ackcode == ACKBADCHECKSUMRESPONSE)
                                {
                                    UpdateLogMessage("Checksum bad " + ToHex(ackcode) + " ACK response received...");
                                    labelStatusMessage.Text = "Checksum bad " + ToHex(ackcode) + " ACK response received...";
                                }
                                else if (ackcode == PINGRESPONSE)
                                {
                                    bWeGotData = true;

                                    UpdateLogMessage("Ping " + ToHex(ackcode) + " ACK response received...");
                                    labelStatusMessage.Text = "Ping " + ToHex(ackcode) + " ACK response received...";
                                }
                                else
                                {
                                    UpdateLogMessage("Unknown " + ToHex(ackcode) + " ACK response received...");
                                    labelStatusMessage.Text = "Unknown " + ToHex(ackcode) + " ACK response received...";
                                }
                            }
                            else
                            {
                                UpdateLogMessage("Bad auditcode byte reflected " + ToHex(auditcode) + "...");
                                labelStatusMessage.Text = "Bad auditcode byte reflected " + ToHex(auditcode) + " ...";
                            }
                        }
                        else
                        {
                            UpdateLogMessage("Wrong command byte reflected " + ToHex(command) + "...");
                            labelStatusMessage.Text = "Wrong comamnd byte reflected " + ToHex(command) + " ...";
                        }
                    }
                    // this is the read return
                    else if (comboBoxReadWrite.SelectedIndex == (byte)READWRITE.READ)
                    {
                        switch (byReadSerial)
                        {
                            case (byte)CONSOLECOMMANDS.ACK:
                                {
                                    // make sure that the checksum is good that we sent
                                    if (ackcode != ACKBADCHECKSUMRESPONSE)
                                    {
                                        // show Read or Write value received
                                        if (data[(byte)PACKETA.READWRITE] == (byte)READWRITE.READ)
                                            comboBoxReadWrite.SelectedIndex = 1;
                                        else
                                            comboBoxReadWrite.Text = "Unknown RW state: " + data[(byte)PACKETA.READWRITE];

                                        // show data value received
                                        if (data[(byte)PACKETA.DATA] < 2)
                                            comboBoxParameters.SelectedIndex = data[(byte)PACKETA.DATA];
                                        else
                                            comboBoxParameters.Text = "Unknown ACK state: " + data[(byte)PACKETA.DATA];

                                        UpdateLogMessage("Read ACK " + ToHex(command) + " read...");
                                        labelStatusMessage.Text = "Read ACK " + ToHex(command) + " read...";
                                    }
                                    else
                                    {
                                        UpdateLogMessage("ACK read command Checksum bad " + ToHex(command) + " ACK response received...");
                                        labelStatusMessage.Text = "ACK read Checksum bad " + ToHex(command) + " ACK response received...";
                                    }
                                }
                                break;

                            case (byte)CONSOLECOMMANDS.AUTOSAVE:
                                {
                                    // make sure that the checksum is good that we sent
                                    if (ackcode != ACKBADCHECKSUMRESPONSE)
                                    {
                                        // show Read or Write value received
                                        if (data[(byte)PACKETA.READWRITE] == (byte)READWRITE.READ)
                                            comboBoxReadWrite.SelectedIndex = 1;
                                        else
                                            comboBoxReadWrite.Text = "Unknown RW state: " + data[(byte)PACKETA.READWRITE];

                                        // show data value received
                                        if (data[(byte)PACKETA.DATA] < 3)
                                            comboBoxParameters.SelectedIndex = data[(byte)PACKETA.DATA];
                                        else
                                            comboBoxParameters.Text = "Unknown Austosave state: " + data[(byte)PACKETA.DATA];

                                        UpdateLogMessage("Read AutoSAVE " + ToHex(command) + " read...");
                                        labelStatusMessage.Text = "Read AutoSAVE " + ToHex(command) + " read...";
                                    }
                                    else
                                    {
                                        UpdateLogMessage("AutoSAVE read command Checksum bad " + ToHex(command) + " ACK response received...");
                                        labelStatusMessage.Text = "AutoSAVE read Checksum bad " + ToHex(command) + " ACK response received...";
                                    }
                                }
                                break;

                            case (byte)CONSOLECOMMANDS.READCHUBVERSION:
                                {
                                    // make sure that the checksum is good that we sent
                                    if (ackcode != ACKBADCHECKSUMRESPONSE)
                                    {
                                        // show data value received
                                        string cHUBVersion = (data[(byte)PACKETE.E_DATA1] - 48).ToString() + "." + (data[(byte)PACKETE.E_DATA2] - 48).ToString() + (data[(byte)PACKETE.E_DATA3] - 48).ToString() + "." + (data[(byte)PACKETE.E_DATA4] - 48).ToString();
                                        textBoxCommandString.Text = cHUBVersion;

                                        UpdateLogMessage("Read " + ToHex(command) + " cHUB version " + cHUBVersion + " read updated OK...");
                                        labelStatusMessage.Text = "Read " + ToHex(command) + " cHUB version " + cHUBVersion + " read updated OK...";
                                    }
                                    else
                                    {
                                        UpdateLogMessage("cHUB version read command Checksum bad " + ToHex(command) + " ACK response received...");
                                        labelStatusMessage.Text = "cHUB version read Checksum bad " + ToHex(command) + " ACK response received...";
                                    }
                                }
                                break;

                            case (byte)CONSOLECOMMANDS.SENDTEMPERATURE:
                                {
                                    // make sure that the checksum is good that we sent
                                    if (ackcode != ACKBADCHECKSUMRESPONSE)
                                    {
                                        // show data value received
                                        UInt16 uiADMTemp = (UInt16)data[(byte)PACKETD.DATAH];
                                        uiADMTemp <<= 8;
                                        uiADMTemp += (UInt16)data[(byte)PACKETD.DATAL];
                                        textBoxCommandString.Text = uiADMTemp.ToString();

                                        UpdateLogMessage("Read " + ToHex(command) + " cHUB version " + uiADMTemp + " read updated OK...");
                                        labelStatusMessage.Text = "Read " + ToHex(command) + " cHUB version " + uiADMTemp + " read updated OK...";
                                    }
                                    else
                                    {
                                        UpdateLogMessage("cHUB version read command Checksum bad " + ToHex(command) + " ACK response received...");
                                        labelStatusMessage.Text = "cHUB version read Checksum bad " + ToHex(command) + " ACK response received...";
                                    }
                                }
                                break;

                            case (byte)CONSOLECOMMANDS.SETAUDITCODE:
                                {
                                    // make sure that the checksum is good that we sent
                                    if (ackcode != ACKBADCHECKSUMRESPONSE)
                                    {
                                        // show data value received
                                        UInt16 uiAuditCode = (UInt16)data[(byte)PACKETD.DATAH];
                                        uiAuditCode <<= 8;
                                        uiAuditCode += (UInt16)data[(byte)PACKETD.DATAL];
                                        comboBoxParameters.SelectedIndex = uiAuditCode;

                                        UpdateLogMessage("Read " + ToHex(command) + " Audit Code " + uiAuditCode + " read updated OK...");
                                        labelStatusMessage.Text = "Read " + ToHex(command) + " Audit Code " + uiAuditCode + " read updated OK...";
                                    }
                                    else
                                    {
                                        UpdateLogMessage("Audit Code read command Checksum bad " + ToHex(command) + " ACK response received...");
                                        labelStatusMessage.Text = "Audit Code read Checksum bad " + ToHex(command) + " ACK response received...";
                                    }
                                }
                                break;

                            case (byte)CONSOLECOMMANDS.TEMP:
                                {
                                    // make sure that the checksum is good that we sent
                                    if (ackcode != ACKBADCHECKSUMRESPONSE)
                                    {
                                        // show Read or Write value received
                                        if (data[(byte)PACKETA.READWRITE] == (byte)READWRITE.READ)
                                            comboBoxReadWrite.SelectedIndex = 1;
                                        else
                                            comboBoxReadWrite.Text = "Unknown RW state: " + data[(byte)PACKETA.READWRITE];

                                        // show data value received
                                        textBoxCommandString.Text = data[(byte)PACKETA.DATA].ToString() + " C";

                                        UpdateLogMessage("Read Driver Temp " + ToHex(command) + " read...");
                                        labelStatusMessage.Text = "Read Driver Temp " + ToHex(command) + " read...";
                                    }
                                    else
                                    {
                                        UpdateLogMessage("Driver Temp read command Checksum bad " + ToHex(command) + " ACK response received...");
                                        labelStatusMessage.Text = "Driver Temp read Checksum bad " + ToHex(command) + " ACK response received...";
                                    }
                                }
                                break;

                            case (byte)CONSOLECOMMANDS.VOLTS:
                                {
                                    // make sure that the checksum is good that we sent
                                    if (ackcode != ACKBADCHECKSUMRESPONSE)
                                    {
                                        // show Read or Write value received
                                        if (data[(byte)PACKETA.READWRITE] == (byte)READWRITE.READ)
                                            comboBoxReadWrite.SelectedIndex = 1;
                                        else
                                            comboBoxReadWrite.Text = "Unknown RW state: " + data[(byte)PACKETA.READWRITE];

                                        // show data value received
                                        textBoxCommandString.Text = data[(byte)PACKETA.DATA].ToString() + " VDC";

                                        UpdateLogMessage("Read Driver Volts " + ToHex(command) + " read...");
                                        labelStatusMessage.Text = "Read Driver Volts " + ToHex(command) + " read...";
                                    }
                                    else
                                    {
                                        UpdateLogMessage("Driver Volts read command Checksum bad " + ToHex(command) + " ACK response received...");
                                        labelStatusMessage.Text = "Driver Volts read Checksum bad " + ToHex(command) + " ACK response received...";
                                    }
                                }
                                break;

                            case (byte)CONSOLECOMMANDS.SETI2CADDRESS:
                                {
                                    // make sure that the checksum is good that we sent
                                    if (ackcode != ACKBADCHECKSUMRESPONSE)
                                    {
                                        // show Read or Write value received
                                        if (data[(byte)PACKETA.READWRITE] == (byte)READWRITE.READ)
                                            comboBoxReadWrite.SelectedIndex = 1;
                                        else
                                            comboBoxReadWrite.SelectedIndex = 0;

                                        // show data value received
                                        textBoxI2CAddress.Text = data[(byte)PACKETA.DATA].ToString();
                                        textBoxCommandString.Text = data[(byte)PACKETA.DATA].ToString() + " I2C Address";
                                        byI2CAddress = data[(byte)PACKETA.DATA];

                                        UpdateLogMessage("Read Driver I2C address " + ToHex(command) + " read...");
                                        labelStatusMessage.Text = "Read Driver I2C address " + ToHex(command) + " read...";
                                    }
                                    else
                                    {
                                        UpdateLogMessage("Driver I2C address command Checksum bad " + ToHex(command) + " ACK response received...");
                                        labelStatusMessage.Text = "Driver I2C address Checksum bad " + ToHex(command) + " ACK response received...";
                                    }
                                }
                                break;

                            case (byte)CONSOLECOMMANDS.READDRVVERSION:
                                {
                                    // make sure that the checksum is good that we sent
                                    if (ackcode != ACKBADCHECKSUMRESPONSE)
                                    {
                                        // show data value received
                                        string DrvVersion = (data[(byte)PACKETE.E_DATA1] - 48).ToString() + "." + (data[(byte)PACKETE.E_DATA2] - 48).ToString() + (data[(byte)PACKETE.E_DATA3] - 48).ToString() + "." + System.Convert.ToChar(data[(byte)PACKETE.E_DATA4]);
                                        textBoxCommandString.Text = DrvVersion;

                                        UpdateLogMessage("Read " + ToHex(command) + " Driver version " + DrvVersion + " read updated OK...");
                                        labelStatusMessage.Text = "Read " + ToHex(command) + " Driver version " + DrvVersion + " read updated OK...";
                                    }
                                    else
                                    {
                                        UpdateLogMessage("Driver version read command Checksum bad " + ToHex(command) + " ACK response received...");
                                        labelStatusMessage.Text = "Driver version read Checksum bad " + ToHex(command) + " ACK response received...";
                                    }
                                }
                                break;

                            case (byte)CONSOLECOMMANDS.SLIDERCH1:
                                {
                                    // make sure that the checksum is good that we sent
                                    if (ackcode != ACKBADCHECKSUMRESPONSE)
                                    {
                                        // show data value received
                                        trackBarCH1.Value = ((UInt16)data[(byte)PACKETD.DATAH] << 8) + (UInt16)data[(byte)PACKETD.DATAL];
                                        trackBarCH1.Update();
                                        CH1UpDown.Value = trackBarCH1.Value;
                                        bWeGotData = true;

                                        UpdateLogMessage("Read " + ToHex(command) + " CH1 value " + trackBarCH1.Value + " read updated OK...");
                                        labelStatusMessage.Text = "Read " + ToHex(command) + " CH1 " + trackBarCH1.Value + " read updated OK...";
                                    }
                                    else
                                    {
                                        UpdateLogMessage("CH1 read command Checksum bad " + ToHex(command) + " ACK response received...");
                                        labelStatusMessage.Text = "CH1 read Checksum bad " + ToHex(command) + " ACK response received...";
                                    }
                                }
                                break;

                            case (byte)CONSOLECOMMANDS.SLIDERCH2:
                                {
                                    // make sure that the checksum is good that we sent
                                    if (ackcode != ACKBADCHECKSUMRESPONSE)
                                    {
                                        // show data value received
                                        trackBarCH2.Value = ((UInt16)data[(byte)PACKETD.DATAH] << 8) + (UInt16)data[(byte)PACKETD.DATAL];
                                        trackBarCH2.Update();
                                        CH2UpDown.Value = trackBarCH2.Value;
                                        bWeGotData = true;

                                        UpdateLogMessage("Read " + ToHex(command) + " CH2 value " + trackBarCH2.Value + " read updated OK...");
                                        labelStatusMessage.Text = "Read " + ToHex(command) + " CH2 " + trackBarCH2.Value + " read updated OK...";
                                    }
                                    else
                                    {
                                        UpdateLogMessage("CH2 read command Checksum bad " + ToHex(command) + " ACK response received...");
                                        labelStatusMessage.Text = "CH2 read Checksum bad " + ToHex(command) + " ACK response received...";
                                    }
                                }
                                break;

                            case (byte)CONSOLECOMMANDS.SLIDERCH3:
                                {
                                    // make sure that the checksum is good that we sent
                                    if (ackcode != ACKBADCHECKSUMRESPONSE)
                                    {
                                        // show data value received
                                        trackBarCH3.Value = ((UInt16)data[(byte)PACKETD.DATAH] << 8) + (UInt16)data[(byte)PACKETD.DATAL];
                                        trackBarCH3.Update();
                                        CH3UpDown.Value = trackBarCH3.Value;
                                        bWeGotData = true;

                                        UpdateLogMessage("Read " + ToHex(command) + " CH3 value " + trackBarCH3.Value + " read updated OK...");
                                        labelStatusMessage.Text = "Read " + ToHex(command) + " CH3 " + trackBarCH3.Value + " read updated OK...";
                                    }
                                    else
                                    {
                                        UpdateLogMessage("CH3 read command Checksum bad " + ToHex(command) + " ACK response received...");
                                        labelStatusMessage.Text = "CH3 read Checksum bad " + ToHex(command) + " ACK response received...";
                                    }
                                }
                                break;

                            case (byte)CONSOLECOMMANDS.SLIDERCH4:
                                {
                                    // make sure that the checksum is good that we sent
                                    if (ackcode != ACKBADCHECKSUMRESPONSE)
                                    {
                                        // show data value received
                                        trackBarCH4.Value = ((UInt16)data[(byte)PACKETD.DATAH] << 8) + (UInt16)data[(byte)PACKETD.DATAL];
                                        trackBarCH4.Update();
                                        CH4UpDown.Value = trackBarCH4.Value;
                                        bWeGotData = true;

                                        UpdateLogMessage("Read " + ToHex(command) + " CH4 value " + trackBarCH4.Value + " read updated OK...");
                                        labelStatusMessage.Text = "Read " + ToHex(command) + " CH4 " + trackBarCH4.Value + " read updated OK...";
                                    }
                                    else
                                    {
                                        UpdateLogMessage("CH4 read command Checksum bad " + ToHex(command) + " ACK response received...");
                                        labelStatusMessage.Text = "CH4 read Checksum bad " + ToHex(command) + " ACK response received...";
                                    }
                                }
                                break;

                            case (byte)CONSOLECOMMANDS.SLIDERCH5:
                                {
                                    // make sure that the checksum is good that we sent
                                    if (ackcode != ACKBADCHECKSUMRESPONSE)
                                    {
                                        // show data value received
                                        trackBarCH5.Value = ((UInt16)data[(byte)PACKETD.DATAH] << 8) + (UInt16)data[(byte)PACKETD.DATAL];
                                        trackBarCH5.Update();
                                        CH5UpDown.Value = trackBarCH5.Value;
                                        bWeGotData = true;

                                        UpdateLogMessage("Read " + ToHex(command) + " CH5 value " + trackBarCH5.Value + " read updated OK...");
                                        labelStatusMessage.Text = "Read " + ToHex(command) + " CH5 " + trackBarCH5.Value + " read updated OK...";
                                    }
                                    else
                                    {
                                        UpdateLogMessage("CH5 read command Checksum bad " + ToHex(command) + " ACK response received...");
                                        labelStatusMessage.Text = "CH5 read Checksum bad " + ToHex(command) + " ACK response received...";
                                    }
                                }
                                break;

                            case (byte)CONSOLECOMMANDS.SLIDERCH6:
                                {
                                    // make sure that the checksum is good that we sent
                                    if (ackcode != ACKBADCHECKSUMRESPONSE)
                                    {
                                        // show data value received
                                        trackBarCH6.Value = ((UInt16)data[(byte)PACKETD.DATAH] << 8) + (UInt16)data[(byte)PACKETD.DATAL];
                                        trackBarCH6.Update();
                                        CH6UpDown.Value = trackBarCH6.Value;
                                        bWeGotData = true;

                                        UpdateLogMessage("Read " + ToHex(command) + " CH6 value " + trackBarCH6.Value + " read updated OK...");
                                        labelStatusMessage.Text = "Read " + ToHex(command) + " CH6 " + trackBarCH6.Value + " read updated OK...";
                                    }
                                    else
                                    {
                                        UpdateLogMessage("CH6 read command Checksum bad " + ToHex(command) + " ACK response received...");
                                        labelStatusMessage.Text = "CH6 read Checksum bad " + ToHex(command) + " ACK response received...";
                                    }
                                }
                                break;

                            default:
                                if (command == ACKBADRESPONSE)
                                {
                                    UpdateLogMessage("Bad " + ToHex(command) + " ACK response received...");
                                    labelStatusMessage.Text = "Bad " + ToHex(command) + " ACK response received...";
                                }
                                else
                                {
                                    UpdateLogMessage("Unknown  " + ToHex(command) + " response received...");
                                        labelStatusMessage.Text = "Unknown  " + ToHex(command) + " response received...";
                                }
                                break;
                        }
                    }

                    // clean up
                    SerialPort.DiscardInBuffer();
                    //                    timerComRead.Enabled = false;
                    // so that we don't do the same thing again, we are trapping DMX data here too now
                    byReadSerial = 0xFF;

                    // so that we can send after we come alive!
                    bTickAlive = true;
                }
            }
            catch
            {
                labelStatusMessage.Text = "Something happened to the serial port during a READ, stopping readback timer...";
                UpdateLogMessage("Something happened to the serial port during a READ, stopping readback timer...");
                timerComRead.Enabled = false;
                timerSerialPush.Enabled = false;

                buttonOpenUSBPort.BackColor = Color.Yellow;
                buttonOpenUSBPort.Text = "Open USB";
                buttonComTest.BackColor = Color.LightGray;
            }
        }

        private void UpdateLogMessage(string message)
        {
            DateTime currentime;

            currentime = DateTime.Now;
            listBoxSystemLog.Items.Add(String.Format("{0:00}", currentime.Hour) + ":" + String.Format("{0:00}", currentime.Minute) + ":" + String.Format("{0:00}", currentime.Second) + "  " + message);
        }

        //
        // small time delay routine in milliseconds
        //
        private void sleep(long sleeptime)
        {
            DateTime currentime;
            DateTime futuretime;
            long currenttimertick;
            long futuretimertick;

            currentime = DateTime.Now;
            currenttimertick = (currentime.Hour * 60 * 60 * 1000) + (currentime.Minute * 60 * 1000) +
                               (currentime.Second * 1000) + currentime.Millisecond;
            futuretimertick = currenttimertick;

            while (futuretimertick < currenttimertick + sleeptime)
            {
                futuretime = DateTime.Now;
                futuretimertick = (futuretime.Hour * 60 * 60 * 1000) + (futuretime.Minute * 60 * 1000) +
                                  (futuretime.Second * 1000 + futuretime.Millisecond);
            }
        }

        private void buttonComTest_Click(object sender, EventArgs e)
        {
            // can only write this one
            comboBoxReadWrite.Items.Clear();
            comboBoxReadWrite.Items.Add("Write");
            comboBoxReadWrite.Enabled = true;
            comboBoxReadWrite.SelectedIndex = 0;

            // do a quick serial com test, show GREEN on the COM TEST if so
            timerSerialPush.Enabled = true;
            byForceSerial = byReadSerial = (byte)CONSOLECOMMANDS.PING;
        }

        private static string ToHex(int value)
        {
            return String.Format("0x{0:X}", value);
        }

        private void ButtonClearLog_Click(object sender, EventArgs e)
        {
            listBoxSystemLog.Items.Clear();
            UpdateLogMessage("System log cleared...");
        }

        private void buttonSerialWatch_Click(object sender, EventArgs e)
        {
            FormSerialSnooper frm2 = new FormSerialSnooper(SerialPort);
            frm2.Show();
        }

        // bootloading action here
        private void buttonBootload_Click(object sender, EventArgs e)
        {

        }

        static bool hammerButtonPressed = false;

        private void buttonHammer_Click(object sender, EventArgs e)
        {
            if (hammerButtonPressed)
            {
                hammerButtonPressed = false;
                buttonHammer.UseVisualStyleBackColor = true;
                buttonHammer.Invalidate();
                return;
            }

            hammerButtonPressed = true;

            Stopwatch sw = new Stopwatch();
            UInt32 loopcount = 0;

            buttonHammer.BackColor = Color.Lime;

            // Set port steering byte to DMX Port 2
            byArray[0] = 2;

            // Initialize all other bytes to 0
            for (UInt32 n = 1; n <= 512; n++)
                byArray[n] = 0;

            while (true)
            {
                try
                {
                    if (SerialPort.IsOpen)
                    {
                        // Start with max DIM and end withh DIM = 0 for each iteration of the hammer test
                        for (Int32 dim = 255; dim >= 0; --dim)
                        {
                            // Return when user presses the hammer button a second time
                            if (!hammerButtonPressed)
                                return;

                            // Return when user closes the SerialPort
                            if (!buttonHammer.Enabled)
                            {
                                hammerButtonPressed = false;
                                buttonHammer.UseVisualStyleBackColor = true;
                                buttonHammer.Invalidate();
                                return;
                            }

                            // Only update the main DIM byte for each CLM (bytes 3, 15, and 27)
                            for (UInt32 slot = 3; slot <= 27; slot += 12)
                                byArray[slot] = (byte)dim;

                            // Output byte array to SerialPort
                            SerialPort.Write(byArray, 0, 513);

                            // Update log message
                            UpdateLogMessage("Loop count = " + ++loopcount);
                            listBoxSystemLog.SelectedIndex = listBoxSystemLog.Items.Count - 1;

                            // Optional sleep
                            if (dim == 0)
                                sleep(1000);
                            else
                                sleep(40);
                        }

                        // Wait while processing events (to allow user to press buttons and delay to view bytes 3, 15, and 27 on Swisson or break into firmware debug)
                        UInt32 milliSecondTimeout = 25;
                        sw.Restart();

                        do
                        {
                            // Return when user presses the hammer button a second time
                            if (!hammerButtonPressed)
                                return;

                            // Return when user closes the SerialPort
                            if (!buttonHammer.Enabled)
                            {
                                hammerButtonPressed = false;
                                buttonHammer.UseVisualStyleBackColor = true;
                                buttonHammer.Invalidate();
                                return;
                            }

                            Application.DoEvents();
                        }
                        while (sw.ElapsedMilliseconds < milliSecondTimeout);

                        sw.Stop();
                    }
                    else
                    {
                        // When user closes the SerialPort, return from the hammer test
                        hammerButtonPressed = false;
                        buttonHammer.UseVisualStyleBackColor = true;
                        buttonHammer.Invalidate();
                        return;
                    }
                }
                catch
                {
                    UpdateLogMessage("Something happened = " + loopcount);
                    hammerButtonPressed = false;
                    buttonHammer.UseVisualStyleBackColor = true;
                    buttonHammer.Invalidate();
                    return;
                }
            }
        }

        private void CH1UpDown_ValueChanged(object sender, EventArgs e)
        {
            trackBarCH1.Value = (int)CH1UpDown.Value;
            CH1UpDown.Update();
            trackBarCH1_Scroll(sender, e);
        }
        private void trackBarCH1_Scroll(object sender, EventArgs e)
        {
            try
            {
                if (SerialPort.IsOpen)
                {
                    z = 0;
                    byArray[z++] = (byte)CONSOLECOMMANDS.SLIDERCH1;
                    byArray[z++] = (byte)SLIDERLENGTH;
                    byArray[z++] = (byte)byAuditCode++;
                    byArray[z++] = (byte)READWRITE.WRITE;
                    byArray[z++] = (byte)ACKGOODRESPONSE;
                    byArray[z++] = (byte)(trackBarCH1.Value >> 8);
                    byArray[z++] = (byte)trackBarCH1.Value;
                    GenerateChecksum();
                    SerialPort.Write(byArray, 0, z);

                    // wait for the reponse
                    timerComRead.Enabled = true;

                    byForceSerial = byReadSerial = (byte)CONSOLECOMMANDS.SLIDERCH1;

                    labelStatusMessage.Text = "Sent CH1 = " + trackBarCH1.Value + " value message...waiting for reply";
                    UpdateLogMessage("Sent " + ToHex((byte)CONSOLECOMMANDS.SLIDERCH1) + " send CH1 " + trackBarCH1.Value + " value version message...waiting for reply...");

                    CH1UpDown.Value = trackBarCH1.Value;
                }
            }
            catch
            {
                labelStatusMessage.Text = "Something happened to the serial port during a WRITE, stopping sample timer...";
                UpdateLogMessage("Something happened to the serial port during a WRITE, stopping sample timer...");

                timerSerialPush.Enabled = false;
                timerComRead.Enabled = false;

                buttonOpenUSBPort.BackColor = Color.Yellow;
                buttonOpenUSBPort.Text = "Open USB";
                buttonComTest.BackColor = Color.LightGray;
            }
        }
        private void CH2UpDown_ValueChanged(object sender, EventArgs e)
        {
            trackBarCH2.Value = (int)CH2UpDown.Value;
            CH2UpDown.Update();
            trackBarCH2_Scroll(sender, e);
        }
        private void trackBarCH2_Scroll(object sender, EventArgs e)
        {
            try
            {
                if (SerialPort.IsOpen)
                {
                    z = 0;
                    byArray[z++] = (byte)CONSOLECOMMANDS.SLIDERCH2;
                    byArray[z++] = (byte)SLIDERLENGTH;
                    byArray[z++] = (byte)byAuditCode++;
                    byArray[z++] = (byte)READWRITE.WRITE;
                    byArray[z++] = (byte)ACKGOODRESPONSE;
                    byArray[z++] = (byte)(trackBarCH2.Value >> 8);
                    byArray[z++] = (byte)trackBarCH2.Value;
                    GenerateChecksum();
                    SerialPort.Write(byArray, 0, z);

                    // wait for the reponse
                    timerComRead.Enabled = true;

                    byForceSerial = byReadSerial = (byte)CONSOLECOMMANDS.SLIDERCH2;

                    labelStatusMessage.Text = "Sent CH2 = " + trackBarCH2.Value + " value message...waiting for reply";
                    UpdateLogMessage("Sent " + ToHex((byte)CONSOLECOMMANDS.SLIDERCH2) + " send CH2 " + trackBarCH2.Value + " value version message...waiting for reply...");

                    CH2UpDown.Value = trackBarCH2.Value;
                }
            }
            catch
            {
                labelStatusMessage.Text = "Something happened to the serial port during a WRITE, stopping sample timer...";
                UpdateLogMessage("Something happened to the serial port during a WRITE, stopping sample timer...");

                timerSerialPush.Enabled = false;
                timerComRead.Enabled = false;

                buttonOpenUSBPort.BackColor = Color.Yellow;
                buttonOpenUSBPort.Text = "Open USB";
                buttonComTest.BackColor = Color.LightGray;
            }
        }
        private void CH3UpDown_ValueChanged(object sender, EventArgs e)
        {
            trackBarCH3.Value = (int)CH3UpDown.Value;
            CH3UpDown.Update();
            trackBarCH3_Scroll(sender, e);
        }
        private void trackBarCH3_Scroll(object sender, EventArgs e)
        {
            try
            {
                if (SerialPort.IsOpen)
                {
                    z = 0;
                    byArray[z++] = (byte)CONSOLECOMMANDS.SLIDERCH3;
                    byArray[z++] = (byte)SLIDERLENGTH;
                    byArray[z++] = (byte)byAuditCode++;
                    byArray[z++] = (byte)READWRITE.WRITE;
                    byArray[z++] = (byte)ACKGOODRESPONSE;
                    byArray[z++] = (byte)(trackBarCH3.Value >> 8);
                    byArray[z++] = (byte)trackBarCH3.Value;
                    GenerateChecksum();
                    SerialPort.Write(byArray, 0, z);

                    // wait for the reponse
                    timerComRead.Enabled = true;

                    byForceSerial = byReadSerial = (byte)CONSOLECOMMANDS.SLIDERCH3;

                    labelStatusMessage.Text = "Sent CH3 = " + trackBarCH3.Value + " value message...waiting for reply";
                    UpdateLogMessage("Sent " + ToHex((byte)CONSOLECOMMANDS.SLIDERCH3) + " send CH3 " + trackBarCH3.Value + " value version message...waiting for reply...");

                    CH3UpDown.Value = trackBarCH3.Value;
                }
            }
            catch
            {
                labelStatusMessage.Text = "Something happened to the serial port during a WRITE, stopping sample timer...";
                UpdateLogMessage("Something happened to the serial port during a WRITE, stopping sample timer...");

                timerSerialPush.Enabled = false;
                timerComRead.Enabled = false;

                buttonOpenUSBPort.BackColor = Color.Yellow;
                buttonOpenUSBPort.Text = "Open USB";
                buttonComTest.BackColor = Color.LightGray;
            }
        }
        private void CH4UpDown_ValueChanged(object sender, EventArgs e)
        {
            trackBarCH4.Value = (int)CH4UpDown.Value;
            CH4UpDown.Update();
            trackBarCH4_Scroll(sender, e);
        }
        private void trackBarCH4_Scroll(object sender, EventArgs e)
        {
            try
            {
                if (SerialPort.IsOpen)
                {
                    z = 0;
                    byArray[z++] = (byte)CONSOLECOMMANDS.SLIDERCH4;
                    byArray[z++] = (byte)SLIDERLENGTH;
                    byArray[z++] = (byte)byAuditCode++;
                    byArray[z++] = (byte)READWRITE.WRITE;
                    byArray[z++] = (byte)ACKGOODRESPONSE;
                    byArray[z++] = (byte)(trackBarCH4.Value >> 8);
                    byArray[z++] = (byte)trackBarCH4.Value;
                    GenerateChecksum();
                    SerialPort.Write(byArray, 0, z);

                    // wait for the reponse
                    timerComRead.Enabled = true;

                    byForceSerial = byReadSerial = (byte)CONSOLECOMMANDS.SLIDERCH4;

                    labelStatusMessage.Text = "Sent CH4 = " + trackBarCH4.Value + " value message...waiting for reply";
                    UpdateLogMessage("Sent " + ToHex((byte)CONSOLECOMMANDS.SLIDERCH4) + " send CH4 " + trackBarCH4.Value + " value version message...waiting for reply...");

                    CH4UpDown.Value = trackBarCH4.Value;
                }
            }
            catch
            {
                labelStatusMessage.Text = "Something happened to the serial port during a WRITE, stopping sample timer...";
                UpdateLogMessage("Something happened to the serial port during a WRITE, stopping sample timer...");

                timerSerialPush.Enabled = false;
                timerComRead.Enabled = false;

                buttonOpenUSBPort.BackColor = Color.Yellow;
                buttonOpenUSBPort.Text = "Open USB";
                buttonComTest.BackColor = Color.LightGray;
            }
        }
        private void CH5UpDown_ValueChanged(object sender, EventArgs e)
        {
            trackBarCH5.Value = (int)CH5UpDown.Value;
            CH5UpDown.Update();
            trackBarCH5_Scroll(sender, e);
        }
        private void trackBarCH5_Scroll(object sender, EventArgs e)
        {
            try
            {
                if (SerialPort.IsOpen)
                {
                    z = 0;
                    byArray[z++] = (byte)CONSOLECOMMANDS.SLIDERCH5;
                    byArray[z++] = (byte)SLIDERLENGTH;
                    byArray[z++] = (byte)byAuditCode++;
                    byArray[z++] = (byte)READWRITE.WRITE;
                    byArray[z++] = (byte)ACKGOODRESPONSE;
                    byArray[z++] = (byte)(trackBarCH5.Value >> 8);
                    byArray[z++] = (byte)trackBarCH5.Value;
                    GenerateChecksum();
                    SerialPort.Write(byArray, 0, z);

                    // wait for the reponse
                    timerComRead.Enabled = true;

                    byForceSerial = byReadSerial = (byte)CONSOLECOMMANDS.SLIDERCH5;

                    labelStatusMessage.Text = "Sent CH5 = " + trackBarCH5.Value + " value message...waiting for reply";
                    UpdateLogMessage("Sent " + ToHex((byte)CONSOLECOMMANDS.SLIDERCH5) + " send CH5 " + trackBarCH5.Value + " value version message...waiting for reply...");

                    CH5UpDown.Value = trackBarCH5.Value;
                }
            }
            catch
            {
                labelStatusMessage.Text = "Something happened to the serial port during a WRITE, stopping sample timer...";
                UpdateLogMessage("Something happened to the serial port during a WRITE, stopping sample timer...");

                timerSerialPush.Enabled = false;
                timerComRead.Enabled = false;

                buttonOpenUSBPort.BackColor = Color.Yellow;
                buttonOpenUSBPort.Text = "Open USB";
                buttonComTest.BackColor = Color.LightGray;
            }
        }
        private void CH6UpDown_ValueChanged(object sender, EventArgs e)
        {
            trackBarCH6.Value = (int)CH6UpDown.Value;
            CH6UpDown.Update();
            trackBarCH6_Scroll(sender, e);
        }
        private void trackBarCH6_Scroll(object sender, EventArgs e)
        {
            try
            {
                if (SerialPort.IsOpen)
                {
                    z = 0;
                    byArray[z++] = (byte)CONSOLECOMMANDS.SLIDERCH6;
                    byArray[z++] = (byte)SLIDERLENGTH;
                    byArray[z++] = (byte)byAuditCode++;
                    byArray[z++] = (byte)READWRITE.WRITE;
                    byArray[z++] = (byte)ACKGOODRESPONSE;
                    byArray[z++] = (byte)(trackBarCH6.Value >> 8);
                    byArray[z++] = (byte)trackBarCH6.Value;
                    GenerateChecksum();
                    SerialPort.Write(byArray, 0, z);

                    // wait for the reponse
                    timerComRead.Enabled = true;

                    byForceSerial = byReadSerial = (byte)CONSOLECOMMANDS.SLIDERCH6;

                    labelStatusMessage.Text = "Sent CH6 = " + trackBarCH6.Value + " value message...waiting for reply";
                    UpdateLogMessage("Sent " + ToHex((byte)CONSOLECOMMANDS.SLIDERCH1) + " send CH6 " + trackBarCH6.Value + " value version message...waiting for reply...");

                    CH6UpDown.Value = trackBarCH6.Value;
                }
            }
            catch
            {
                labelStatusMessage.Text = "Something happened to the serial port during a WRITE, stopping sample timer...";
                UpdateLogMessage("Something happened to the serial port during a WRITE, stopping sample timer...");

                timerSerialPush.Enabled = false;
                timerComRead.Enabled = false;

                buttonOpenUSBPort.BackColor = Color.Yellow;
                buttonOpenUSBPort.Text = "Open USB";
                buttonComTest.BackColor = Color.LightGray;
            }
        }

        static byte byState = 0;
        private void timerGetInitialSettings_Tick(object sender, EventArgs e)
        {
            if (bTickAlive == true && bWeGotData == true)
            {
                bTickAlive = false;
                bWeGotData = false;

                switch (byState)
                {
                    case 0:
                        {
                            trackBarCH1.Enabled = true;
                            comboBoxReadWrite.SelectedIndex = (byte)READWRITE.READ;
                            byForceSerial = byReadSerial = (byte)CONSOLECOMMANDS.SLIDERCH1;
                            timerSerialPush.Enabled = true;
                            byState++;
                        }
                        break;

                    case 1:
                        {
                            trackBarCH2.Enabled = true;
                            comboBoxReadWrite.SelectedIndex = (byte)READWRITE.READ;
                            byForceSerial = byReadSerial = (byte)CONSOLECOMMANDS.SLIDERCH2;
                            timerSerialPush.Enabled = true;
                            byState++;
                        }
                        break;

                    case 2:
                        {
                            trackBarCH3.Enabled = true;
                            comboBoxReadWrite.SelectedIndex = (byte)READWRITE.READ;
                            byForceSerial = byReadSerial = (byte)CONSOLECOMMANDS.SLIDERCH3;
                            timerSerialPush.Enabled = true;
                            byState++;
                        }
                        break;

                    case 3:
                        {
                            trackBarCH4.Enabled = true;
                            comboBoxReadWrite.SelectedIndex = (byte)READWRITE.READ;
                            byForceSerial = byReadSerial = (byte)CONSOLECOMMANDS.SLIDERCH4;
                            timerSerialPush.Enabled = true;
                            byState++;
                        }
                        break;

                    case 4:
                        {
                            trackBarCH5.Enabled = true;
                            comboBoxReadWrite.SelectedIndex = (byte)READWRITE.READ;
                            byForceSerial = byReadSerial = (byte)CONSOLECOMMANDS.SLIDERCH5;
                            timerSerialPush.Enabled = true;
                            byState++;
                        }
                        break;

                    case 5:
                        {
                            trackBarCH6.Enabled = true;
                            comboBoxReadWrite.SelectedIndex = (byte)READWRITE.READ;
                            byForceSerial = byReadSerial = (byte)CONSOLECOMMANDS.SLIDERCH6;
                            timerSerialPush.Enabled = true;
                            byState++;
                        }
                        break;

                    default:
                        timerGetInitialSettings.Enabled = false;
                        break;
                }
            }

            this.Update();
        }

        private void buttonAllOff_Click(object sender, EventArgs e)
        {
            CH1UpDown.Value = trackBarCH1.Value = 0;
            CH2UpDown.Value = trackBarCH2.Value = 0;
            CH3UpDown.Value = trackBarCH3.Value = 0;
            CH4UpDown.Value = trackBarCH4.Value = 0;
            CH5UpDown.Value = trackBarCH5.Value = 0;
            CH6UpDown.Value = trackBarCH6.Value = 0;

            labelStatusMessage.Text = "All intensity set to OFF sent to the driver...";
            UpdateLogMessage("All intensity set to OFF sent to the driver...");
        }
    }
}

