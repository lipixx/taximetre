#include "include/16F876_CCS.h"
#include "include/constants.h"
#include "lcd_lab.c"
#include "teclat_lab.c"
#org 0x1F00, 0x1FFF void loader16F876(void) {}


//Variables
unsigned char fracc_de_fracc_de_segon;

//-----------------------------------------------------------------------------------------*FIXME*#
#define sw1 (PORTA & 0x01)
#define sw2 (PORTA & 0x02)
#define sw3 (PORTA & 0x04)
#define sw4 (PORTA & 0x08)
#define sw5  PORTA_4 //(PORTA & 0x10)
#define sw6 (PORTA & 0x20)
short sw7;
short am_pm;
char bloc;
short bandera_pampallugues;
short comptador_hora;
short compta_analogic;
uint16_t hora_en_segons;

char tarifa;
short index_tarifa1_2;

int ganancies_avui, kms_avui, consum_100km;	//4 digits maxim, amb 8 bits kk
uint16_t fraccio_de_segon, fraccio_de_pampalluga;
int import;
char comptador_import;


/*
  tarifaN[0] = Preu baixada bandera
  tarifaN[1] = Preu/KM
  tarifaN[2] = Hora d'espera
  tarifa3[3] = Suplement horari nocturn
 */
uint16_t tarifa1_2[2][3];
uint16_t tarifa3[4];

//Capceleres de funcions
#define printf_xy(x,y,s)   { lcd_gotoxy(x,y); lcd_putc(s); }
static inline void scanf_xy (char x, char y, char *buffer, char len);
static void get_time_input ();
static int get_preu_kbd ();
static inline void print_tarifa (char i);
static inline void printf_import (int x, int y, char *s);
static inline void printf_hora (int x, int y, char *s);
static void lcd_clear ();
static inline void led_bandera (char status);
inline static void printf_int (int x, int y, int s);
inline void suplement_ascii_to_index(char k);


//Codi
#int_global
void
ext_int ()
{
  if (INTF == 1 && INTE == 1)
    {
      sw7 = ON;
      INTF = 0;
    }

  if (TMR0IF == 1 && TMR0IE == 1)
    {				//START TMR0 Interrupt
      if (fracc_de_fracc_de_segon++ == 255)
	{
	  if (comptador_hora && (fraccio_de_segon++ > TICS_PER_SEGON))
	    {
	      //Pensar a Actualitzar DATA!!, i si canviem de dia resetejar-------------------------*WARN*
	      //facturacio del dia i km's (i mitja consum/100km?) bit de AM/PM!!!
	      
	      if (!(hora_en_segons%43200))
		//Si han passat 12h canvi am_pm
		am_pm++;

	      if (am_pm) //Si som les 00:00hAM, nou dia
		{
		  ganancies_avui = 0;
		  kms_avui = 0;
		  //falta res?
		}
	      hora_en_segons++;
	      fraccio_de_segon = 0;
	    }

	  if (bandera_pampallugues
	      && (fraccio_de_pampalluga++ > TICS_PER_PAMPALLUGA))
	    {
	      if ((PORTB & 0x80) == 0)
		PORTB = PORTB | 0x80;
	      else
		PORTB = PORTB & 0x7F;
	      fraccio_de_pampalluga = 0;
	    }

	  if (sw6 == 1)		//Comprovam el sw6, si activat començem a comptar
	    {
	      compta_analogic = ON;
	    }
	}
      TMR0IF = 0;

    }				//END TMR0 Interrupt

  if (ADIF == 1 && ADIE == 1)	//---------------------------------------------------------------*FIXME*
    {

      /*
         SW6 = RA5 per comensar a comptar km's i combustible
         SW5/Generador_LogicT0CKL = RA4 -- Encoder per el combustible
         SW2:4 = RA1:3 per les tarifes T1,T2,T3
         SW1/Entrada Analogica = RA0
       */
      //Hem rebut final de conversio
      //Llegir resultat en els regs ADRESH:ADRESL

      if (compta_analogic == ON)	//----
	{
	  if (comptador_import == 1)
	    import++;		//--------
	  kms_avui++;		//-------------------------------------------------------------------*FIXME*
	  consum_100km++;	//---
	}

      ADIF = 0;
    }

}

static inline char
int_to_flag (char i)
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
  //El led D7 es l'RB7
  switch (status)
    {
    case BANDERA_ON:
      PORTB = PORTB | 0x80;
      break;
    case BANDERA_OFF:
      PORTB = PORTB & 0x7F;
      bandera_pampallugues = 0;
      break;
    case BANDERA_PAMPALLUGUES_:
      bandera_pampallugues = 1;
      break;
    }
}

static void
lcd_clear ()
{
  unsigned char x, y;
  for (y = 0; y < LCD_SIZE_Y; y++)
    { 
      lcd_gotoxy (0, y);
      for (x = 0; x < LCD_SIZE_X; x++)
	lcd_putc (' ');
    }
}

static inline void
printf_xy_hora (int x, int y, char *s)
{
  //  printf_xy (x, y, "HH:MM");//-------------------------------------------------------------------Rapinyar---------------*FIXME*
}

static inline void
printf_xy_import (int x, int y, char *s)
{
  //  printf_xy (x, y, "00,00");//-------------------------------------------------------------------Rapinyar---------------*FIXME*
}

static inline void
print_tarifa (char i)
{
  short activa_bandera = OFF;

  if ((PORTB & 0x80) != 0)
    activa_bandera = ON;

  PORTB = taula_print_tarifa[i];

  //Restaurem l'estat de la bandera
  if (activa_bandera)
    PORTB = PORTB | 0x80;
}


void
main ()
{
  /*
     SW6 = RA5 per comensar a comptar km's i combustible
     SW5 LLIURE!!  L'assignem a canvi d'estat en sortir de repòs
     SW2:4 = RA1:3 per les tarifes T1,T2,T3
     SW1/Entrada Analogica = RA0 boia
   */

  //Pins
  TRISB = 0x01;			//RB0 Input, RB1:7 Output
  TRISA = 0x3F;			//RA0:5 Pins coma a Input
  ADCON1 = 0x2F;		//RA0:4 Pins com a Digital
  //RA4 Analog Input

  lcd_init ();

  /* Per algun motiu, el primer caràcter que enviem es perd.  */
  lcd_putc (' ');

  //Variables
  fracc_de_fracc_de_segon = 0;
  bandera_pampallugues = OFF;
  sw7 = OFF;
  bloc = REPOS;
  comptador_hora = ON;
  hora_en_segons = 0;
  tarifa = 0;
  ganancies_avui = 0;
  kms_avui = 0;
  consum_100km = 0;
  fraccio_de_segon = 0;
  fraccio_de_pampalluga = 0;
  am_pm = 0;

  //Interrupcions//
  //Timer preescaler de 256:
  PSA = 0;
  PS0 = 1;
  PS1 = 1;
  PS2 = 1;
  TMR0 = 0;			//Reestablim Contador
  TMR0IF = 0;
  TMR0IE = ON;

  INTE = OFF;
  GIE = ON;

  while (1)
    {
      switch (bloc)
	{
	case LLIURE:
	 	  
	  lcd_clear ();

	  led_bandera (BANDERA_ON);
	  print_tarifa (0);	//Netejem display 7 segments (indic. tarifa)
	
	  sw7 = OFF;
	  INTE = ON;
	  while (sw7 != ON) //Apagar la bandereta
	    printf_xy_hora (X_HORA, Y_HORA, hora_en_segons);
	  sw7 = OFF;
	  INTE = OFF;
	  
	  //Mirem l'estat del sw5, si OFF anem a REPOS
	  //si ON anem a OCUPAT
	  if (sw5 == OFF)
	    {
	      print_tarifa (0);
	      bloc = REPOS;
	    }
	  else
	    {
	      //ANEM A OCUPAT, hem de llegir la tarifa
	      //i llavors saltar.
	      
	      //Llegim switchs per obtenir tarifa, obligatori posar-la
	      tarifa = 0;
	      do {
		if (sw2)
		  tarifa = 1;			//------------- S'Ha de setejar tambe index_tarifa_1_2 si escau
		else if (sw3)
		  tarifa = 2;
		else if (sw4)
		  tarifa = 3;
	      } while (tarifa == 0);
	      
	      bloc = OCUPAT;
	      lcd_clear();
	      print_tarifa (tarifa);
	    }
       	  led_bandera (BANDERA_OFF);
	  break;

    case REPOS:
      {
	char c;
	char buffer[NCHARS_PASSWD];
	char i;
	
	while (bloc == REPOS)
	  {
	    //Depenent de com estigui sw5, farem que si
	    //ens equivoquem de password, anem a LLIURE o
	    //ens quedem a REPOS per reintentar.
	    //SW5 = ON -> Possibilitat de reintents
	    //SW5 = OFF -> Nomes un intent
	    
	    lcd_clear ();
	    printf_xy (0, 1, "Password:");
	
	    //Get Passwd
	    lcd_gotoxy (0, 0);
	    for (i = 0; i < NCHARS_PASSWD; i++)
	      {
		c = keyScan ();
		buffer[i] = c;
		lcd_putc ('*');
	      }
	    
	    for (i = 0; i < NCHARS_PASSWD && buffer[i] == passwd[i]; i++);
	    
	    if (i == NCHARS_PASSWD)
	      bloc = CONTROLS;
	    else
	      {
		if (sw5 == ON)
		  bloc = LLIURE;
		else
		  {
		    printf_xy(0,0,"Try again!");
		    printf_xy(0,1,"Press Sw7");

		    sw7 = OFF;
		    INTE = ON;
		    while (!sw7);
		    sw7 = OFF;
		    INTE = OFF;
		  }
	      }
	  }
      }
      break;

	case CONTROLS:
	  {
	    char c, i, tmp[NCHARS_PASSWD];
	    lcd_clear ();

	    //Set Passwd
	    //Donem l'opcio de no canviar el passwd
	    //pressionant directament el *
	    i = 0;
	    printf_xy(0,1,"New Pwd o '*':");

	    while (c != '*' && i < NCHARS_PASSWD)
	      {
		c = keyScan ();

		if (c == '*') continue;
		
		tmp[i] = c;
		printf_xy(i,0,c);
		i++;
	      }

	    if (i == NCHARS_PASSWD)
	      for (i = 0; i < NCHARS_PASSWD; i++)
		passwd[i] = tmp[i];
	    
	    //Set Time
	    lcd_clear ();
	    printf_xy (0, 1, "Set hora:");
	    comptador_hora = OFF;
	    get_time_input ();
	    comptador_hora = ON;
#if 0
	    //Set Preus
	    lcd_clear ();
	    printf_xy (0, 1, "T1 - B.b.:");
	    tarifa1_2[0][0] = get_preu_kbd ();

	    lcd_clear ();
	    printf_xy (0, 1, "T1 - P/Km:");
	    tarifa1_2[0][1] = get_preu_kbd ();

	    lcd_clear ();
	    printf_xy (0, 1, "T1 - H.Espera:");
	    tarifa1_2[0][2] = get_preu_kbd ();

	    lcd_clear ();
	    printf_xy (0, 1, "T2 - B.b.:");
	    tarifa1_2[1][0] = get_preu_kbd ();

	    lcd_clear ();
	    printf_xy (0, 1, "T2 - P/Km:");
	    tarifa1_2[1][1] = get_preu_kbd ();

	    lcd_clear ();
	    printf_xy (0, 1, "T2 - H.Espera:");
	    tarifa1_2[1][2] = get_preu_kbd ();

	    lcd_clear ();
	    printf_xy (0, 1, "T3 - B.b.:");
	    tarifa3[0] = get_preu_kbd ();

	    lcd_clear ();
	    printf_xy (0, 1, "T3 - P/Km:");
	    tarifa3[1] = get_preu_kbd ();

	    lcd_clear ();
	    printf_xy (0, 1, "T3 - H.Espera:");
	    tarifa3[2] = get_preu_kbd ();

	    lcd_clear ();
	    printf_xy (0, 1, "T3 - S. Noct:");
	    tarifa3[3] = get_preu_kbd ();
#endif
	    //Comencem a mostrar dades estadistiques al taxista
	    lcd_clear ();
	    printf_xy (0, 1, "Fact. avui:");
	    printf_int (0, 0, ganancies_avui);
	    INTE = ON;
	    sw7 = OFF;
	    while (!sw7);

	    lcd_clear ();
	    printf_xy (0, 1, "Kms avui:");
	    printf_int (0, 0, kms_avui);
	    sw7 = OFF;
	    while (!sw7);

	    lcd_clear ();
	    printf_xy (0, 1, "Consum 100km:");
	    printf_int (0, 0, consum_100km);
	    sw7 = OFF;
	    while (!sw7);

	    INTE = OFF;
	    bloc = REPOS;
	    break;
	  }

	case OCUPAT:
	  //ROBERT
	  lcd_clear ();
	  //print_tarifa (tarifa); <--Ja t'ho poso des de LLIURE.
	switch (tarifa)	//PREU_FIX_BAIXADA_DE_BANDERA;// AQUEST PREU ES SEGONS LA TARIFA!!!!!!-----*FIXME*
	{
	case 3:
		import = tarifa3[0];
	break;
	default:
		import = tarifa1_2[index_tarifa1_2][0];
	break;
	}  
	  comptador_import = ON;	/* Demanem a l'RSI que incrementi 'import' */
	  sw7 = OFF;
	  while (!sw7)
	    {
	      printf_xy_import (X_IMPORT, Y_IMPORT, import);
	      printf_xy_hora (X_HORA, Y_HORA, hora_en_segons);
	    }
	  comptador_import = OFF;
	  bloc = IMPORT_;

	  break;

	case IMPORT_:
	  {
	    //ROBERT
	    char suplements_emprats = 0;
	    led_bandera (BANDERA_PAMPALLUGUES_);
	    tarifa = 4;		//Posara una I al display de 7 segments ---(es confon amb 1)----------*FIXME*
	    print_tarifa (tarifa);

	    //Mirem estat de sw1 (pujada de bandera)
	    while ((PORTA & 0x01) == 0)
	      {
		char k;

       		k = suplement_ascii_to_index (keyScan ());

		if (k != SUPLEMENT_MALETA)
		  {
		    char j;
		    /* La gracia d'aquesta collonada és que `suplements_que_ja_hem_activat'
		       només ens ocupa un byte (el codi de flagificació ja ocupa més d'un
		       byte, però de memòria de codi anem sobrats).  */
		    j = int_to_flag (k);
		    if ((suplements_emprats & j) != 0)
		      continue;
		    else
		      suplements_emprats |= j;
		  }
		import += suplement_index_to_preu[k];

		k = keyScan ();
	      }
	  }
	  break;

	default:
	  	printf_xy(0,0,"bloc?");
		break;
	}
    }
}

static int
get_preu_kbd ()
{
  char x, i;
  uint16_t retorn;

 ini_funcio_gpreu:
  i = 0x80;
  x = 0;
  
  printf_xy (0, 0, "00,00");
  
  while (x < 5)
    {
      lcd_gotoxy (x, 0);
      i = keyScan ();
      
      //Opcio de rectificar amb la tecla C
      if (i == 'C')
	{
	  if (x == 3)
	    x -= 2;
	  else if (x != 0)
	    x--;
	  printf_xy (x, 0, '0');
	}
      else
	{
	  //Si error, ometem el caracter polsat
	  if (i < '0' || i > '9')
	    continue;

	  printf_xy (x, 0, i);
	  x++;
	  
	  if (x == 2)
	    x++;
	}
    }
  
  sw7 = OFF;

  //Si estem d'acord, pitjem C de continue
  //Si volem tornar a introduir dades, pitjem *
 espera_confirmacio:
  i = keyScan ();
  
  if (i == 'C') //C de continue
    goto end_gpreu;
  
  if (i == '*') //* de tornar a intro
    goto ini_funcio_gpreu;

  i = 0x80; //Tecla erronia, no fem res
  goto espera_confirmacio;

 end_gpreu:
  {
    //Acabarem retornant el preu
    char preu[6];
    sw7 = OFF;
    scanf_xy (0, 1, preu, 5);

    //Preu en centims
    retorn = ((long int) (preu[0] - '0')) * 1000
      + ((long int) (preu[1] - '0')) * 100
      + ((long int) (preu[3] - '0')) * 10 + ((long int) (preu[4] - '0'));
    return retorn;
  }
}

static void
get_time_input ()
{
  //Aprofitament de codi de la practica anterior
  /*Bucle per introduïr dades al cronometre, quan haguem acabat
     polsarem el sw7. Si pitjem la tecla C es
     fan net les dades introduides.
     El temps introduit sortira a la banda esquerra de la pantalla, 
     i si polsem una tecla incorrecte sortira un missatge a la 
     dreta de la segona fila */

  char i, x;
 
 ini_funcio_gtime:
  x = 0;
  printf_xy (0, 0, "00:00");

  while (x < 5)
    {
      lcd_gotoxy (x, 0);
      i = 0x80;
      
      i = keyScan ();
      printf_xy (15, 1, ' ');

      //Si polsem C comensem de nou o netejem
      //Si no es un nombre: Error
      //Si massa gran: Big
      if (i == 'C')
	{
	  //Quan ens pitjin una 'C':
	  //Si volem eliminar ultim caracter, descomentar seg 3 linies
	  if (x == 3)
	    x = x - 2;
	  else if (x != 0)
	    x--;
	  printf_xy (x, 0, '0');

	  //Si volem començar de nou, descomentar les 2 seguents linies
	  //x = 0;
	  // lcd_clear();
	}
      else
	{
	  if (i < '0' || i > '9')
	    {
	      printf_xy (15, 1, 'E');
	      continue;
	    }
	  
	  //Hem de fer que no es puguin fer mes de 12:59
	  
	  if ( x == 0 && i > '1')
	    {
	      printf_xy (15,1,'B');
	      continue;
	    }
	  
	  if ( x == 3 && i > '5')
	    {
	      printf_xy (15,1,'B');
	    }
	  	  
	  printf_xy (x, 0, i);
	  x++;
  	  if (x == 2) x++;
	}
    }

 //Si estem d'acord, pitjem C de continue
 //Si volem tornar a introduir dades, pitjem *
 espera_confirmacio2:
  i = keyScan ();
  
  if (i == 'C') //C de continue
    goto end_gtime;
  
  if (i == '*') //* de tornar a intro
    goto ini_funcio_gtime;

  i = 0x80; //Tecla erronia, no fem res
  goto espera_confirmacio2;

end_gtime:
  {
    char hora[6];
    //Acabarem guardant la hora
    scanf_xy (0, 1, hora, 5);
    //Falta guardar hora display_to_seconds!!!
    //------------------------------------------------------Rapinyar codi practica anterior---*FIXME*
  }
}


/** Agafa 'len' bytes del display de la posico x,y
  * i els posa dins el *buffer
  */
inline static void
scanf_xy (char x, char y, char *buffer, char len)
{
  char i;
  for (i = 0; i < len; i++)
    buffer[i] = lcd_getc (x + i, y);
}

inline static void
printf_int (int x, int y, int s)
{
  char d[4];

  d[3] = (s % 10) + '0';
  s /= 10;
  d[2] = (s % 6) + '0';
  s /= 6;
  d[1] = (s % 10) + '0';
  d[0] = (s / 10) + '0';

  printf_xy (x, y, d);
}
