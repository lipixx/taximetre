///////////////////////////////////////////////////////////////////////
/// PIC 16F87X HEADER FILE											///
/// Versio 5 setembre 2008											///
/// Autor: Pere Marès, Joan Climent, Joan Vidos, Joel Sala			///
/// DEPARTAMENT d'ESAII												///
///////////////////////////////////////////////////////////////////////


// HEADER FILE

#device PIC16F876 
#list 
/* Predefined:
  char W;
  char INDF, TMR0, PCL, STATUS, FSR, PORTA, PORTB;
  char OPTION, TRISA, TRISB;
  char PCLATH, INTCON;
  bit PS0, PS1, PS2, PSA, T0SE, T0CS, INTEDG, RBPU_;
  bit Carry, DC, Zero_, PD, TO, RP0, RP1, IRP;
  bit RBIF, INTF, T0IF, RBIE, INTE, T0IE, GIE;
  bit PA0, PA1;  // PCLATH
*/

#BYTE TMR0	  = 1
#BYTE PORTA	  = 5
#BYTE PORTB	  = 6
#BYTE PORTC	  = 7 

#BYTE TRISA		  = 0x85
#BYTE TRISB		  = 0x86
#BYTE TRISC		  = 0x87 
#BYTE OPTION_REG  = 0x81
#BYTE INTCON	  = 0x0B		//mapped_into_all_banks

#BYTE PIR1    = 12
#BYTE PIR2    = 13
#BYTE TMR1L   = 14
#BYTE TMR1H   = 15
#BYTE T1CON   = 16
#BYTE TMR2    = 17
#BYTE T2CON   = 18
#BYTE SSPBUF  = 19
#BYTE SSPCON  = 20
#BYTE CCPR1L  = 21
#BYTE CCPR1H  = 22
#BYTE CCP1CON = 23
#BYTE RCSTA   = 24
#BYTE TXREG   = 25
#BYTE RCREG   = 26
#BYTE CCPR2L  = 27
#BYTE CCPR2H  = 28
#BYTE CCP2CON = 29
#BYTE ADRESH  = 30
#BYTE ADCON0  = 31


#BYTE PIE1    = 0x8C
#BYTE PIE2    = 0x8D
#BYTE PCON    = 0x8E

#BYTE SSPCON2 = 0x91
#BYTE PR2     = 0x92
#BYTE SSPADD  = 0x93
#BYTE SSPSTAT = 0x94

#BYTE TXSTA   = 0x98
#BYTE SPBRG   = 0x99

#BYTE ADRESL  = 0x9E
#BYTE ADCON1  = 0x9F

#BYTE EEDATA  = 0x10C
#BYTE EEADR   = 0x10D
#BYTE EEDATH  = 0x10E
#BYTE EEADRH  = 0x10F

#BYTE EECON1  = 0x18C
#BYTE EECON2  = 0x18D

#BIT  PORTA_0  = 5.0
#BIT  PORTA_1  = 5.1
#BIT  PORTA_2  = 5.2
#BIT  PORTA_3  = 5.3
#BIT  PORTA_4  = 5.4
#BIT  PORTA_5  = 5.5
#BIT  PORTA_6  = 5.6
#BIT  PORTA_7  = 5.7 

#BIT  PORTB_0  = 6.0
#BIT  PORTB_1  = 6.1
#BIT  PORTB_2  = 6.2
#BIT  PORTB_3  = 6.3
#BIT  PORTB_4  = 6.4
#BIT  PORTB_5  = 6.5
#BIT  PORTB_6  = 6.6
#BIT  PORTB_7  = 6.7

#BIT  PORTC_0  = 7.0
#BIT  PORTC_1  = 7.1
#BIT  PORTC_2  = 7.2
#BIT  PORTC_3  = 7.3
#BIT  PORTC_4  = 7.4
#BIT  PORTC_5  = 7.5
#BIT  PORTC_6  = 7.6
#BIT  PORTC_7  = 7.7 

// OPTION_REGISTER
#BIT  PS0    = 0X81.0
#BIT  PS1    = 0X81.1
#BIT  PS2    = 0X81.2
#BIT  PSA    = 0X81.3
#BIT  TOSE   = 0X81.4
#BIT  TOCS   = 0X81.5
#BIT  INTEDG = 0X81.6
#BIT  RBPU   = 0X81.7

//INTCON
#BIT  RBIF	  = 0x0B.0
#BIT  INTF	  = 0x0B.1
#BIT  TMR0IF  = 0x0B.2
#BIT  RBIE	  = 0x0B.3
#BIT  INTE	  = 0x0B.4
#BIT  TMR0IE  = 0x0B.5
#BIT  PEIE	  = 0x0B.6
#BIT  GIE	  = 0x0B.7

#BIT  TMR1IF  = 12.0
#BIT  TMR2IF  = 12.1
#BIT  CCP1IF  = 12.2
#BIT  SSPIF   = 12.3
#BIT  TXIF    = 12.4
#BIT  RCIF    = 12.5
#BIT  ADIF    = 12.6

#BIT  CCP2IF  = 13.0
#BIT  BCLIF   = 13.3
#BIT  EEIF    = 13.4

#BIT  TMR1ON  = 16.0
#BIT  TMR1CS  = 16.1
#BIT  T1SYNC_ = 16.2
#BIT  T1OSCEN = 16.3
#BIT  T1CKPS0 = 16.4
#BIT  T1CKPS1 = 16.5

#BIT  T2CKPS0 = 18.0
#BIT  T2CKPS1 = 18.1
#BIT  TMR2ON  = 18.2
#BIT  TOUTPS0 = 18.3
#BIT  TOUTPS1 = 18.4
#BIT  TOUTPS2 = 18.5
#BIT  TOUTPS3 = 18.6

#BIT  SSPM0   = 20.0
#BIT  SSPM1   = 20.1
#BIT  SSPM2   = 20.2
#BIT  SSPM3   = 20.3
#BIT  CKP     = 20.4
#BIT  SSPEN   = 20.5
#BIT  SSPOV   = 20.6
#BIT  WCOL    = 20.7

#BIT  CCP1M0  = 23.0
#BIT  CCP1M1  = 23.1
#BIT  CCP1M2  = 23.2
#BIT  CCP1M3  = 23.3
#BIT  CCP1Y   = 23.4
#BIT  CCP1X   = 23.5

#BIT  RX9D    = 24.0
#BIT  OERR    = 24.1
#BIT  FERR    = 24.2
#BIT  ADDEN   = 24.3
#BIT  CREN    = 24.4
#BIT  SREN    = 24.5
#BIT  RX9     = 24.6
#BIT  SPEN    = 24.7

#BIT  CCP2M0  = 29.0
#BIT  CCP2M1  = 29.1
#BIT  CCP2M2  = 29.2
#BIT  CCP2M3  = 29.3
#BIT  CCP2Y   = 29.4
#BIT  CCP2X   = 29.5

#BIT  ADON    = 31.0
#BIT  GO      = 31.2
#BIT  CHS0    = 31.3
#BIT  CHS1    = 31.4
#BIT  CHS2    = 31.5
#BIT  ADCS0   = 31.6
#BIT  ADCS1   = 31.7

#BIT  PSPMODE = 0x89.4
#BIT  IBOV    = 0x89.5
#BIT  OBF     = 0x89.6
#BIT  IBF     = 0x89.7

#BIT  TMR1IE  = 0x8C.0
#BIT  TMR2IE  = 0x8C.1
#BIT  CCP1IE  = 0x8C.2
#BIT  SSPIE   = 0x8C.3
#BIT  TXIE    = 0x8C.4
#BIT  RCIE    = 0x8C.5
#BIT  ADIE    = 0x8C.6

#BIT  CCP2IE  = 0x8D.0
#BIT  BCLIE   = 0x8D.3
#BIT  EEIE    = 0x8D.4

#BIT  BOR_    = 0x8E.0
#BIT  POR_    = 0x8E.1

#BIT  SEN     = 0x91.0
#BIT  RSEN    = 0x91.1
#BIT  PEN     = 0x91.2
#BIT  RCEN    = 0x91.3
#BIT  ACKEN   = 0x91.4
#BIT  ACKDT   = 0x91.5
#BIT  ACKSTAT = 0x91.6
#BIT  GCEN    = 0x91.7

#BIT  BF      = 0x94.0
#BIT  UA      = 0x94.1
#BIT  RW_     = 0x94.2
#BIT  S       = 0x94.3
#BIT  P       = 0x94.4
#BIT  DA_     = 0x94.5
#BIT  CKE     = 0x94.6
#BIT  SMP     = 0x94.7

#BIT  TX9D    = 0x98.0
#BIT  TRMT    = 0x98.1
#BIT  BRGH    = 0x98.2
#BIT  SYNC    = 0x98.4
#BIT  TXEN    = 0x98.5
#BIT  TX9     = 0x98.6
#BIT  CSRC    = 0x98.7

#BIT  PCFG0   = 0x9F.0
#BIT  PCFG1   = 0x9F.1
#BIT  PCFG2   = 0x9F.2
#BIT  PCFG3   = 0x9F.3
#BIT  ADFM    = 0x9F.7

#BIT  RD      = 0x18C.0
#BIT  WR      = 0x18C.1
#BIT  WREN    = 0x18C.2
#BIT  WRERR   = 0x18C.3
#BIT  EEPGD   = 0x18C.7
