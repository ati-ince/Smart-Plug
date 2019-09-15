using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO.Ports;


namespace WindowsFormsApplication1
{
    public partial class Form1 : Form
    {
        ///////////////////////////////////////////////////////////////////
        SerialPort _SerialPort = new SerialPort();
        //
        public static byte Check_control_value = 0;// if 0, not push any thing
        //
        string RxString;
        byte[] RxByte;
        string[] _GetPortNames = System.IO.Ports.SerialPort.GetPortNames();//Bu _SerialPort objesine string formatında yazdırılıyor zaten
        int[] _BaudrateList = new int[] { 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600 };//int formatinda yazdiiliyor
        int[] _DataBits = new int[] { 8, 7, 6, 5 };//int formatinda yazdiriliyor
        object[] _StopBits = new object[] { System.IO.Ports.StopBits.None, System.IO.Ports.StopBits.One, System.IO.Ports.StopBits.OnePointFive, System.IO.Ports.StopBits.Two };
        object[] _ParityList = new object[] { System.IO.Ports.Parity.Even, System.IO.Ports.Parity.Mark, System.IO.Ports.Parity.None, System.IO.Ports.Parity.Odd, System.IO.Ports.Parity.Space };
        object[] _HandshakeList = new object[] { System.IO.Ports.Handshake.None, System.IO.Ports.Handshake.RequestToSend, System.IO.Ports.Handshake.RequestToSendXOnXOff, System.IO.Ports.Handshake.XOnXOff };
        ////////////////NOT: Burada Obj cinsinden tanimlayarak daima kullanilabilir halde tutuyoruz.
       
        //
        /// <summary>
        /// Burada devamli kullanacagimiz class ve gerekli Objeleri girelim
        /// </summary>
        public Form1()
        {
            InitializeComponent();
          _SerialPort.DataReceived += new SerialDataReceivedEventHandler(DataReceivedHandler);//bunu eventi gormesi için ekledik

        }
        private void Form1_Load(object sender, EventArgs e)
        {
          //  
            
            // Baslangicta COM1 Version 9600 Baud ve diğer ozelliklere ayarlı. Dolayisi ile yanlislikla
            ///// Burada yapılmasi gerekenleri doseyelim. Com port bulunmasi, diğer gerekli değerlerşn yuklenmesi vs..       
            
            foreach (string port in _GetPortNames)//varolan portlari goruyoruz....
            {
                this.comboBox_ComPorts.Items.Add(port);
            }
            for (int i = 0; i < _BaudrateList.Length; i++)//Baudrate
            {
                this.comboBox_BaudRates.Items.Add(_BaudrateList[i]);
            }
            for (int i = 0; i < _DataBits.Length; i++)//DataBits
            {
                this.comboBox_DataBits.Items.Add(_DataBits[i]);
            }
            for (int i = 0; i < _StopBits.Length; i++)//StopBits
            {
                this.comboBox_StopBits.Items.Add(_StopBits[i].ToString());
            }
            for (int i = 0; i < _ParityList.Length; i++)//Parity
            {
                this.comboBox_Parity.Items.Add((_ParityList[i]).ToString());
            }
            for (int i = 0; i < _HandshakeList.Length; i++)//Handshake
            {
                this.comboBox_Handshaking.Items.Add(_HandshakeList[i].ToString());
            }
            //Girişte bazı aktif tutulacaklar....
            radioButton2.Checked = true;//şimdilik sadece bunlar işaretli
            radioButton5.Checked = true;
            radioButton3.Checked = true;

            radioButton1.Enabled = false;//şimdilik kapalı
            radioButton4.Enabled = false;//şimdilik kapalı
            radioButton6.Enabled = false;
            radioButton7.Enabled = false;
            //it will close (temporarily)
            radioButton5.Enabled = false;//şimdilik kapalı
            radioButton6.Enabled = false;//şimdilik kapalı
            radioButton7.Enabled = false;//şimdilik kapalı
            textBox2.Enabled = false;
            //////
            button4.Enabled = false;// i will open when i start serialport
            ////////////
            //deneme alanı text i,çin
           
            textBox5.Text = "";
            textBox3.Text="";
            textBox6.Text="";
            //textBox4.Text="";
            //textBox7.Text="";
            //textBox8.Text="";
            //textBox9.Text="";
            //textBox10.Text = "";
            ////////////
            //
          
        }
        private void Form1_FormClosing(object sender, FormClosingEventArgs e)//Bu program kapanirken eğer açık kalmışsa portu kapatıyor, baya guzel bir ozellik
        {
            if (_SerialPort.IsOpen) _SerialPort.Close();
        }
        private void comboBox1_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void comboBox2_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void comboBox3_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void radioButton1_CheckedChanged(object sender, EventArgs e)
        {

        }

        private void radioButton2_CheckedChanged(object sender, EventArgs e)
        {

        }

        private void label1_Click(object sender, EventArgs e)
        {

        }

        private void label2_Click(object sender, EventArgs e)
        {

        }

        private void label4_Click(object sender, EventArgs e)
        {

        }

        private void groupBox1_Enter(object sender, EventArgs e)
        {

        }

        

        private void groupBox2_Enter(object sender, EventArgs e)
        {

        }
        private void textBox1_KeyPress(object sender, KeyPressEventArgs e)
        {
            // If the port is closed, don't try to send a character.
            if (!_SerialPort.IsOpen) return;

            // If the port is Open, declare a char[] array with one element.
            char[] buff = new char[1];

            // Load element 0 with the key character.
            buff[0] = e.KeyChar;

            // Send the one character buffer.
            _SerialPort.Write(buff, 0, 1);

            // Set the KeyPress event as handled so the character won't
            // display locally. If you want it to display, omit the next line.
            e.Handled = true;
        }
        private void DisplayText(object sender, EventArgs e)
        {        
            for (byte i = 0; i < RxByte.Length; i++)
            {
                textBox1.AppendText(RxByte[i].ToString());
                textBox1.AppendText(" ");
            }
            if (RxByte.Length == 3) // data 3 byte degilse salla
            {
                switch (Check_control_value)
                {//Not bu kesme içinde bakılması gerekiyor...
                    // Gelen data baska bir fn içerisinde gostertilmiştir....
                    case (byte)Check_Button_Name.Pavr:
                        double deger_Pavr = (double)((RxByte[0]) + ((RxByte[1] * 256)) + ((RxByte[2] * 256*256)));  
                        textBox5.Text = (deger_Pavr/((double)8388607)).ToString();//2^23-1
                        // addition our formula
                        Pavr_calibrate();
                        break;
                    case (byte)Check_Button_Name.Vrms:
                        double deger_Vrms = (double)((RxByte[0]) + ((RxByte[1] * 256)) + ((RxByte[2] * 256*256)));  
                        textBox3.Text = ((deger_Vrms/((double)16777215))).ToString();//double calisilabilecek en rahat sayi tipi
                        // addition our formula
                        Vrms_calibrate_firsth();
                        //
                        break;

                    case (byte)Check_Button_Name.Irms:
                        double deger_Irms = (double)((RxByte[0]) + ((RxByte[1] * 256)) + ((RxByte[2] * 256*256)));
                        // addition our formula
                        textBox6.Text = (deger_Irms / ((double)16777215)).ToString();
                        Irms_calibrate_firsth(Convert.ToDouble(textBox6.Text));
                        //double Irms_last_result = Irms_formula(result_Irms_before);
                        //textBox6.Text = Irms_last_result.ToString();//2^24-1   
                        break;

                    //case (byte)Check_Button_Name.Qavr:
                    //    double deger_Qavr = (double)((RxByte[0]) + ((RxByte[1] * 256)) + ((RxByte[2] * 256*256)));
                    //    textBox4.Text = (deger_Qavr / ((double)8388607)).ToString();
                    //    break;
                    //case (byte)Check_Button_Name.S:
                    //    double deger_S = (double)((RxByte[0]) + ((RxByte[1] * 256)) + ((RxByte[2] * 256*256)));
                    //    textBox7.Text = (deger_S / ((double)8388607)).ToString();
                    //    break;
                    //case (byte)Check_Button_Name.PF:
                    //    double deger_PF = (double)((RxByte[0]) + ((RxByte[1] * 256)) + ((RxByte[2] * 256*256)));
                    //    textBox8.Text = (deger_PF / ((double)8388607)).ToString();
                    //    break;
                    //case (byte)Check_Button_Name.Psum:
                    //    double deger_Psum = (double)((RxByte[0]) + ((RxByte[1] * 256)) + ((RxByte[2] * 256*256)));
                    //    textBox9.Text = (deger_Psum / ((double)8388607)).ToString();
                    //    break;
                    //case (byte)Check_Button_Name.Sys_Time:
                    //    double deger_Sys_Time = (double)((RxByte[0]) + ((RxByte[1] * 256)) + ((RxByte[2] * 256*256)));  
                    //    textBox10.Text = (deger_Sys_Time/((double)16777215)).ToString();
                    //    break;
                    default://nothing specially
                        break;
                }
                Check_control_value = 0;//again
            }
            //after all of them finish
            
              
        }
        void Pavr_calibrate()
        {
            double value_Pavr_R_d = (215*15);//carpma icin belirledik
            string value_Pavr = textBox5.Text;
            double value_Pavr_d = Convert.ToDouble(value_Pavr);
            value_Pavr_R_d = value_Pavr_R_d / ((double)0.36);
            value_Pavr_d = value_Pavr_d * value_Pavr_R_d;
            textBox5.Text = ((float)value_Pavr_d).ToString();
        }

        void Vrms_calibrate_firsth()
        {
            double value_Pavr_R_d = 215;//carpma icin belirledik
            string value_Pavr = textBox3.Text;
            double value_Pavr_d = Convert.ToDouble(value_Pavr);
            value_Pavr_R_d = value_Pavr_R_d / ((double)0.6);
            value_Pavr_d = value_Pavr_d * value_Pavr_R_d;
            textBox3.Text = ((float)value_Pavr_d).ToString();
        }

        void Irms_calibrate_firsth(double Irms_coming)
        {
            double multiply_sonuc=0;

            if (Irms_coming <= 0.033) { multiply_sonuc = 15.0; }
            else if ((Irms_coming > 0.033) && (Irms_coming <=0.057)) { multiply_sonuc = 15.0; }
            else if ((Irms_coming > 0.057) && (Irms_coming <= 0.08)) { multiply_sonuc = 15.0; }
            else if ((Irms_coming > 0.08) && (Irms_coming <= 0.092)) { multiply_sonuc = 15.0; }
            else if ((Irms_coming > 0.092) && (Irms_coming <= 0.1)) { multiply_sonuc = 15.0; }
            else if ((Irms_coming > 0.1) && (Irms_coming <= 0.11)) { multiply_sonuc = 15.0; }
            else if ((Irms_coming > 0.11) && (Irms_coming <= 0.12)) { multiply_sonuc = 15.0; }
            else if ((Irms_coming > 0.12) && (Irms_coming <= 0.139)) { multiply_sonuc = 15.0; }
            else if ((Irms_coming > 0.139) && (Irms_coming <= 0.154)) { multiply_sonuc = 15.0; }
            else if ((Irms_coming > 0.154) && (Irms_coming <= 0.168)) { multiply_sonuc = 15.0; }
            else if ((Irms_coming > 0.168) && (Irms_coming <= 0.178)) { multiply_sonuc = 15.0; }
            else { multiply_sonuc = 15.0; }
            //
            //
            string value_Pavr_R = (multiply_sonuc).ToString();//Burada carpma icin hazır ettik
            double value_Pavr_R_d = Convert.ToDouble(value_Pavr_R);
            string value_Pavr = textBox6.Text;
            double value_Pavr_d = Convert.ToDouble(value_Pavr);
            value_Pavr_R_d = value_Pavr_R_d /((double)0.6);
            value_Pavr_d = value_Pavr_d * value_Pavr_R_d;
            textBox6.Text = ((float)value_Pavr_d).ToString();
        }


        double Irms_formula(double deger)
        {
            double result_Irms=0;
            result_Irms = deger;

            return result_Irms; 
        }


        //public  double  Vrms_formula ()
       // {
       //     double result_Vrms=0;


      //    return result_Vrms;
      //  }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {
            
        }

        private void textBox2_TextChanged(object sender, EventArgs e)
        {

        }

        private void radioButton5_CheckedChanged(object sender, EventArgs e)
        {

        }

        private void radioButton7_CheckedChanged(object sender, EventArgs e)
        {

        }

        private void radioButton6_CheckedChanged(object sender, EventArgs e)
        {

        }

        private void button1_Click(object sender, EventArgs e)
        {
            //
            button4.Enabled = true;// i will open when i start serialport
            //
            SByte i_ComPorts = Convert.ToSByte(comboBox_ComPorts.SelectedIndex.ToString());
            SByte i_BaudRates = Convert.ToSByte(comboBox_BaudRates.SelectedIndex.ToString());
            SByte i_DataBits = Convert.ToSByte(comboBox_DataBits.SelectedIndex.ToString());
            SByte i_StopBits = Convert.ToSByte(comboBox_StopBits.SelectedIndex.ToString());
            SByte i_Handshaking = Convert.ToSByte(comboBox_Handshaking.SelectedIndex.ToString());
            SByte i_Parity = Convert.ToSByte(comboBox_Parity.SelectedIndex.ToString());
            //Buraya kadar değerleri kullanma amacı ile alıp aşagıda da kullandık
            if (i_ComPorts == (-1)) { i_ComPorts++; }
            _SerialPort.PortName = _GetPortNames[i_ComPorts];
            if (i_BaudRates == (-1)) { i_BaudRates++; }
            _SerialPort.BaudRate = _BaudrateList[i_BaudRates];
            if (i_DataBits == (-1)) { i_DataBits++; }
            _SerialPort.DataBits = _DataBits[i_DataBits];
            //Şunları bir ara ayarla artık nasıl olacaksa
            //_SerialPort.StopBits =StopBits.None;
            //_SerialPort.Handshake = Parity.None;
            //_SerialPort.Parity = Handshake.None;
            /////// 
            ///Burada portu eğer açık değilse açalım
            if (!(_SerialPort.IsOpen))
            {
                _SerialPort.Open();
                if (_SerialPort.IsOpen)
                {
                    button1.Enabled = false;
                    button2.Enabled = true;
                    //
                    textBox1.ReadOnly = false;//bir deneyelim
                }
            }
        }

        private void comboBox_StopBits_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void button2_Click(object sender, EventArgs e)
        {
            //
            textBox1.Clear();
            //
            button4.Enabled = false;// i will open when i start serialport
            //
            if (_SerialPort.IsOpen)
            {
                _SerialPort.Close();
                if (!(_SerialPort.IsOpen))
                {
                    button1.Enabled = true;
                    button2.Enabled = false;
                    //
                    textBox1.ReadOnly = true;
                }
            }

        }

        private void button4_Click(object sender, EventArgs e)
        {
            // If the port is closed, don't try to send a character.
            if (!_SerialPort.IsOpen) return;

            // If the port is Open, declare a char[] array with one element.
            char[] buff = new char[1];//sadece 0 datası gonderiliyor...

           //  Load element 0 with the key character.


             //Send the one character buffer.
            _SerialPort.Write(buff, 0, 1);

            // Set the KeyPress event as handled so the character won't
             //display locally. If you want it to display, omit the next line. 

        }

        private void DataReceivedHandler(object sender, SerialDataReceivedEventArgs e)
        {   // Event for receiving data
            // Read the buffer to text box.
            //SerialPort sp = (SerialPort)sender;
            int sonuc;
            RxByte=new byte[_SerialPort.BytesToRead];
            sonuc = _SerialPort.Read(RxByte, 0, _SerialPort.BytesToRead);//i'll change string to byte 
            //in this place i have to decide whic data is came....
            this.Invoke(new EventHandler(DisplayText));
            //
        }

        private void label5_Click(object sender, EventArgs e)
        {

        }

        private void groupBox3_Enter(object sender, EventArgs e)
        {

        }

        private void label10_Click(object sender, EventArgs e)
        {

        }

        private void label11_Click(object sender, EventArgs e)
        {

        }

        private void label9_Click(object sender, EventArgs e)
        {

        }

        private void label12_Click(object sender, EventArgs e)
        {

        }

       

        private void button5_Click(object sender, EventArgs e)
        { //16-6
        if (!_SerialPort.IsOpen) return;//else
            Check_control_value = 3;
            // If the port is Open, declare a char[] array with one element.
            byte[] buff = new byte[] { 144, 6 };
            //Send the one character buffer.
            _SerialPort.Write(buff, 0, buff.Length);
        }

        private void button3_Click(object sender, EventArgs e)
        {//16-7
        if (!_SerialPort.IsOpen) return;//else
            Check_control_value = 2;
            // If the port is Open, declare a char[] array with one element.
            byte[] buff = new byte[] { 144, 7 };
            //Send the one character buffer.
            _SerialPort.Write(buff, 0, buff.Length);
        }

        private void button10_Click(object sender, EventArgs e)
        {//16-29
        if (!_SerialPort.IsOpen) return;//else
            Check_control_value = 7;
            // If the port is Open, declare a char[] array with one element.
            byte[] buff = new byte[] { 144, 29 };
            //Send the one character buffer.
            _SerialPort.Write(buff, 0, buff.Length);
        }

        private void label16_Click(object sender, EventArgs e)
        {

        }

        private void button6_Click_1(object sender, EventArgs e)
        {//16-5
        if (!_SerialPort.IsOpen) return;//else
            Check_control_value = 1;
            // If the port is Open, declare a char[] array with one element.
            byte[] buff = new byte[] { 144, 5 };
            //Send the one character buffer.
            _SerialPort.Write(buff, 0, buff.Length);
        }

        private void button7_Click(object sender, EventArgs e)
        {//16-14
        if (!_SerialPort.IsOpen) return;//else
            Check_control_value = 4;
            // If the port is Open, declare a char[] array with one element.
            byte[] buff = new byte[] { 144, 14 };
            //Send the one character buffer.
            _SerialPort.Write(buff, 0, buff.Length);
        }

        private void button8_Click(object sender, EventArgs e)
        {//16-20
        if (!_SerialPort.IsOpen) return;//else
            Check_control_value = 5;
            // If the port is Open, declare a char[] array with one element.
            byte[] buff = new byte[] { 144, 20 };
            //Send the one character buffer.
            _SerialPort.Write(buff, 0, buff.Length);
        }

        private void button9_Click(object sender, EventArgs e)
        {//16-21
        if (!_SerialPort.IsOpen) return;//else
            Check_control_value = 6;
            // If the port is Open, declare a char[] array with one element.
            byte[] buff = new byte[] { 144, 21 };
            //Send the one character buffer.
            _SerialPort.Write(buff, 0, buff.Length);
        }

        private void button11_Click(object sender, EventArgs e)
        {//16-61//in samples
        if (!_SerialPort.IsOpen) return;//else
            Check_control_value = 8;
            // If the port is Open, declare a char[] array with one element.
            byte[] buff = new byte[] { 144, 61 };
            //Send the one character buffer.
           _SerialPort.Write(buff, 0, buff.Length);
            //
            


        }
        private enum Check_Button_Name
        { 
            Pavr=1,
            Vrms=2,
            Irms=3,
            Qavr=4,
            S=5,
            PF=6,
            Psum=7,
            Sys_Time=8
        }

        private void textBox13_TextChanged(object sender, EventArgs e)
        {

        }

        //private void button12_Click(object sender, EventArgs e)
        //{
        //    string value_Pavr_R=textBox11.Text;
        //    double value_Pavr_R_d = Convert.ToDouble(value_Pavr_R);
        //    string value_Pavr = textBox5.Text;
        //    double value_Pavr_d = Convert.ToDouble(value_Pavr);
        //    value_Pavr_R_d=value_Pavr_R_d / ((double)0.36);
        //    value_Pavr_d = value_Pavr_d * value_Pavr_R_d;
        //    textBox5.Text = value_Pavr_d.ToString();
        //}

        //private void button13_Click(object sender, EventArgs e)
        //{
        //    string value_Pavr_R = textBox19.Text;
        //    double value_Pavr_R_d = Convert.ToDouble(value_Pavr_R);
        //    string value_Pavr = textBox3.Text;
        //    double value_Pavr_d = Convert.ToDouble(value_Pavr);
        //    value_Pavr_R_d = value_Pavr_R_d / ((double)0.6);
        //    value_Pavr_d = value_Pavr_d * value_Pavr_R_d;
        //    textBox3.Text = value_Pavr_d.ToString();
        //}

        //private void button15_Click(object sender, EventArgs e)
        //{
        //    string value_Pavr_R = textBox18.Text;
        //    double value_Pavr_R_d = Convert.ToDouble(value_Pavr_R);
        //    string value_Pavr = textBox6.Text;
        //    double value_Pavr_d = Convert.ToDouble(value_Pavr);
        //    value_Pavr_R_d = value_Pavr_R_d / ((double)0.6);
        //    value_Pavr_d = value_Pavr_d * value_Pavr_R_d;
        //    textBox6.Text = value_Pavr_d.ToString();
        //}

        private void button14_Click(object sender, EventArgs e)
        {

        }

        private void button19_Click(object sender, EventArgs e)
        {

        }

        private void button17_Click(object sender, EventArgs e)
        {

        }

        private void textBox5_TextChanged(object sender, EventArgs e)
        {
          

        }

        private void textBox19_TextChanged(object sender, EventArgs e)
        {

        }

        private void textBox3_TextChanged(object sender, EventArgs e)
        {
          
        }

        private void button12_Click(object sender, EventArgs e)
        {
            if (!_SerialPort.IsOpen) return;//else
            Check_control_value = 0;
            // If the port is Open, declare a char[] array with one element.
            byte[] buff = new byte[] { 153, 153 };//99 hex
            //Send the one character buffer.
            _SerialPort.Write(buff, 0, buff.Length);
        }

        private void button13_Click(object sender, EventArgs e)
        {
            if (!_SerialPort.IsOpen) return;//else
           Check_control_value = 0;
            // If the port is Open, declare a char[] array with one element.
            byte[] buff = new byte[] { 153, 153 };//99 hex
            //Send the one character buffer.
            _SerialPort.Write(buff, 0, buff.Length);
        }

        private void button7_Click_1(object sender, EventArgs e)
        {
            if (!_SerialPort.IsOpen) return;//else
            Check_control_value = 0;
            // If the port is Open, declare a char[] array with one element.
            byte[] buff = new byte[] { 213 }; //Cirus reset 88Hex
            //Send the one character buffer.
            _SerialPort.Write(buff, 0, buff.Length);//start ...           
        }

        private void pictureBox1_Click(object sender, EventArgs e)
        {

        }

        private void pictureBox2_Click(object sender, EventArgs e)
        {

        }

        private void button8_Click_1(object sender, EventArgs e)
        {
            if (!_SerialPort.IsOpen) return;//else
            Check_control_value = 0;
            // If the port is Open, declare a char[] array with one element.
            byte[] buff = new byte[] { 136, 136 };//88 restart hex
            //Send the one character buffer.
            _SerialPort.Write(buff, 0, buff.Length);
        }

        private void button9_Click_1(object sender, EventArgs e)
        {
            if (!_SerialPort.IsOpen) return;//else
            Check_control_value = 0;
            // If the port is Open, declare a char[] array with one element.
            byte[] buff = new byte[] { 215 }; //Cirus reset 88Hex
            //Send the one character buffer.
            _SerialPort.Write(buff, 0, buff.Length);//start ...   
        }

        private void button11_Click_1(object sender, EventArgs e)
        {
            if (!_SerialPort.IsOpen) return;//else
            Check_control_value = 0;
            // If the port is Open, declare a char[] array with one element.
            byte[] buff = new byte[] { 144, 64, 10, 2,  16 }; //Cirus HPF on 90 40 0A 02 10
            //Send the one character buffer .
            _SerialPort.Write(buff, 0, buff.Length);//start ...   
        }

        private void button10_Click_1(object sender, EventArgs e)
        {
            if (!_SerialPort.IsOpen) return;//else
            Check_control_value = 0;
            // If the port is Open, declare a char[] array with one element.
            byte[] buff = new byte[] { 128, 64, 32, 32, 192 }; //Cirus HPF on 90 40 0A 02 10
            //Send the one character buffer .
            _SerialPort.Write(buff, 0, buff.Length);//start ...   
        }

        private void label12_Click_1(object sender, EventArgs e)
        {

        }

        private void textBox6_TextChanged(object sender, EventArgs e)
        {
            
        }
    }
}

            
        


    

