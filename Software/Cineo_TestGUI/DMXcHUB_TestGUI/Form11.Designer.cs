namespace DMXcHUB_TestGUI
{
    partial class FormMain
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

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.SerialPort = new System.IO.Ports.SerialPort(this.components);
            this.groupBox9 = new System.Windows.Forms.GroupBox();
            this.buttonHammer = new System.Windows.Forms.Button();
            this.comboBoxPortNumber = new System.Windows.Forms.ComboBox();
            this.buttonComTest = new System.Windows.Forms.Button();
            this.label3 = new System.Windows.Forms.Label();
            this.buttonOpenUSBPort = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.labelStatusMessage = new System.Windows.Forms.Label();
            this.comboBoxCommand = new System.Windows.Forms.ComboBox();
            this.label2 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.comboBoxParameters = new System.Windows.Forms.ComboBox();
            this.label5 = new System.Windows.Forms.Label();
            this.listBoxSystemLog = new System.Windows.Forms.ListBox();
            this.buttonSend = new System.Windows.Forms.Button();
            this.timerComRead = new System.Windows.Forms.Timer(this.components);
            this.timerSerialPush = new System.Windows.Forms.Timer(this.components);
            this.comboBoxReadWrite = new System.Windows.Forms.ComboBox();
            this.label6 = new System.Windows.Forms.Label();
            this.buttonClearLog = new System.Windows.Forms.Button();
            this.buttonBootload = new System.Windows.Forms.Button();
            this.ProgressBar = new System.Windows.Forms.ProgressBar();
            this.textBoxCommandString = new System.Windows.Forms.TextBox();
            this.trackBarCH1 = new System.Windows.Forms.TrackBar();
            this.label7 = new System.Windows.Forms.Label();
            this.label8 = new System.Windows.Forms.Label();
            this.trackBarCH2 = new System.Windows.Forms.TrackBar();
            this.label9 = new System.Windows.Forms.Label();
            this.trackBarCH3 = new System.Windows.Forms.TrackBar();
            this.label10 = new System.Windows.Forms.Label();
            this.trackBarCH4 = new System.Windows.Forms.TrackBar();
            this.label11 = new System.Windows.Forms.Label();
            this.trackBarCH5 = new System.Windows.Forms.TrackBar();
            this.label12 = new System.Windows.Forms.Label();
            this.trackBarCH6 = new System.Windows.Forms.TrackBar();
            this.timerGetInitialSettings = new System.Windows.Forms.Timer(this.components);
            this.CH1UpDown = new System.Windows.Forms.NumericUpDown();
            this.CH2UpDown = new System.Windows.Forms.NumericUpDown();
            this.CH3UpDown = new System.Windows.Forms.NumericUpDown();
            this.CH4UpDown = new System.Windows.Forms.NumericUpDown();
            this.CH5UpDown = new System.Windows.Forms.NumericUpDown();
            this.CH6UpDown = new System.Windows.Forms.NumericUpDown();
            this.label13 = new System.Windows.Forms.Label();
            this.buttonAllOff = new System.Windows.Forms.Button();
            this.label14 = new System.Windows.Forms.Label();
            this.textBoxI2CAddress = new System.Windows.Forms.TextBox();
            this.groupBox9.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.trackBarCH1)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.trackBarCH2)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.trackBarCH3)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.trackBarCH4)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.trackBarCH5)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.trackBarCH6)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.CH1UpDown)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.CH2UpDown)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.CH3UpDown)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.CH4UpDown)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.CH5UpDown)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.CH6UpDown)).BeginInit();
            this.SuspendLayout();
            // 
            // SerialPort
            // 
            this.SerialPort.BaudRate = 921600;
            this.SerialPort.ParityReplace = ((byte)(255));
            this.SerialPort.ReadBufferSize = 64;
            this.SerialPort.WriteBufferSize = 64;
            // 
            // groupBox9
            // 
            this.groupBox9.Controls.Add(this.buttonHammer);
            this.groupBox9.Controls.Add(this.comboBoxPortNumber);
            this.groupBox9.Controls.Add(this.buttonComTest);
            this.groupBox9.Controls.Add(this.label3);
            this.groupBox9.Controls.Add(this.buttonOpenUSBPort);
            this.groupBox9.Location = new System.Drawing.Point(15, 12);
            this.groupBox9.Name = "groupBox9";
            this.groupBox9.Size = new System.Drawing.Size(138, 110);
            this.groupBox9.TabIndex = 38;
            this.groupBox9.TabStop = false;
            this.groupBox9.Text = "Comm";
            // 
            // buttonHammer
            // 
            this.buttonHammer.Enabled = false;
            this.buttonHammer.Location = new System.Drawing.Point(73, 74);
            this.buttonHammer.Name = "buttonHammer";
            this.buttonHammer.Size = new System.Drawing.Size(59, 23);
            this.buttonHammer.TabIndex = 41;
            this.buttonHammer.Text = "Hammer";
            this.buttonHammer.UseVisualStyleBackColor = true;
            this.buttonHammer.Click += new System.EventHandler(this.buttonHammer_Click);
            // 
            // comboBoxPortNumber
            // 
            this.comboBoxPortNumber.FormattingEnabled = true;
            this.comboBoxPortNumber.Location = new System.Drawing.Point(31, 19);
            this.comboBoxPortNumber.Name = "comboBoxPortNumber";
            this.comboBoxPortNumber.Size = new System.Drawing.Size(101, 21);
            this.comboBoxPortNumber.TabIndex = 40;
            // 
            // buttonComTest
            // 
            this.buttonComTest.Enabled = false;
            this.buttonComTest.Location = new System.Drawing.Point(7, 74);
            this.buttonComTest.Name = "buttonComTest";
            this.buttonComTest.Size = new System.Drawing.Size(60, 23);
            this.buttonComTest.TabIndex = 39;
            this.buttonComTest.Text = "ComTest";
            this.buttonComTest.UseVisualStyleBackColor = true;
            this.buttonComTest.Click += new System.EventHandler(this.buttonComTest_Click);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(4, 22);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(26, 13);
            this.label3.TabIndex = 34;
            this.label3.Text = "Port";
            // 
            // buttonOpenUSBPort
            // 
            this.buttonOpenUSBPort.BackColor = System.Drawing.Color.Yellow;
            this.buttonOpenUSBPort.Location = new System.Drawing.Point(45, 45);
            this.buttonOpenUSBPort.Name = "buttonOpenUSBPort";
            this.buttonOpenUSBPort.Size = new System.Drawing.Size(75, 23);
            this.buttonOpenUSBPort.TabIndex = 36;
            this.buttonOpenUSBPort.Text = "Open USB";
            this.buttonOpenUSBPort.UseVisualStyleBackColor = false;
            this.buttonOpenUSBPort.Click += new System.EventHandler(this.buttonOpenUSBPort_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label1.ForeColor = System.Drawing.Color.Red;
            this.label1.Location = new System.Drawing.Point(12, 519);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(75, 13);
            this.label1.TabIndex = 42;
            this.label1.Text = "Last Status:";
            // 
            // labelStatusMessage
            // 
            this.labelStatusMessage.AutoSize = true;
            this.labelStatusMessage.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.labelStatusMessage.Location = new System.Drawing.Point(93, 519);
            this.labelStatusMessage.Name = "labelStatusMessage";
            this.labelStatusMessage.Size = new System.Drawing.Size(282, 13);
            this.labelStatusMessage.TabIndex = 41;
            this.labelStatusMessage.Text = "Software load OK...awaiting command from user.";
            // 
            // comboBoxCommand
            // 
            this.comboBoxCommand.Enabled = false;
            this.comboBoxCommand.FormattingEnabled = true;
            this.comboBoxCommand.Items.AddRange(new object[] {
            "Ping",
            "ACK ON-OFF",
            "Set DMX Main Mode (mode selection below)",
            "Set number of DMX data bytes in/out of port",
            "Set DMX Univere data source (mode selection below)",
            "Set DMX port input priority (mode selection below)",
            "Load DMX Personality (RDM viewable) number to \"write to\" and set",
            "Load RDM Personality textual description",
            "Load RDM Personality slot count",
            "Initiate the save of the Personality loaded (save the last three items to memory)" +
                "",
            "Set the RDM major and minor versions reported",
            "Set the RDM product ID (text)",
            "Set the RDM UUID (6 bytes)",
            "Select a DMX Personality previously stored",
            "Set the DMX Start Address",
            "Write an audit code to DMXcHUB memory",
            "Manage the audit code from DMXcHUB memory",
            "Perform a DMXcHUB soft reset"});
            this.comboBoxCommand.Location = new System.Drawing.Point(218, 18);
            this.comboBoxCommand.Name = "comboBoxCommand";
            this.comboBoxCommand.Size = new System.Drawing.Size(526, 21);
            this.comboBoxCommand.TabIndex = 43;
            this.comboBoxCommand.Text = "Select command here...";
            this.comboBoxCommand.SelectedIndexChanged += new System.EventHandler(this.comboBoxCommand_SelectedIndexChanged);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(158, 22);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(54, 13);
            this.label2.TabIndex = 44;
            this.label2.Text = "Command";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(182, 58);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(30, 13);
            this.label4.TabIndex = 45;
            this.label4.Text = "Data";
            // 
            // comboBoxParameters
            // 
            this.comboBoxParameters.Enabled = false;
            this.comboBoxParameters.FormattingEnabled = true;
            this.comboBoxParameters.Location = new System.Drawing.Point(218, 54);
            this.comboBoxParameters.Name = "comboBoxParameters";
            this.comboBoxParameters.Size = new System.Drawing.Size(354, 21);
            this.comboBoxParameters.TabIndex = 46;
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(578, 58);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(28, 13);
            this.label5.TabIndex = 47;
            this.label5.Text = "Text";
            // 
            // listBoxSystemLog
            // 
            this.listBoxSystemLog.HorizontalScrollbar = true;
            this.listBoxSystemLog.Location = new System.Drawing.Point(15, 361);
            this.listBoxSystemLog.Name = "listBoxSystemLog";
            this.listBoxSystemLog.Size = new System.Drawing.Size(926, 134);
            this.listBoxSystemLog.TabIndex = 49;
            // 
            // buttonSend
            // 
            this.buttonSend.Enabled = false;
            this.buttonSend.Location = new System.Drawing.Point(857, 92);
            this.buttonSend.Name = "buttonSend";
            this.buttonSend.Size = new System.Drawing.Size(75, 23);
            this.buttonSend.TabIndex = 51;
            this.buttonSend.Text = "Send";
            this.buttonSend.UseVisualStyleBackColor = true;
            this.buttonSend.Click += new System.EventHandler(this.buttonSend_Click);
            // 
            // timerComRead
            // 
            this.timerComRead.Interval = 1;
            this.timerComRead.Tick += new System.EventHandler(this.timerComRead_Tick);
            // 
            // timerSerialPush
            // 
            this.timerSerialPush.Interval = 1;
            this.timerSerialPush.Tick += new System.EventHandler(this.timerSerialPush_Tick);
            // 
            // comboBoxReadWrite
            // 
            this.comboBoxReadWrite.Enabled = false;
            this.comboBoxReadWrite.FormattingEnabled = true;
            this.comboBoxReadWrite.Items.AddRange(new object[] {
            "0 - Write",
            "1 - Read"});
            this.comboBoxReadWrite.Location = new System.Drawing.Point(857, 18);
            this.comboBoxReadWrite.Name = "comboBoxReadWrite";
            this.comboBoxReadWrite.Size = new System.Drawing.Size(75, 21);
            this.comboBoxReadWrite.TabIndex = 52;
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(826, 22);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(31, 13);
            this.label6.TabIndex = 54;
            this.label6.Text = "W/R";
            // 
            // buttonClearLog
            // 
            this.buttonClearLog.Location = new System.Drawing.Point(849, 507);
            this.buttonClearLog.Name = "buttonClearLog";
            this.buttonClearLog.Size = new System.Drawing.Size(92, 42);
            this.buttonClearLog.TabIndex = 56;
            this.buttonClearLog.Text = "ClearLog";
            this.buttonClearLog.UseVisualStyleBackColor = true;
            this.buttonClearLog.Click += new System.EventHandler(this.ButtonClearLog_Click);
            // 
            // buttonBootload
            // 
            this.buttonBootload.Enabled = false;
            this.buttonBootload.Location = new System.Drawing.Point(762, 509);
            this.buttonBootload.Name = "buttonBootload";
            this.buttonBootload.Size = new System.Drawing.Size(75, 23);
            this.buttonBootload.TabIndex = 60;
            this.buttonBootload.Text = "Bootload";
            this.buttonBootload.UseVisualStyleBackColor = true;
            this.buttonBootload.Visible = false;
            this.buttonBootload.Click += new System.EventHandler(this.buttonBootload_Click);
            // 
            // ProgressBar
            // 
            this.ProgressBar.BackColor = System.Drawing.Color.Yellow;
            this.ProgressBar.Location = new System.Drawing.Point(13, 537);
            this.ProgressBar.Name = "ProgressBar";
            this.ProgressBar.Size = new System.Drawing.Size(824, 12);
            this.ProgressBar.TabIndex = 61;
            // 
            // textBoxCommandString
            // 
            this.textBoxCommandString.Enabled = false;
            this.textBoxCommandString.Location = new System.Drawing.Point(612, 54);
            this.textBoxCommandString.Name = "textBoxCommandString";
            this.textBoxCommandString.Size = new System.Drawing.Size(320, 20);
            this.textBoxCommandString.TabIndex = 50;
            // 
            // trackBarCH1
            // 
            this.trackBarCH1.Cursor = System.Windows.Forms.Cursors.Hand;
            this.trackBarCH1.Enabled = false;
            this.trackBarCH1.LargeChange = 1;
            this.trackBarCH1.Location = new System.Drawing.Point(47, 128);
            this.trackBarCH1.Maximum = 65535;
            this.trackBarCH1.Name = "trackBarCH1";
            this.trackBarCH1.Size = new System.Drawing.Size(820, 45);
            this.trackBarCH1.TabIndex = 63;
            this.trackBarCH1.TickStyle = System.Windows.Forms.TickStyle.Both;
            this.trackBarCH1.Scroll += new System.EventHandler(this.trackBarCH1_Scroll);
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label7.Location = new System.Drawing.Point(11, 133);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(34, 16);
            this.label7.TabIndex = 64;
            this.label7.Text = "Ch1";
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label8.Location = new System.Drawing.Point(11, 168);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(34, 16);
            this.label8.TabIndex = 66;
            this.label8.Text = "Ch2";
            // 
            // trackBarCH2
            // 
            this.trackBarCH2.Enabled = false;
            this.trackBarCH2.LargeChange = 1;
            this.trackBarCH2.Location = new System.Drawing.Point(47, 163);
            this.trackBarCH2.Maximum = 65535;
            this.trackBarCH2.Name = "trackBarCH2";
            this.trackBarCH2.Size = new System.Drawing.Size(820, 45);
            this.trackBarCH2.TabIndex = 65;
            this.trackBarCH2.TickStyle = System.Windows.Forms.TickStyle.Both;
            this.trackBarCH2.Scroll += new System.EventHandler(this.trackBarCH2_Scroll);
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label9.Location = new System.Drawing.Point(11, 200);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(34, 16);
            this.label9.TabIndex = 68;
            this.label9.Text = "Ch3";
            // 
            // trackBarCH3
            // 
            this.trackBarCH3.Enabled = false;
            this.trackBarCH3.LargeChange = 1;
            this.trackBarCH3.Location = new System.Drawing.Point(47, 200);
            this.trackBarCH3.Maximum = 65535;
            this.trackBarCH3.Name = "trackBarCH3";
            this.trackBarCH3.Size = new System.Drawing.Size(820, 45);
            this.trackBarCH3.TabIndex = 67;
            this.trackBarCH3.TickStyle = System.Windows.Forms.TickStyle.Both;
            this.trackBarCH3.Scroll += new System.EventHandler(this.trackBarCH3_Scroll);
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label10.Location = new System.Drawing.Point(11, 238);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(34, 16);
            this.label10.TabIndex = 70;
            this.label10.Text = "Ch4";
            // 
            // trackBarCH4
            // 
            this.trackBarCH4.Enabled = false;
            this.trackBarCH4.LargeChange = 1;
            this.trackBarCH4.Location = new System.Drawing.Point(47, 238);
            this.trackBarCH4.Maximum = 65535;
            this.trackBarCH4.Name = "trackBarCH4";
            this.trackBarCH4.Size = new System.Drawing.Size(820, 45);
            this.trackBarCH4.TabIndex = 69;
            this.trackBarCH4.TickStyle = System.Windows.Forms.TickStyle.Both;
            this.trackBarCH4.Scroll += new System.EventHandler(this.trackBarCH4_Scroll);
            // 
            // label11
            // 
            this.label11.AutoSize = true;
            this.label11.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label11.Location = new System.Drawing.Point(11, 274);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(34, 16);
            this.label11.TabIndex = 72;
            this.label11.Text = "Ch5";
            // 
            // trackBarCH5
            // 
            this.trackBarCH5.Enabled = false;
            this.trackBarCH5.LargeChange = 1;
            this.trackBarCH5.Location = new System.Drawing.Point(47, 274);
            this.trackBarCH5.Maximum = 65535;
            this.trackBarCH5.Name = "trackBarCH5";
            this.trackBarCH5.Size = new System.Drawing.Size(820, 45);
            this.trackBarCH5.TabIndex = 71;
            this.trackBarCH5.TickStyle = System.Windows.Forms.TickStyle.Both;
            this.trackBarCH5.Scroll += new System.EventHandler(this.trackBarCH5_Scroll);
            // 
            // label12
            // 
            this.label12.AutoSize = true;
            this.label12.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label12.Location = new System.Drawing.Point(11, 313);
            this.label12.Name = "label12";
            this.label12.Size = new System.Drawing.Size(34, 16);
            this.label12.TabIndex = 74;
            this.label12.Text = "Ch6";
            // 
            // trackBarCH6
            // 
            this.trackBarCH6.Enabled = false;
            this.trackBarCH6.LargeChange = 1;
            this.trackBarCH6.Location = new System.Drawing.Point(47, 313);
            this.trackBarCH6.Maximum = 65535;
            this.trackBarCH6.Name = "trackBarCH6";
            this.trackBarCH6.Size = new System.Drawing.Size(820, 45);
            this.trackBarCH6.TabIndex = 73;
            this.trackBarCH6.TickStyle = System.Windows.Forms.TickStyle.Both;
            this.trackBarCH6.Scroll += new System.EventHandler(this.trackBarCH6_Scroll);
            // 
            // timerGetInitialSettings
            // 
            this.timerGetInitialSettings.Interval = 5;
            this.timerGetInitialSettings.Tick += new System.EventHandler(this.timerGetInitialSettings_Tick);
            // 
            // CH1UpDown
            // 
            this.CH1UpDown.Location = new System.Drawing.Point(871, 134);
            this.CH1UpDown.Maximum = new decimal(new int[] {
            65535,
            0,
            0,
            0});
            this.CH1UpDown.Name = "CH1UpDown";
            this.CH1UpDown.Size = new System.Drawing.Size(61, 20);
            this.CH1UpDown.TabIndex = 75;
            this.CH1UpDown.Value = new decimal(new int[] {
            65535,
            0,
            0,
            0});
            this.CH1UpDown.ValueChanged += new System.EventHandler(this.CH1UpDown_ValueChanged);
            // 
            // CH2UpDown
            // 
            this.CH2UpDown.Location = new System.Drawing.Point(871, 168);
            this.CH2UpDown.Maximum = new decimal(new int[] {
            65535,
            0,
            0,
            0});
            this.CH2UpDown.Name = "CH2UpDown";
            this.CH2UpDown.Size = new System.Drawing.Size(61, 20);
            this.CH2UpDown.TabIndex = 76;
            this.CH2UpDown.Value = new decimal(new int[] {
            65535,
            0,
            0,
            0});
            this.CH2UpDown.ValueChanged += new System.EventHandler(this.CH2UpDown_ValueChanged);
            // 
            // CH3UpDown
            // 
            this.CH3UpDown.Location = new System.Drawing.Point(871, 200);
            this.CH3UpDown.Maximum = new decimal(new int[] {
            65535,
            0,
            0,
            0});
            this.CH3UpDown.Name = "CH3UpDown";
            this.CH3UpDown.Size = new System.Drawing.Size(61, 20);
            this.CH3UpDown.TabIndex = 77;
            this.CH3UpDown.Value = new decimal(new int[] {
            65535,
            0,
            0,
            0});
            this.CH3UpDown.ValueChanged += new System.EventHandler(this.CH3UpDown_ValueChanged);
            // 
            // CH4UpDown
            // 
            this.CH4UpDown.Location = new System.Drawing.Point(871, 238);
            this.CH4UpDown.Maximum = new decimal(new int[] {
            65535,
            0,
            0,
            0});
            this.CH4UpDown.Name = "CH4UpDown";
            this.CH4UpDown.Size = new System.Drawing.Size(61, 20);
            this.CH4UpDown.TabIndex = 78;
            this.CH4UpDown.Value = new decimal(new int[] {
            65535,
            0,
            0,
            0});
            this.CH4UpDown.ValueChanged += new System.EventHandler(this.CH4UpDown_ValueChanged);
            // 
            // CH5UpDown
            // 
            this.CH5UpDown.Location = new System.Drawing.Point(873, 274);
            this.CH5UpDown.Maximum = new decimal(new int[] {
            65535,
            0,
            0,
            0});
            this.CH5UpDown.Name = "CH5UpDown";
            this.CH5UpDown.Size = new System.Drawing.Size(61, 20);
            this.CH5UpDown.TabIndex = 79;
            this.CH5UpDown.Value = new decimal(new int[] {
            65535,
            0,
            0,
            0});
            this.CH5UpDown.ValueChanged += new System.EventHandler(this.CH5UpDown_ValueChanged);
            // 
            // CH6UpDown
            // 
            this.CH6UpDown.Location = new System.Drawing.Point(873, 313);
            this.CH6UpDown.Maximum = new decimal(new int[] {
            65535,
            0,
            0,
            0});
            this.CH6UpDown.Name = "CH6UpDown";
            this.CH6UpDown.Size = new System.Drawing.Size(61, 20);
            this.CH6UpDown.TabIndex = 80;
            this.CH6UpDown.Value = new decimal(new int[] {
            65535,
            0,
            0,
            0});
            this.CH6UpDown.ValueChanged += new System.EventHandler(this.CH6UpDown_ValueChanged);
            // 
            // label13
            // 
            this.label13.AutoSize = true;
            this.label13.Font = new System.Drawing.Font("Microsoft Sans Serif", 14.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label13.Location = new System.Drawing.Point(269, 106);
            this.label13.Name = "label13";
            this.label13.Size = new System.Drawing.Size(373, 24);
            this.label13.TabIndex = 81;
            this.label13.Text = "Channel Intensity Control - 16 Bit Value";
            // 
            // buttonAllOff
            // 
            this.buttonAllOff.Enabled = false;
            this.buttonAllOff.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.buttonAllOff.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(192)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))));
            this.buttonAllOff.Location = new System.Drawing.Point(176, 109);
            this.buttonAllOff.Name = "buttonAllOff";
            this.buttonAllOff.Size = new System.Drawing.Size(75, 23);
            this.buttonAllOff.TabIndex = 82;
            this.buttonAllOff.Text = "All Off";
            this.buttonAllOff.UseVisualStyleBackColor = true;
            this.buttonAllOff.Click += new System.EventHandler(this.buttonAllOff_Click);
            // 
            // label14
            // 
            this.label14.AutoSize = true;
            this.label14.Location = new System.Drawing.Point(704, 97);
            this.label14.Name = "label14";
            this.label14.Size = new System.Drawing.Size(64, 13);
            this.label14.TabIndex = 83;
            this.label14.Text = "I2C Address";
            // 
            // textBoxI2CAddress
            // 
            this.textBoxI2CAddress.Enabled = false;
            this.textBoxI2CAddress.Location = new System.Drawing.Point(774, 94);
            this.textBoxI2CAddress.Name = "textBoxI2CAddress";
            this.textBoxI2CAddress.Size = new System.Drawing.Size(46, 20);
            this.textBoxI2CAddress.TabIndex = 84;
            // 
            // FormMain
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.AutoScroll = true;
            this.BackColor = System.Drawing.SystemColors.Info;
            this.ClientSize = new System.Drawing.Size(953, 568);
            this.Controls.Add(this.textBoxI2CAddress);
            this.Controls.Add(this.label14);
            this.Controls.Add(this.buttonAllOff);
            this.Controls.Add(this.label13);
            this.Controls.Add(this.CH6UpDown);
            this.Controls.Add(this.CH5UpDown);
            this.Controls.Add(this.CH4UpDown);
            this.Controls.Add(this.CH3UpDown);
            this.Controls.Add(this.CH2UpDown);
            this.Controls.Add(this.CH1UpDown);
            this.Controls.Add(this.label12);
            this.Controls.Add(this.trackBarCH6);
            this.Controls.Add(this.label11);
            this.Controls.Add(this.trackBarCH5);
            this.Controls.Add(this.label10);
            this.Controls.Add(this.trackBarCH4);
            this.Controls.Add(this.label9);
            this.Controls.Add(this.trackBarCH3);
            this.Controls.Add(this.label8);
            this.Controls.Add(this.trackBarCH2);
            this.Controls.Add(this.label7);
            this.Controls.Add(this.trackBarCH1);
            this.Controls.Add(this.ProgressBar);
            this.Controls.Add(this.buttonBootload);
            this.Controls.Add(this.buttonClearLog);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.comboBoxReadWrite);
            this.Controls.Add(this.buttonSend);
            this.Controls.Add(this.textBoxCommandString);
            this.Controls.Add(this.listBoxSystemLog);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.comboBoxParameters);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.comboBoxCommand);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.labelStatusMessage);
            this.Controls.Add(this.groupBox9);
            this.MaximizeBox = false;
            this.MinimumSize = new System.Drawing.Size(785, 467);
            this.Name = "FormMain";
            this.Text = "Cineo 6-Channel DMX and I2C Driver Test GUI 1.00B0";
            this.groupBox9.ResumeLayout(false);
            this.groupBox9.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.trackBarCH1)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.trackBarCH2)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.trackBarCH3)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.trackBarCH4)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.trackBarCH5)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.trackBarCH6)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.CH1UpDown)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.CH2UpDown)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.CH3UpDown)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.CH4UpDown)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.CH5UpDown)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.CH6UpDown)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        public System.IO.Ports.SerialPort SerialPort;
        private System.Windows.Forms.GroupBox groupBox9;
        private System.Windows.Forms.ComboBox comboBoxPortNumber;
        private System.Windows.Forms.Button buttonComTest;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Button buttonOpenUSBPort;
        private System.Windows.Forms.Label label1;
        public System.Windows.Forms.Label labelStatusMessage;
        private System.Windows.Forms.ComboBox comboBoxCommand;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.ComboBox comboBoxParameters;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.ListBox listBoxSystemLog;
        private System.Windows.Forms.Button buttonSend;
        private System.Windows.Forms.Timer timerComRead;
        private System.Windows.Forms.ComboBox comboBoxReadWrite;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Button buttonClearLog;
        private System.Windows.Forms.Button buttonBootload;
        private System.Windows.Forms.ProgressBar ProgressBar;
        private System.Windows.Forms.Button buttonHammer;
        private System.Windows.Forms.TextBox textBoxCommandString;
        private System.Windows.Forms.TrackBar trackBarCH1;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.TrackBar trackBarCH2;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.TrackBar trackBarCH3;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.TrackBar trackBarCH4;
        private System.Windows.Forms.Label label11;
        private System.Windows.Forms.TrackBar trackBarCH5;
        private System.Windows.Forms.Label label12;
        private System.Windows.Forms.TrackBar trackBarCH6;
        private System.Windows.Forms.Timer timerSerialPush;
        private System.Windows.Forms.Timer timerGetInitialSettings;
        private System.Windows.Forms.NumericUpDown CH1UpDown;
        private System.Windows.Forms.NumericUpDown CH2UpDown;
        private System.Windows.Forms.NumericUpDown CH3UpDown;
        private System.Windows.Forms.NumericUpDown CH4UpDown;
        private System.Windows.Forms.NumericUpDown CH5UpDown;
        private System.Windows.Forms.NumericUpDown CH6UpDown;
        private System.Windows.Forms.Label label13;
        private System.Windows.Forms.Button buttonAllOff;
        private System.Windows.Forms.Label label14;
        private System.Windows.Forms.TextBox textBoxI2CAddress;
    }
}

