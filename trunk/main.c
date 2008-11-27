#include <16F876_CCS.h>
#include <constants.h>
#include <lcd_lab.c>
#include <teclat_lab.c>
#org 0x1F00, 0x1FFF void loader16F876(void) {}


//Variables
char sw6,sw7;
char bloc;
char bandera_pampallugues;
char comptador_hora;
char compta_analogic;
int fraccio_pampalluga;
int hora_en_segons;
char tarifa;
int ganancies_avui, kms_avui, consum_100km; //4 digits maxim.

int import;
char comptador_import;

/*
  tarifaN[0] = Preu baixada bandera
  tarifaN[1] = Preu/KM
  tarifaN[2] = Hora d'espera
  tarifa3[3] = Suplement horari nocturn
 */
int tarifa1[3], tarifa2[3], tarifa3[4];


//Capceleres de funcions
inline void printc_xy(int x, int y, char c);
void printf_xy(int x, int y, char * buffer);
inline static void scanf_xy (int x, int y, char *buffer, int len);
void get_time_input();
void printf_int(int x, int y, int s);
int get_preu_kbd();
static inline void print_tarifa(char i);
static inline void printf_import(int x, int y, char * s);
static inline void printf_hora(int x, int y, char * s);
static inline void lcd_clear();
static inline void led_bandera (char status);


//Codi
#int_global
void
ext_int()
{
  if (INTF == 1 && INTE == 1)
    {
      sw7 = ON;
      INTF = 0;
    }

  if (TMR0IF == 1 && TMR0IE == 1)
    {
      if (comptador_hora && (fraccio_de_segon++ > TICS_PER_SEGON))
	{
	  //Pensar a Actualitzar DATA!!, i si canviem de dia resetejar-------------------------*WARN*
	  //facturacio del dia i km's (i mitja consum/100km?)
	  hora_en_segons++;
	  fraccio_de_segon = 0;
	}

      if (bandera_pampallugues && (fraccio_pampalluga++ > TICS_PER_PAMPALLUGA))
	{
	  if ((PORTB & 0x20) == 0)
	    PORTB = PORTB | 0x20;
	  else
	    PORTB = PORTB & 0xDF;
	  fraccio_de_pampalluga = 0;
	}

      if ((PORTA & 0x20) == 1)
	{
	  compta_analogic = 1;
	}

      TMR0IF = 0;
    }

  if (ADIF == 1 && ADIE == 1)//---------------------------------------------------------------*FIXME*
    {

      /*
	SW6 = RA5 per comensar a comptar km's i combustible
	SW5/Generador_LogicT0CKL = RA4 -- Encoder per el combustible
	SW2:4 = RA1:3 per les tarifes T1,T2,T3
	SW1/Entrada Analogica = RA0
      */
      //Hem rebut final de conversio
      //Llegir resultat en els regs ADRESH:ADRESL
      
      if (compta_analogic == 1) //----
	{
	  if (comptador_import == 1) import++; //--------
	  kms_avui++;    //-------------------------------------------------------------------*FIXME*
	  consum_100km++;//---
	}
      
      ADIF = 0;
    }

}

static inline char int_to_flag (char i)
{
  return (1 << i);
}

static inline int
suplement_ascii_to_index (char k)
{
  switch (k)
    {
    case 'A':
      return SUPLEMENT_AERUPORT;
    case 'B':
      return SUPLEMENT_MALETA;
    case 'C':
      return SUPLEMENT_MOLL;
    case 'D':
      return SUPLEMENT_ASPECIAL;
    case '#':
      return SUPLEMENT_FIRA;
    case '*':
      return SUPLEMENT_GOS;
    }
}

static inline void
led_bandera (char status)
{
  switch (status)
    {
    case BANDERA_ON:
      PORTB = PORTB | 0x20;	
      break;
    case BANDERA_OFF:
      PORTB = PORTB & 0xDF;
      bandera_pampallugues = 0;
      break;
    case BANDERA_PAMPALLUGUES:
      bandera_pampallugues = 1;
      break;
    }
}

static inline void
lcd_clear ()
{
  char i, y;
  
  lcd_gotoxy(0,0);
  
  for (i = 0; i < LCD_SIZE_X; i++)
    for (y = 0; y < LCD_SIZE_Y; y++)
      lcd_putc('\0');
}

static inline void
printf_hora (int x, int y, char *s)
{
  //-------------------------------------------------------------------Rapinyar---------------*FIXME*
}

static inline void
printf_import (int x, int y, char *s)
{
  //-------------------------------------------------------------------Rapinyar---------------*FIXME*
}

static inline void
print_tarifa(char i)
{
  char activa_bandera = 0;

  if ((PORTB & 0x20) != 0)
    activa_bandera = 1;

  switch (i)
    {
    case 0: //No pinta res
      PORTB = 0x00;
      break;
    case 1: //Pinta un 1
      PORTB = 0x06;
      break;
    case 2: //Pinta un 2
      PORTB = 0x5A;
      break;
    case 3: //Pinta un 3
      PORTB = 0x4E;
      break;
    case 4: //Pinta una I.. es confon amb 1 --------------------------------------------------*FIXME*
      PORTB = 0x06;
      break;
    default:
      break;
    }

  //Restaurem l'estat de la bandera
  if (activa_bandera == 1)
    PORTB = PORTB | 0x20;
}


void
main()
{

  /*
    SW6 = RA5 per comensar a comptar km's i combustible
    SW5/Generador_LogicT0CLK = RA4 -- Encoder per el combustible
    SW2:4 = RA1:3 per les tarifes T1,T2,T3
    SW1/Entrada Analogica = RA0 -- Switch1 o Potenciometre???
   */

  //Pins
  TRISB = 0x01; //RB0 Input, RB1:7 Output
  TRISA = 0x3F; //RA0:5 Pins coma a Input
  ADCON1 = 0x2F; //RA0:4 Pins com a Digital
                 //RA4 Analog Input

  //Variables
  bandera_pampallugues = OFF;
  sw7 = OFF;
  bloc = REPOS;
  comptador_hora = ON;
  hora_en_segons = 0;
  tarifa = 0;
  ganancies_avui = 0;
  kms_avui = 0;
  consum_100km = 0;

  //Interrupcions//
  //Timer preescaler de 256:
  PSA = 0;
  PS0 = 1;
  PS1 = 1;
  PS2 = 1;
  TMR0 = 0; //Reestablim Contador
  TMR0IF = 0;
  T0IE = ON;

  INTE = OFF;
  GIE = ON;
  
  while (1)
    {
      switch(bloc)
	{
	case LLIURE:

	  lcd_clear();
	  
	  //----------------------DEBUG:
	  printf_xy(0,0,"Lliure");
	  //

	  led_bandera(BANDERA_ON);
	  print_tarifa(0); //Netejem display 7 segments (indic. tarifa)

	  sw7 = OFF;
	  while(sw7 != ON) printf_xy_hora (X_HORA, Y_HORA, hora_en_segons);	
	  sw7 = OFF;

	  //Mirem l'estat del sw1, si OFF anem a REPOS
	  //si ON anem a OCUPAT
	  if ((PORTA & 0x01) == 0)
	    {
	      print_tarifa(0);
	      bloc = REPOS;
	    }
	  else
	    {
	      //ANEM A OCUPAT, hem de llegir la tarifa
	      //i llavors saltar.

	      //Llegim switchs
	      if (PORTA & 0x02) tarifa = 2;
	      else if (PORTA & 0x03) tarifa = 3;
	      else if (PORTA & 0x04) tarifa = 4;
	      else tarifa = 0;
	      bloc = OCUPAT;
	      
	      //Hem de posar una tarifa obligatoriament
	      if (tarifa == 0)
		{
		  printf_xy(0,1,"Introdueix tarifa!");
		  do {
		    if (PORTA & 0x02) tarifa = 2;
		    else if (PORTA & 0x03) tarifa = 3;
		    else if (PORTA & 0x04) tarifa = 4;
		  } while (tarifa == 0);
		  printf_xy(0,1,"                  ");
		}
	      
	      print_tarifa(tarifa);
	    }

	  led_bandera(BANDERA_OFF);
	  
	  break;

	case REPOS:
	  char intent = 0;
	  char c;
	  char buffer[NCHARS_PASSWD];
	  int i;
	  
	  lcd_clear();
	  printf_xy(0,1,"Entra password:");

	  while (bloc == REPOS)
	    {	    
	      //Depenent de com estigui sw1, farem que si
	      //ens equivoquem de password, anem a LLIURE o
	      //ens quedem a REPOS per reintentar.
	      //SW1 = ON -> Possibilitat de reintents
	      //SW1 = OFF -> Nomes un intent

	      if (intent > 0)
		printf_xy(0,0,"\0\0\0\0\0\0\0\0\0\0");
	      
	      //Get Passwd
	      for (i=0; i<NCHARS_PASSWD; i++)
		{
		  do c = keyscan();
		  while (c == 0x80);
		  
		  buffer[i] = c;
		  lcd_gotoxy(0+i,0);
		  lcd_putc('*');
		}
	      
	      for (i=0; i<NCHARS_PASSWD && buffer[i] == passwd[i]; i++);
	      
	      if (i == (NCHARS_PASSWD-1))
		  bloc = CONTROLS;
	      else 
		{
		  if (sw1 == OFF)
		    bloc = LLIURE;
		  else
		    {
		      printf_xy(0,0,"Try again!");
		      intent++;
		    }
		}
	    }
	  break;
	  
	case CONTROLS:

	  char i, tmp[NCHARS_PASSWD];
	  lcd_clear();

	  //Set Passwd
	  //Donem l'opcio de no canviar el passwd
	  //pressionant directament el sw7
	  sw7 = OFF;
	  i = 0;
	  printf_xy(0,1,"Nou passwd:");

	  while(!sw7 && i < NCHARS_PASSWD)
	    {
	      do c = keyscan(); //-----------------------------------------------------------*WARN*
	      while (c == 0x80 && !sw7);
		  
	      if (!sw7)
		{
		  tmp[i] = c;
		  lcd_gotoxy(0+i,0);
		  lcd_putc(c);
		}
		  i++;
	    }
	  if (i == NCHARS_PASSWD)
	    for (i=0; i<NCHARS_PASSWD; i++) passwd[i] = tmp[i];

	  lcd_clear();
	  printf_xy(0,1,"Nova hora:");

	  //Set Time
	  comptador_hora = OFF;
	  get_time_input();
	  comptador_hora = ON;

	  //Set Preus
	  lcd_clear();
	  printf_xy(0,1,"T1 - B.b.");
	  tarifa1[0] = get_preu_kbd();

	  lcd_clear();
	  printf_xy(0,1,"T1 - P/Km");
	  tarifa1[1] = get_preu_kbd();

	  lcd_clear();
	  printf_xy(0,1,"T1 - H.Espera");
	  tarifa1[2] = get_preu_kbd();
	  
	  lcd_clear();
	  printf_xy(0,1,"T2 - B.b.");
	  tarifa2[0] = get_preu_kbd();

	  lcd_clear();
	  printf_xy(0,1,"T2 - P/Km");
	  tarifa2[1] = get_preu_kbd();

	  lcd_clear();
	  printf_xy(0,1,"T2 - H.Espera");
	  tarifa2[2] = get_preu_kbd();

	  lcd_clear();
	  printf_xy(0,1,"T3 - B.b.");
	  tarifa3[0] = get_preu_kbd();

	  lcd_clear();
	  printf_xy(0,1,"T3 - P/Km");
	  tarifa3[1] = get_preu_kbd();

	  lcd_clear();
	  printf_xy(0,1,"T3 - H.Espera");
	  tarifa3[2] = get_preu_kbd();

	  lcd_clear();
	  printf_xy(0,1,"T3 - S. Noct.");
	  tarifa3[3] = get_preu_kbd();

	  lcd_clear();
	  printf_xy(0,1,"Fact. avui:");
	  printf_int(0,0,ganancies_avui);
	  sw7 = OFF;
	  while(!sw7) sleep();

	  lcd_clear();
	  printf_xy(0,1,"Kms avui:");
	  printf_int(0,0,kms_avui);
	  sw7 = OFF;
	  while(!sw7) sleep();
	  
	  lcd_clear();
	  printf_xy(0,1,"Consum 100km:");
	  printf_int(0,0,consum_100km);
	  sw7 = OFF;
	  while(!sw7) sleep();

	  bloc = REPOS;
	  break;

	case OCUPAT:
	  //ROBERT
	  lcd_clear ();
	  //print_tarifa (tarifa); <--Ja t'ho poso des de LLIURE.
	  import = PREU_FIX_BAIXADA_DE_BANDERA;// AQUEST PREU ES SEGONS LA TARIFA!!!!!!-----*FIXME*
	  
	  comptador_import = ON;  /* Demanem a l'RSI que incrementi 'import' */
	  sw7 = OFF;
	  while (!sw7)
	    {
	      printf_xy_import (X_IMPORT, Y_IMPORT, import);
	      printf_xy_hora (X_HORA, Y_HORA, hora_en_segons);
	    }
	  comptador_import = OFF;
	  bloc = IMPORT;

	  break;

	case IMPORT:
	  //ROBERT
	  char suplements_emprats = 0;
	  led_bandera (BANDERA_PAMPALLUGUES);
	  tarifa = 4; //Posara una I al display de 7 segments ---(es confon amb 1)----------*FIXME*
	  print_tarifa (tarifa);

	  //Mirem estat de sw1 (pujada de bandera)
	  while ((PORTA & 0x01) == 0)
	    {
	      char k;
	      k = suplement_ascii_to_index (keyScan ());

	      if (k != SUPLEMENT_MALETA)
		{
                  char j;
		  /* La gràcia d'aquesta collonada és que `suplements_que_ja_hem_activat'
		     només ens ocupa un byte (el codi de flagificació ja ocupa més d'un
		     byte, però de memòria de codi anem sobrats).  */
                  j = int_to_flag (k);
		  if ((suplements_emprats & j) != 0)
		    continue;
		  else
		    suplements_emprats |= j;
		}
	      import += suplement_index_to_preu[k];

	      k = 0;
	      while (k != 0x80) k = keyScan();
	    }
	  break;

	default:
	  break;
	}
    }
}

/**Pinta una lletra a una posicio de sa pantalla*/
inline void printc_xy(int x, int y, char c)
{
  lcd_gotoxy(x,y);
  lcd_putc(c);
}

int get_preu_kbd()
{
  char x,i;
  char preu[5] = "00.00";
  uint16_t retorn;

 ini_funcio_gpreu:
  i = 0x80;
  x = 0;

  printf_xy(0,1,preu);

  while (x < 5)
    {
      lcd_gotoxy(x,0);
      
      while (i == 0x80)
	i = keyScan();
      
      //Opcio de rectificar
      if (i == 'C')
	{
	  if (x==3) x -= 2;
	  else if (x != 0) x--;
	  printc_xy(x,0,'0');
	}
      else
	{
	  //Si error, ometem
	  if (i < '0' || i > '9')
	    goto fi_bucle_gpreu;
	  
	  printc_xy(x,0,i);
	  x++;
	  
	  if (x==2)
	    x++;
	}
    
    fi_bucle_gpreu:
      while (keyScan () != 0x80);
    }

  sw7 = OFF;

 espera_confirmacio: //Per sortir o pitjem * o pitjem sw7.
  
  while (i == 0x80 || !sw7) i = keyScan (); //ALERTA QUE NO SE SOLAPI--------------------------*WARN*
  
  if (sw7) goto end_gpreu;

  if (i == '*') 
    {
      preu[0] = '0';
      preu[1] = '0';
      preu[2] = '.';
      preu[3] = '0';
      preu[4] = '0';
      goto ini_funcio_gpreu;
    }
  i = 0x80;
  goto espera_confirmacio;

 end_gpreu:  
  //Acabarem retornant el preu
  sw7 = OFF;
  retorn = 0;
  scanf_xy(0,1,preu,5);
  retorn = preu[0]*1000 + preu[1]*100 + preu[3]*10 + preu[4] - (53328); //-65536-tamany INT---*FIXME*
}

void
printf_xy (int x, int y, char *buffer)
{
  //Pre: x : {0,15}
  //     y : {0,1}
   
  int i;
  lcd_gotoxy (x, y);
  for (i = 0; buffer[i] != '\0'; i++)
    lcd_putc (buffer[i]);
}


void 
get_time_input ()
{
  //Aprofitament de codi de la practica anterior
  /*Bucle per introduïr dades al cronometre, quan haguem acabat
    polsarem el sw7. Si pitjem la tecla C es
    fan net les dades introduides.
    El temps introduit sortira a la banda esquerra de la pantalla, 
    i si polsem una tecla incorrecte sortira un missatge a la 
    dreta de la segona fila*/

  char ant, i, x;
 ini_funcio_gtime:

  x = 0;
  printf_xy(0,0,hora);

  while (x < 5)
    {
      lcd_gotoxy (x, 0);
      i = 0x80;

      while (i == 0x80)
	i = keyScan ();
      printc_xy (15, 1, ' ');

      //Si polsem C comensem de nou o netejem
      //Si no es un nombre: Error
      //Si massa gran: Big
      if (i == 'C')
	{
	  //Quan ens pitjin una 'C':
	  //Si volem eliminar ultim caracter, descomentar seg 3 linies
	   if (x==3) x = x-2;
	   else if (x!=0) x--;
	   printc_xy(x,0,'0');

	  //Si volem començar de nou, descomentar les 2 seguents linies
	  //x = 0;
	  // lcd_clear();
	}
      else
	{
	  if (i < '0' || i > '9')
	    {
	      printc_xy(15,1,'E');
	      goto big_o_error;
	    }
	      
	  //Hem de fer que no es puguin fer mes de 23:59

	  switch (x)
	    {
	    case 0: //Comprovacio de la desena d'hora
	      if (i > '2')
		{
		  printc_xy(15,1,'B');
		  goto big_o_error; 
		}
	      break;

	    case 1: //Comprovacio de la decima d'hora
	      aux = lcd_getc(x-1,0);
	      if (aux == 2 && i > 3)
		{
		  printc_xy(15,1,'B');
		  goto big_o_error;
		}
	      break;

	    case 3:  //Comprovacio de desena de minut
	      if (i > '5')
		{
		printc_xy(15,1,'B');
		goto big_o_error;
		}
	      break;
	    
	    default:
	      break;
	    }
	  
	  printc_xy(x,0,i);
	  x++;

	  if (x==2)
	    x++;
	}

	big_o_error:
      while (keyScan () != 0x80);
    }

  //Si no estem d'acord, podem tornar a introduir hora
  sw7 = OFF;

 espera_confirmacio: //Per sortir o pitjem * o pitjem sw7.
  
  while (i == 0x80 || !sw7) i = keyScan (); //ALERTA QUE NO SE SOLAPI--------------------------*WARN*
  
  if (sw7) goto end_gtime;

  if (i == '*') 
    {
      hora[0] = '0';
      hora[1] = '0';
      hora[2] = ':';
      hora[3] = '0';
      hora[4] = '0';
      goto ini_funcio_gtime;
    }
  i = 0x80;
  goto espera_confirmacio;

 end_gtime:  
  //Acabarem guardant la hora
  sw7 = OFF;
  scanf_xy(0,1,hora,5);
}


/** Agafa 'len' bytes del display de la posico x,y
  * i els posa dins el *buffer
  */
inline static void
scanf_xy  (int x, int y, char *buffer, int len)
{
  int i;
  for (i = 0; i < len; i++)
    buffer[i] = lcd_getc(x + i, y);
}

void printf_int(int x, int y, int s)
{
  char d[4];
  
  d[3] = (s % 10) + '0';
  s /= 10;
  d[2] = (s % 6) + '0';
  s /= 6;
  d[1] = (s % 10) + '0';
  d[0] = (s / 10) + '0';

  printf_xy(x,y,d);
}
