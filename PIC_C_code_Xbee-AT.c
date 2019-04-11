/* 
This software has developed by  @atahir.ince and @giraykiral. 

The main purpose of this software is taking various order which are defined before from a external modul (wireless or another modul) to
getting electrical power measurement value from cirrus power  ic.

One of them this UART is using to communicate between cirrus electrical power ic and PIC mcu,
and also, another UART channel is using for communicate between external modul and PIC. 

In this application we developed a novel smart device communication protocol for communicate easly  between external modul. 
------------
The Protocol
------------
Packet Type (1 Byte) + Time Values [Hour, Minute, Second ( 3 Byte)] + Getting or Setting Values (up to 5 Byte)
--------------------------------------------------------------------------------------------------------------
Packets 
-------
0x11 -> Read Vrms , example, Sending -> 0x11 0x01 0x02 0x03 , Getting <- 0x11 0x01 0x02 0x03 0xD9 0x03 (we are taking 216.3 V )
0x22 -> Read Irms , example, Sending -> 0x22 0x01 0x02 0x03 , Getting <- 0x22 0x01 0x02 0x03 0x05 0x01 (we are taking 5.1 A )
0x33 -> Read Pavr , example, Sending -> 0x33 0x01 0x02 0x03 , Getting <- 0x33 0x01 0x02 0x03 0x01 0x64 0x10 0x05 (we are taking 1116.5 W )
0x44 -> Read PF   , example, Sending -> 0x44 0x01 0x02 0x03 , Getting <- 0x44 0x01 0x02 0x03 0x40 0x07 (we are taking + 48.7 degree )
--
for PF if we inside 0-90 degree, we are positive, if we are in 90-180 degree we are negative, can be consider like sinus (1 byte enough)
----
0x55 -> another Pavr measurement method, Read Pavg using Vrms and Irms simultaneously.... And multiplied them for getting Pavg value
....
....
0xBB -> write something inside cirrus register for what we want (using 24 bit again)
0xCC -> read cirrus ic register be 24 bit character
0xDD -> Start system properly.... Reset all value and set again. Check the system and start.....
        If your system not working well, use this command frame, and check the some register if you think handle that or not.
0xEE -> Check the relay state ( 0x00 <- Open Circuit , 0x01 <- Close Circuit, line is activated)
        (or tyristor, MOSFET, .. power BJT whatever... Just we are using "Power Switch")

*/
#include <18F26J11.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//
#use delay(clock=8Mhz) 
#use         fast_io(c) //c ve b portlari kulanima hazirlaniyor
#use         fast_io(b)
//    
#FUSES NOWDT,WDT128,HS,STVREN,NOPROTECT,FCMEN,IESO,IOL1WAY,NOWPCFG,WPEND,WPDIS                 
#FUSES LPT1OSC,T1DIG,MSSPMSK7,DSWDT2147483648,DSWDT,DSBOR,RTCOSC_T1,DSWDTOSC_INT,WPFP                   
//
//Burada UART2 ve UART1 tan�mlanm�� oldu
//UART 2 icin pin_select yapilmali
#define      XB_DI       PIN_C1 
#define      XB_DO       PIN_C0 
#pragma      pin_select  U2TX=XB_DI  
#pragma      pin_select  U2RX=XB_DO 
// Sistemimiz (modul ile haberle�me=UART1) 9600 da �al��mnaktad�r ve Ba�lang��ta Cirrus 600 baud da �al��makta
#pragma use  rs232(baud=9600,xmit=PIN_C6,rcv=PIN_C7,bits=8,ERRORS,STREAM=UART_1)   // Modul
#pragma use  rs232(uart2,baud=600,bits=8,ERRORS,STREAM=UART_2) //  Cirrus


//PROTOT�PLER//////////////////////////////////////////////////////////////////////////
//
void system_starting(void); // sistem ba�lat�lmas� i�in gereken ayarlar...
//
void serihaberlesme_kesmesi (void);
void serihaberlesme_kesmesi_2 (void);
void Cirrus_Starting_Setting(void);
void pic_kurulum(void);
void uart_change(void);
void HPF_ON(void);
void change_CG(void);
void Start_Cirrus(void);
void Start_Single(void);
void Stop_Cirrus(void);
void Reset_Cirrus(void);
void Dizi_Temizleme(void);
void UartIeGelen_UartIIyeAktarilsin(void);
void UartIIyeGelen_UartIeAktarilsin(void);
void Set_Tsettle(void);
void Set_SampleCount(void);
void ClearDRDY(void);
void Send_AC_Gain(void);
void Read_Vrms(void);
void Read_Irms(void);
void Read_Pavg(void);
float32 Read_Pavg_diff(void);
void Read_PF(void);
void Read_INT_STATUS_DRDY(void);
void  Kalibrasyon(void);
//
void plug_packet_fn(void )    ;   
unsigned int checksum_API_mode(void)   ;
char checksum_API_corrected(unsigned int checksum_x);
float32 calculation_24bits(unsigned int32 bolen, float32 AFE_scala, unsigned int32 carpan );//16777215,.6,215
/****************************************************************************************************************/
void Akim_Okuma_Fn(void);
void Gerilim_Okuma_Fn(void);
void Guc_Okuma_Fn(void);
void Cirrus_Register_Yazdirma(void);
void Cirrus_Register_Okuma(void);
void Plug_Restart(void);
void Plug_Relay_Control(void);
//////
void Xbee_API_mod_kullanim(void);
////////////
/////////////////////////////////////////Degi�kenler Tan�mlan�yor
unsigned  char         uart_1_alinan[50];
unsigned  char         uart_2_alinan[50];
//
static char  i=0; // UART1 kac byte data geldi
static char  k=0; // UART2 kac byte data geldi

static int32 timer_sayac=0;
static int32 timer_zaman=0;
static int32 timer_zaman2=0;

char         kosul_rda=0; // data geldi ise kesme de�i�keni
char         kosul_rda2=0;



/////////////
static int8 relay_state=0;
// Kesme-Interrupt'lar
// RX ucuna veri gelince meydane gelen kesme
#int_rda   
   void serihaberlesme_kesmesi (void)
         {  
            kosul_rda=1; // eger data gelmi� ise
            timer_zaman=timer_sayac;
            // Burada UART timeout de�eri i�in ayarlama yapt�k timer ile
            //alinan verilerin hepsini dizine atiyoruz
            uart_1_alinan[i]=getch(UART_1);
            i++;      
            enable_interrupts(int_rda);  // int ��karken s�f�rla         
         }         
// RX2 ucuna veri gelince meydane gelen kesme  
#int_rda2   
   void serihaberlesme_kesmesi_2 (void)
         {
            kosul_rda2=1;
            timer_zaman2=timer_sayac;
            uart_2_alinan[k]=getch(UART_2);
            k++;
            enable_interrupts(int_rda2);
         } 
//timer rda kesmesi sonucu zaman gecikmesi ve daha d�zg�n bir cal��ma i�in yap�ld�
#INT_TIMER2
   void timer2_kesme()
         {
            // Burada UART kesmeleri timeout i�in ayarlamaklar yap�lmakta....
            // timer ayarlar� ~2ms i�in (timer sayac 2ms de bir artmakta)
            timer_sayac++;
            if (timer_sayac>100000){timer_sayac=0;} 
            // overflow oplmamas� i�in arada temizlemeli.
            
         }

   void main()
      {   
      ///////////////BURADAK� �FADELER� RESET YAPILDI�INDA MUTLAKA TEKRARLAMALISIN
      system_starting();//tum gerekli donan�m ayarlar� yap�lmakta.....
      //
      while(1)// kalibrasyon_test yazars�n
         {
           if((kosul_rda==1)&&((timer_sayac-timer_zaman)>5)) // 2ms*5 = 10ms timeout degeri konuldu...
           // eger herhangi bir UART1 kesmesi aktif olmus ise....
           {
           plug_packet_fn();// UART1 tum i�lemler burada yap�lmakta..
           // burada diziler de temizleniyor ayn� zamanda yeni i�lem i�in
           }
           
           //Eger Cirrus kendini resetlerse ayarlar� tekrar yap�lmakta....
           else if ((kosul_rda2==1)&&(k==1)&&(uart_2_alinan[0]==0x00)&&((timer_sayac-timer_zaman)>5)) 
           {Cirrus_Starting_Setting();k=0;kosul_rda2=0;}
         }        
     }
     
// sub functions/////////////////////////////////
void system_starting(void) // t�m ba�lang�� de�erleri burada ayarlamnaktay�z...
{
      Dizi_Temizleme();//�al��maya ba�lamadan t�m dizinlerin i�ini bo�alt�yoruz (UART i�in kullan�lan)
      pic_kurulum(); // A��l��ta gerekli baz� ayarlar yap�lmakta
      delay_ms(500);// 
      
      //Burada ba�lang��ta cirrus' a donan�msal reset at�lmakta
      Reset_Cirrus();      
      // Cirrus ayarlamalar� yap�l�p, baudrate 9600 ayarlanmakta...
      Cirrus_Starting_Setting();       
}

//
void Cirrus_Starting_Setting() // Reset sonrasi, Cirrus kurulum ayarlar� yap�ld�
{
       uart_change();
         delay_ms(50);
      HPF_ON();
         delay_ms(50);
      change_CG();
         delay_ms(50);
}

//
void pic_kurulum()
{
  setup_oscillator(OSC_PLL_ON); 
  setup_adc_ports(NO_ANALOGS|VSS_VDD); 
  setup_wdt(WDT_OFF); 
  setup_timer_0(RTCC_EXT_L_TO_H); 
  setup_timer_1(T1_FOSC | T1_DIV_BY_1); 
  enable_interrupts(GLOBAL); 
  enable_interrupts(INT_RDA);      
  enable_interrupts(INT_RDA2);
  enable_interrupts(INT_TIMER2);
  setup_timer_2(T2_DIV_BY_16,255,1); 
  setup_timer_3(T3_DISABLED|T3_DIV_BY_1); 
  setup_timer_4(T4_DISABLED,0,1); 
  setup_ccp1(CCP_OFF); 
  setup_comparator(NC_NC_NC_NC);// This device COMP currently not supported by the PICWizard 
  
  set_tris_C(0b10000001);// UART I ve II i�in input tayini yap�ld�
  set_tris_B(0x00); //B portu komple ��k�� olarak y�nlendiriliyor
  
  output_C(0x00); // C portu ��k��� komple s�f�r yap�l�yor.
  output_B(0x00); // B portu ��k��� komple s�f�r yap�l�yor.  
  //Burada cirrus donan�msal reset i�in kulan�lacak RB5 
  output_high(pin_b7);
}
void uart_change()
     {
          //reset at�l���nda tekrar 600 degerine set edilmeli yoksa resetten sonra putc islevini yapamaz
          // Bu fn Cirrus reseti i�in kullan�lmakta...
          set_uart_speed(600,UART_2);
          delay_ms(1000);
          putc(0x80,UART_2);
          putc(0x47,UART_2);
          putc(0xCD,UART_2);
          putc(0x04,UART_2);
          putc(0x02,UART_2);
         delay_ms(1000);
          set_uart_speed(9600,UART_2);//define olarak tanimladik,acildiktan sonra kendini 9600'e cekecek.
         delay_ms(1000);
     }
void HPF_ON()  /////////high pass filter aktif hale getiriliyor, cirrus
     {
          putc(0x90,UART_2);
          putc(0x40,UART_2);
          putc(0x0A,UART_2);
          putc(0x02,UART_2);
          putc(0x10,UART_2);
          delay_ms(50);
     //     putc(0x90,UART_2);
       //   putc(0x00,UART_2);
         // delay_ms(50);
     }
void change_CG() ///// akim kazanc� x50 cikariliyor, Cirrus i�in
     {
          putc(0x80,UART_2);
          putc(0x40,UART_2);
          putc(0x20,UART_2);
          putc(0x20,UART_2);
          putc(0xC0,UART_2);
         delay_ms(50);
     }
void Start_Cirrus()  /////////
     {
          //continious mod ba�lat�lmakta
          putc(0xD5,UART_2);
          relay_state=1;
          delay_ms(50);
     }
void Start_Single() ///////////
     {
          // single mod ba�lat�lmakta
          putc(0xD4,UART_2);
          delay_ms(50);
     }
void Stop_Cirrus() /////////////stop biti g�nderiliyor, Cirrus i�in
     {
          putc(0xD8,UART_2);
          delay_ms(50);
          
     }
void Reset_Cirrus()
     {     
           // Cirrus donan�msal reset
           output_low(pin_b7);
           delay_ms(50);
           output_high(pin_b7);
           delay_ms(50);
           
     }
void Set_Tsettle()   ////////////2000ms ayarlan�yor, Cirrus i�in
     {
          putc(0x90,UART_2);
          putc(0x79,UART_2);
          putc(0x40,UART_2);
          putc(0x1F,UART_2);
          putc(0x00,UART_2);
          delay_ms(50);

     }
void Set_SampleCount() /////////////// 16000 de�erine set ediliyor, Cirrus i�in
     {
          putc(0x90,UART_2);
          putc(0x73,UART_2);
          putc(0x80,UART_2);
          putc(0x3E,UART_2);
          putc(0x00,UART_2);
         delay_ms(50);

     }
void ClearDRDY() /////////////// DRDY siliniyor, Cirrus i�in
     {
          putc(0x80,UART_2);
          putc(0x57,UART_2);
          putc(0xFF,UART_2);
          putc(0xFF,UART_2);
          putc(0xFF,UART_2);
         delay_ms(50);
     }
void Send_AC_Gain()// gain ayarlan�yor , Cirrus i�in
     {
          putc(0xFE,UART_2);
          delay_ms(50);
     }
void Read_Vrms() ///////////////////////Gerilim de�eri okunuyor
     {
          putc(0x90,UART_2);
          putc(0x07,UART_2);
          delay_ms(50);
         
     }
void Read_Irms()  ///////////////////////Ak�m de�eri okunuyor
     {
          putc(0x90,UART_2);
          putc(0x06,UART_2);
          delay_ms(50);
     }
void Read_Pavg()  ///////////////////////G�� de�eri okunuyor
     {
          putc(0x90,UART_2);
          putc(0x05,UART_2);
          delay_ms(50);
     }

void Read_PF()  ///////////////////////G�� Fakt�r� de�eri okunuyor
     {
          putc(0x90,UART_2);
          putc(0x15,UART_2);
          delay_ms(50);
     }
void Read_INT_STATUS_DRDY()
     {
          putc(0x80,UART_2);
          putc(0x17,UART_2);
          delay_ms(50);
     }
void Dizi_Temizleme()//t�m bufferler temizlendi, ilk andaki, durum
{
         char n;
            for(n=0;n<50;n++)
               {
                  uart_1_alinan[n]=0;
                  
               }i=0; kosul_rda=0;
         char p;
            for(p=0;p<50;p++)
               {        
                  uart_2_alinan[p]=0;

               }  k=0;kosul_rda2=0;       
}
///////////////
void plug_packet_fn(void ) // tum UART okuma vs.. buarada yap�lmakta.
{
float32 return_double_value,value;
unsigned char Vrms_high,Vrms_low;
unsigned char Irms_high,Irms_low;
unsigned char Pavr_256_High,Pavr_256_Low,Pavr_After_Comma;
unsigned int Pavr_Total;
/////
switch(uart_1_alinan[0])
   {
                  //////////////////////////////////////
      case 0x11: //Geilim icin ayarlanan case yap�s�///                 
             Read_Vrms();
             return_double_value=calculation_24bits(16777215,0.6,378);
             value=return_double_value;
             Vrms_high=((unsigned char) value);
             value=value-((double)Vrms_high);
             value=value*((double) 100.0);
             Vrms_low=(unsigned char) value;
             ///////////////////////////////////////////////////////
             putc(0x11,UART_1);
             putc(uart_1_alinan[1],UART_1);
             putc(uart_1_alinan[2],UART_1);
             putc(uart_1_alinan[3],UART_1);
             putc(Vrms_high,UART_1);
             putc(0x2E,UART_1);
             putc(Vrms_low,UART_1); 
             break;
      
                 ///////////////////////////////////
      case 0x22://Akim icin ayarlanan case yap�s�//
             Read_Irms(); // gerilim degeri cirrusa soruluyor
             return_double_value=calculation_24bits(16777215,0.6,16);// Bunu katsay� bolgelerine ay�racag�z......
             value=return_double_value;
             Irms_high=(unsigned char) value;
             value=value-((double)Irms_high);
             value=value*((double) 100.0);
             Irms_low=(unsigned char) value;
             //////////////////////////////////////////////////////
             putc(0x22,UART_1);
             putc(uart_1_alinan[1],UART_1);
             putc(uart_1_alinan[2],UART_1);
             putc(uart_1_alinan[3],UART_1);
             putc(Irms_high,UART_1);
             putc(0x2E,UART_1);
             putc(Irms_low,UART_1);     
             break;
    
                ///////////////////////////////////
      case 0x33://Guc icin ayarlanan case yap�s�//
            // Burada �ncelikle Dogrudan Pavr olcumu yapacagiz      
            Read_Pavg();
            return_double_value=calculation_24bits(8388607,0.36,775);// Bunu katsay� bolgelerine ay�racag�z......
            value=return_double_value;
            //
            Pavr_Total=(unsigned int) value;
            value=value-((float32)Pavr_Total);
            value=value*((float32) 100.0);
            Pavr_After_Comma=(unsigned char) value;
            ////
            Pavr_256_Low=( (unsigned char) (Pavr_Total%256) );
            Pavr_256_High=( (unsigned char) ((Pavr_Total-Pavr_256_Low)/256) );
            //////////////////////////////////////////////////////
            putc(0x33,UART_1);
            putc(uart_1_alinan[1],UART_1);
            putc(uart_1_alinan[2],UART_1);
            putc(uart_1_alinan[3],UART_1);
            putc(Pavr_256_High,UART_1);
            putc(Pavr_256_Low,UART_1);
            putc(0x2E,UART_1);
            putc(Pavr_After_Comma,UART_1);   
            break;
      
                ///////////////////////////////////
      case 0x44: 
            Read_PF();     
            break;
     
      
      case 0x55: // Power avr measurement different method
                 // 0x44 ile de bir di�er g�� okuma y�ntemini kullanabilmekteyiz
           //   float32 double_power_value=Read_Pavg_diff(); // Bir ara kullanaca��z...
            break;
                 ///////////////////////////////////////
      case 0xBB: //yazd�rma i�in komut   
            putc(uart_1_alinan[4],UART_2);
            putc(uart_1_alinan[5],UART_2);
            putc(uart_1_alinan[6],UART_2);
            putc(uart_1_alinan[7],UART_2);
            putc(uart_1_alinan[8],UART_2);
            delay_ms(50);  
            break;
     
                //////////////////////////////////////
      case 0xCC://okuma i�in komut
            putc(uart_1_alinan[4],UART_2);
            putc(uart_1_alinan[5],UART_2);
            delay_ms(50);
            putc(0xCC,UART_1);
            putc(uart_1_alinan[1],UART_1);
            putc(uart_1_alinan[2],UART_1);
            putc(uart_1_alinan[3],UART_1);
            putc(uart_2_alinan[0],UART_1);
            putc(uart_2_alinan[1],UART_1);
            putc(uart_2_alinan[2],UART_1);  
            break;
      
                //////////////////////////////////////////////////////////
      case 0xDD://gerekli kurulum ayarlari icin olusturulan case yap�s�//
                ////////////////////////////////////////////////////////
            Plug_Restart(); // Burada sistem gerekli kalibreler vs... herseyi ile haz�r hale getiriliyor
            ////
            // devam�nda Qavg vs.. icin gerekliler yazd�r�lacaklar....
            ///////////////////////////////////////////////////////                                       
           break;
                 /////////////////////////////////////////////  
      case 0xEE:// Role kontrol icin haz�rlanan case yapisi//
              switch (uart_1_alinan[4]) //ilk byte frame number sonraki 3 byte zaman sonraki byte'a g�re karar verilmesi i�in bir sw-case yap�s� daha olu�turuldu
               {                     
                case 0x01: //r�leye a�ma komutu g�nderilmesi isteniyor, Kullanilan picde b4 pini role cektirmek icin ayarland� bu pini toogle ederek role ac/kapa yapabiliyoruz            
                  output_high(pin_b4);
                  relay_state=1;
                  break;
            
               case 0x02: //r�leye kapama komutu g�nderilmesi isteniyor
                  output_low(pin_b4);
                  relay_state=0;
                  break;            

              default: break;//finish
               }
               
                 putc(0xEE,UART_1);
                 putc(uart_1_alinan[1],UART_1);
                 putc(uart_1_alinan[2],UART_1);
                 putc(uart_1_alinan[3],UART_1);
                 putc(relay_state,UART_1);   
              break;
      
      
          // buras� API modunda paketin geldi�ine dalalet etmekte 
          // Burada paket �u �ekildedir...
          // NOT sadece 0x10 ile gonderilen data paketi kullan�lacakt�r. ve data geldi�inde malum 0x90 ile gelmi� olacakt�r.
          //0x10 -> 0x90 normal RF data transmission
//////////////
//////////////
//////////////
  /*    case 0x7E:
               Xbee_API_mod_kullanim();// Yapilacak isler a�agida uzunca tanimlanmi�tir!!!!          
                                         break;  */
//////////////
//////////////
//////////////
      
      default:                           break;//finish
   }
   Dizi_Temizleme(); //��karken ko�ullar� ve alinan veri de�erlili�i olan i,k,rda,rda2 de�i�kenini 0 a �ekiyoruz     
   }
   
//Burada okunan 24 bit data donusumu yap�lmakta
/////////////////////////////////////////////////
float32 calculation_24bits(unsigned int32 bolen, float32 AFE_scala, unsigned int32 carpan )//16777215,.6,215
 {
   unsigned int32 sum_24bits, max_value_24bit=0xFFFFFF;
   sum_24bits=(unsigned int32) uart_2_alinan[0]+(unsigned int32)256*uart_2_alinan[1]+(unsigned int32)256*256*uart_2_alinan[2];//gelen cevab�n decimal degeri hesaplaniyor
   if (uart_1_alinan[0]==0x33)
   {
       sum_24bits= max_value_24bit-sum_24bits;//guc degerleri ak�m artt�kca azal�yordu ak�m artarken okunan gucun de artmas� icin bu yaz�ld�
   }  
   //
  
 
      float32 sum_24bits_buf= ((float32)sum_24bits/(float32)bolen);
      float32 sum_24_real=((float32) carpan) * (sum_24bits_buf/AFE_scala);
       if (uart_1_alinan[0]==0x22)//ak�m �c�n belirli katsay�lar
        {
             if ((sum_24_real<3)) { sum_24_real=sum_24_real/1.2; }
             else if ( (sum_24_real>=3)&& (sum_24_real<4)) { sum_24_real=sum_24_real/1.236;}
             else if ( (sum_24_real>=4)&& (sum_24_real<5)) { sum_24_real=sum_24_real/1.239;}
             else if ( (sum_24_real>=5)&& (sum_24_real<6)) { sum_24_real=sum_24_real/1.2425;}
             else if ( (sum_24_real>=6)&& (sum_24_real<7)) { sum_24_real=sum_24_real/1.247;}
             else if ( (sum_24_real>=7)&& (sum_24_real<8)) { sum_24_real=sum_24_real/1.2515;}
             else if ( (sum_24_real>=8)&& (sum_24_real<9)) { sum_24_real=sum_24_real/1.256;}
             else if ( (sum_24_real>=9)&& (sum_24_real<10)) { sum_24_real=sum_24_real/1.26;}
             else if (sum_24_real >= 10) {sum_24_real=sum_24_real/1.26;}
        }
        //
        if (uart_1_alinan[0]==0x33)//power �c�n belirli katsay�lar ve bolgeleri
        {
            if ((sum_24_real<0.05)) { sum_24_real=sum_24_real/2.5; }
            else if ( (sum_24_real>=0.25)&& (sum_24_real<0.5)) { sum_24_real=sum_24_real/1.121;}
            else if ( (sum_24_real>=0.5)&& (sum_24_real<0.75)) { sum_24_real=sum_24_real/1.143;}
            else if ( (sum_24_real>=0.75)&& (sum_24_real<1)) { sum_24_real=sum_24_real/1.158;}
            else if ( (sum_24_real>=1)&& (sum_24_real<1.5)) { sum_24_real=sum_24_real/1.16;}
            else if ( (sum_24_real>=1.5)&& (sum_24_real<2)) { sum_24_real=sum_24_real/1.163;}
            else if ( (sum_24_real>=2)&& (sum_24_real<2.5)) { sum_24_real=sum_24_real/1.18;}
            else if ( (sum_24_real>=2.5)&& (sum_24_real<3.5)) { sum_24_real=sum_24_real/1.1905;}
            else if ( (sum_24_real>=3.5)&& (sum_24_real<4)) { sum_24_real=sum_24_real/1.197;}
            else if ( (sum_24_real>=4)&& (sum_24_real<5)) { sum_24_real=sum_24_real/1.207;}
            else if ( (sum_24_real>=5)&& (sum_24_real<6)) { sum_24_real=sum_24_real/1.2127;}
            else if ( (sum_24_real>=6)&& (sum_24_real<7)) { sum_24_real=sum_24_real/1.2267;}
            else if ( (sum_24_real>=7)&& (sum_24_real<8)) { sum_24_real=sum_24_real/1.2295;}
            else if ( (sum_24_real>=8)&& (sum_24_real<9)) { sum_24_real=sum_24_real/1.2408;}
            else if   (sum_24_real>=9){ sum_24_real=sum_24_real/1.252;}
        }
        //
        // Burada gerilim dalgalanmas� biraz fazla oldugu i�in onun i�in bir sey yapm�yoruz....
        //
      return sum_24_real;
  }
/////////////////////////////////////////////////////////////////////////

void Plug_Restart(void)
{
           output_low(pin_b7);//pic b7 nolu pine cirrus donanimsal reset icin ayarland� bu bacagi 0'a bir kere cekerek cirrusa donanimsal reset atiyoruz
           delay_ms(250);
           output_high(pin_b7);// donanimsal reset bacagini tekrardan eski konumuna getiriyoruz
           delay_ms(50);
           uart_change();//Reset sonrasi degisen baudrate h�z�n� (600) tekrardan ayarlanan degere (9600) ayarliyoruz
           delay_ms(50);
           HPF_ON();//kalibre icin gerekli high pass filter aktif ediliyor
           delay_ms(50);
           change_CG();//kalibre icin gerekli akim kazanci ayarlaniyor
           delay_ms(2000);
           output_high(pin_b4);// ROLE CEK�L�YOR priz uclarina 220V besleme saglaniyor
           relay_state=1;
           delay_ms(250);
           Start_Cirrus();//cirrus okumaya hazir hale getiriliyor  
}



