#include "include/16F876_CCS.h"
#include "include/constants.h"
#include "lcd_lab.c"
#include "teclat_lab.c"
#org 0x1F00, 0x1FFF void loader16F876(void) {}


/*Variables*/

#define sw1 PORTA_0
#define sw2 PORTA_1
#define sw3 PORTA_2
#define sw4 PORTA_3
#define sw5 PORTA_4
#define sw6 PORTA_5
short sw7;
char bloc;

enum kjf0d49wf
  {
    FACT_PER_TEMPS,
    FACT_PER_POLSOS
  };

short bandera_pampallugues;
short comptador_hora;
short am_pm;
short comptador_import;
short tipus_de_fact = FACT_PER_TEMPS;
short sw6_inhibit = 0;
enum possibles_estats_destat_lectura_litres
  {
    LECTURA_LITRES_REPOS,
    LECTURA_LITRES_ESPERA1,
    LECTURA_LITRES_ESPERA2,
    LECTURA_LITRES_INICIA_CONVERSIO,
    LECTURA_LITRES_CONVERSIO_FINALITZADA
  };
char estat_lectura_litres = LECTURA_LITRES_REPOS;

enum possibles_estats_de_la_lectura_de_litres_es_per_a
  {
    LECTURA_LITRES_INICIAL,
    LECTURA_LITRES_ACTUAL
  };
short la_lectura_de_litres_es_per_a;

uint16_t hora_en_segons;
uint16_t hora_darrer_sw7;

char tarifa;

int ganancies_avui, kms_avui;
uint16_t litres, litres_inicial = 0;
uint16_t fraccio_de_segon, fraccio_de_pampalluga, fraccio_de_km;
uint16_t import;
uint16_t tics_pols;

#define INDEX_PREU_BAIXADA_BANDERA	0
#define INDEX_PREU_PER_KM		1
#define INDEX_PREU_PER_SEGON		2
#define INDEX_SUPLEMENT_HORARI_NOCT	3	/* nomes amb la 3 */
uint16_t tarifa1_2[3][2];
uint16_t tarifa3[4];

/*Capceleres de funcions*/
#define printf_xy(x,y,s)   { lcd_gotoxy(x,y); lcd_putc(s); }
static inline void scanf_xy (char x, char y, char *buffer, char len);
static void get_time_input ();
static uint16_t get_preu_kbd ();
static inline void print_tarifa (char i);
static inline void printf_xy_import (int x, int y, uint16_t s);
static void printf_xy_hora (int x, int y);
static void lcd_clear ();
static inline void led_bandera (char status);
static inline void printf_int (int x, int y, uint16_t s, char ndigits);
inline void suplement_ascii_to_index (char k);

uint16_t
mul (uint16_t a, uint16_t b)
{
  uint16_t r, i;
  r = 0;

  for (i = 0; i < a; i++)
    r += b;

  return r;
}

uint16_t
div (uint16_t n, uint16_t d)
{
  uint16_t i;

  for (i = 0; n >= d; i++)
    n -= d;

  return i;
}

uint16_t
mod (uint16_t n, uint16_t d)
{
  return n - mul (div (n, d), d);
}

inline void
engega_conversio_ad (short arg)
{
  la_lectura_de_litres_es_per_a = arg;
  estat_lectura_litres = LECTURA_LITRES_ESPERA1;
  ADON = ON;
  ADIF = OFF;
  ADIE = ON;
}

//Codi
#int_global
void
interrupcions ()
{
#define W_OLD		0x20
#define STATUS_OLD	0x21
#define PCLATH_OLD	0x22

#asm
  movwf W_OLD;

  swapf STATUS, W;
  clrf STATUS;
  movwf STATUS_OLD;

  movf PCLATH, W;
  movwf PCLATH_OLD;
  clrf PCLATH;
#endasm

  /* L'activació d'sw6 per sí no genera cap interrupció, però qualssevol interrupció
     és bona per atendre aquest esdeveniment.  */
  if (sw6 && !sw6_inhibit){
    sw6_inhibit = 1;
    engega_conversio_ad (LECTURA_LITRES_INICIAL);
  }

  if (TMR1IF == 1 && TMR1IE == 1)
    {
      if (tics_pols >= TICS_PER_30KM_S)
	tipus_de_fact = FACT_PER_TEMPS;
      else
	tipus_de_fact = FACT_PER_POLSOS;
      tics_pols = 0;
      
      if (fraccio_de_km++ == POLSOS_PER_KM)
	{
	  if (comptador_import && (tipus_de_fact == FACT_PER_POLSOS))
   	    import += (tarifa == 3 ? tarifa3[INDEX_PREU_PER_KM] : tarifa1_2[INDEX_PREU_PER_KM][tarifa - 1]);
	  kms_avui++;
	  fraccio_de_km = 0;
	}
      TMR1IF = 0;
    }

  if (INTF == 1 && INTE == 1)
    {
      /* Per filtrar els rebots.  */
      if (hora_darrer_sw7 != hora_en_segons)
	{
	  sw7 = ON;
	  hora_darrer_sw7 = hora_en_segons;
	}
      INTF = 0;
    }

  if (TMR0IF == 1 && TMR0IE == 1)
    {
      tics_pols++;

      if (estat_lectura_litres == LECTURA_LITRES_INICIA_CONVERSIO)
	GO = 1;
      else if (estat_lectura_litres == LECTURA_LITRES_ESPERA1 || estat_lectura_litres == LECTURA_LITRES_ESPERA2)
	estat_lectura_litres++;
      
      if ((fraccio_de_segon++ == TICS_PER_SEGON))
	{
	  if (hora_en_segons == 43200)
	    {
	      /*Si han passat 12h canvi am_pm */
	      am_pm++;
	      hora_en_segons = 0;
	    }

	  if (am_pm)		/*Si som les 00:00hAM, nou dia */
	    {
	      ganancies_avui = 0;
	      kms_avui = 0;
	    }
	  
	  if (comptador_import && (tipus_de_fact == FACT_PER_TEMPS))
	    import += (tarifa == 3 ? tarifa3[INDEX_PREU_PER_SEGON] : tarifa1_2[INDEX_PREU_PER_SEGON][tarifa - 1]);
	  
	  hora_en_segons++;
	  fraccio_de_segon = 0;
	}


      if (bandera_pampallugues
	  && (fraccio_de_pampalluga++ == TICS_PER_PAMPALLUGA))
	{
	  if ((PORTB & 0x80) == 0)
	    PORTB = PORTB | 0x80;
	  else
	    PORTB = PORTB & 0x7F;
	  fraccio_de_pampalluga = 0;
	}

      TMR0IF = 0;
    }

  if (ADIF == 1 && ADIE == 1)
    {
      //El diposit te 1024 litres
      litres = (ADRESH << 8) & ADRESL;
      if (la_lectura_de_litres_es_per_a == LECTURA_LITRES_INICIAL)
	litres_inicial = litres;
      estat_lectura_litres = LECTURA_LITRES_CONVERSIO_FINALITZADA;
      ADIF = 0;
    }

#asm
  movf PCLATH_OLD, W;
  movwf PCLATH;

  swapf STATUS_OLD, W;
  movwf STATUS;

  swapf W_OLD, F;
  swapf W_OLD, W;
#endasm
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
  /*El led D7 es l'RB7 */
  switch (status)
    {
    case BANDERA_ON:
      PORTB = PORTB | 0x80;
      break;
    case BANDERA_OFF:
      PORTB = PORTB & 0x7F;
      bandera_pampallugues = OFF;
      break;
    case BANDERA_PAMPALLUGUES_:
      bandera_pampallugues = ON;
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

static void
printf_xy_hora (int x, int y)
{
  /*
     La funcio es generica, funcionara tant
     si tenim posat les 12h o les 24h.. sempre
     i quan a la RSI es posi a 0 hora_en_segons
     quan pertoqui.
     s mod 60 = segon actual
     (s div 60) mod 60 = minut actual 
     (s div 3600) mod 23 = hora actual
   */

  uint16_t s;
  s = hora_en_segons;

  printf_xy (x + 2, y, ':');
  printf_xy (x + 5, y, ':');

  x += 6;
  printf_int (x, y, mod (s, 60), 2);
  x -= 3;
  s = div (s, 60);
  printf_int (x, y, mod (s, 60), 2);
  x -= 3;
  s = div (s, 60);
  printf_int (x, y, mod (s, 23), 2);
}

static inline void
printf_xy_import (int x, int y, uint16_t s)
{
  x += 4;
  printf_xy (x--, y, mod(s,10) + '0');
  s = div(s,10);
  printf_xy (x--, y, mod(s,10) + '0');
  s = div(s,10);

  printf_xy (x--, y, '.');

  printf_xy (x--, y, mod(s,10) + '0');
  printf_xy (x, y, div(s,10) + '0');
}

static inline void
print_tarifa (char i)
{
  PORTB = (PORTB & 0x80) | (taula_print_tarifa[i] & 0x7f);
}


void
main ()
{
  /*
     SW6 = RA5 per comensar a comptar combustible
     SW5 LLIURE!!  L'assignem a canvi d'estat en sortir de repòs
     SW2:4 = RA1:3 per les tarifes T1,T2,T3
     SW1/Entrada Analogica = RA0 boia
   */

  /*Pins */
  TRISB = 0x01;			/*RB0 Input, RB1:7 Output
				   TRISA = 0x3F;                        RA0:5 Pins coma a Input
				   ADCON1 = 0x??;               RA0:4 Pins com a Analogic */

  TRISA = 0x3D;
  PORTA_1 = 0;
  lcd_init ();

  /* Per algun motiu, el primer caràcter que enviem es perd.  */
  lcd_putc (' ');

  /*Variables */
  bandera_pampallugues = OFF;
  sw7 = OFF;
  bloc = REPOS;
  hora_en_segons = 0;
  comptador_hora = ON;
  comptador_import = OFF;
  import = 0;
  tarifa = 0;
  ganancies_avui = 0;
  kms_avui = 0;
  litres = 0;
  fraccio_de_segon = 0;
  fraccio_de_pampalluga = 0;
  fraccio_de_km = 0;
  am_pm = 0;
  tics_pols = 0;

  tarifa1_2[INDEX_PREU_BAIXADA_BANDERA][0] = 200;
  tarifa1_2[INDEX_PREU_PER_KM][0] = 250;
  tarifa1_2[INDEX_PREU_PER_SEGON][0] = 1;

  /*Interrupcions */
  /*Timer preescaler de 16: */

  PSA = 0;
  PS0 = 1;
  PS1 = 1;
  PS2 = 0;
  TOCS = 0;
  TMR0 = 0;			/*Reestablim Contador */
  TMR0IF = 0;
  TMR0IE = ON;
  TMR1IE = 1;

  TMR1CS = 1;
  T1CKPS0 = 0;
  T1CKPS1 = 0;
  T1SYNC_ = 1;
  TMR1ON = 1;

  INTE = OFF;

  ADIF = OFF;
  ADIE = OFF;

  PEIE = ON;

  //Sortida de AD
  CHS0 = 0;
  CHS1 = 0;
  CHS2 = 0;

  //Tensio referencia i
  //entrades com analogiques
#if 0
  PCFG0 = 0;
  PCFG1 = 0;
  PCFG2 = 0;
  PCFG3 = 1;
#endif
  //Llegir normal
  ADFM = 1;

  GIE = ON;

  while (1)
    {
    canvia_d_estat:
      switch (bloc)
	{
	case LLIURE:
	  lcd_clear ();
	  printf_xy (0, 1, "Lliure");
	  led_bandera (BANDERA_ON);
	  print_tarifa (0);	/*Netejem display 7 segments (indic. tarifa) */
	  sw7 = OFF;
	  INTE = ON;
	  while (sw7 != ON)	/*Apagar la bandereta */
	    printf_xy_hora (X_HORA, Y_HORA);
	  sw7 = OFF;
	  INTE = OFF;

	  /*Mirem l'estat del sw5, si OFF anem a REPOS
	     si ON anem a OCUPAT */
	  if (sw5 == OFF)
	    {
	      print_tarifa (0);
	      bloc = REPOS;
	    }
	  else
	    {
	      /*ANEM A OCUPAT, hem de llegir la tarifa
	         i llavors saltar. */

	      /*Llegim switchs per obtenir tarifa, obligatori posar-la */
	      tarifa = 0;
	      do
		{
		  if (sw4)
		    tarifa = 3;
		  else if (sw3)
		    tarifa = 2;
		  else if (sw2)
		    tarifa = 1;
		}
	      while (tarifa == 0);
	      /*Problema: Els switchs de TARIFA estan multiplexats amb 3 bits de l'LCD. L'lcd hi te valors
	         posats i els llegim d'alla. No te solucio. Mala sort */
	      bloc = OCUPAT;
	      lcd_clear ();
	      print_tarifa (tarifa);
	    }
	  led_bandera (BANDERA_OFF);
	  break;

	case REPOS:
	  {
	    char c;
	    char buffer[NCHARS_PASSWD];
	    char i;

	    lcd_clear ();
	    printf_xy (0, 0, "sw5=apujar band.");
	    printf_xy (0, 1, "sw7=mode control");

	    sw7 = OFF;
	    INTE = ON;
	    while (!sw5 && !sw7);
	    INTE = OFF;

	    if (sw5)
	      {
		bloc = LLIURE;
		goto canvia_d_estat;
	      }

	    /* S'ha premut sw7.  */

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
	      {
		bloc = CONTROLS;
		goto canvia_d_estat;
	      }
	  }
	  break;

	case CONTROLS:
	  {
	    char c, i, tmp[NCHARS_PASSWD];

	    //Comencem a mostrar dades estadistiques al taxista
	    lcd_clear ();
	    printf_xy (0, 1, "Fact. avui:");
	    printf_int (0, 0, ganancies_avui, 4);
	    sw7 = OFF;
	    INTE = ON;
	    while (!sw7);
	    INTE = OFF;

	    lcd_clear ();
	    printf_xy (0, 1, "Kms avui:");
	    printf_int (0, 0, kms_avui, 4);
	    sw7 = OFF;
	    INTE = ON;
	    while (!sw7);
	    INTE = OFF;

	    lcd_clear ();
	    printf_xy (0, 1, "Consum 100km:");
	    if (litres_inicial == 0)
	      /* Encara no s'ha iniciat el comptador (via sw6).  */
	      printf_int (0, 0, 0, 4);
	    else
	      {
		engega_conversio_ad (LECTURA_LITRES_ACTUAL);
		while (estat_lectura_litres != LECTURA_LITRES_CONVERSIO_FINALITZADA);
		estat_lectura_litres = LECTURA_LITRES_REPOS;
		
		printf_int (0, 0, mul (100, div (litres - litres_inicial, kms_avui)), 4);
	      }

	    sw7 = OFF;
	    INTE = ON;
	    while (!sw7);
	    INTE = OFF;

	    printf_xy (0, 1, "C=Set tarifes");
	    printf_xy (0, 0, "else goto REPOS");

	    c = keyScan ();
	    if (c != 'C')
	      {
		bloc = REPOS;
		goto canvia_d_estat;
	      }

	    lcd_clear ();
	    //Set Passwd
	    //Donem l'opcio de no canviar el passwd
	    //pressionant directament el *
	    i = 0;
	    printf_xy (0, 1, "New Pwd o '*':");

	    while (c != '*' && i < NCHARS_PASSWD)
	      {
		c = keyScan ();

		if (c == '*')
		  continue;

		tmp[i] = c;
		printf_xy (i, 0, c);
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

	    //Set Preus
	    lcd_clear ();
	    printf_xy (0, 1, "T1 - B.b.:");
	    tarifa1_2[INDEX_PREU_BAIXADA_BANDERA][0] = get_preu_kbd ();

	    lcd_clear ();
	    printf_xy (0, 1, "T1 - P/Km:");
	    tarifa1_2[INDEX_PREU_PER_KM][0] = get_preu_kbd ();

	    lcd_clear ();
	    printf_xy (0, 1, "T1 - H.Espera:");
	    tarifa1_2[INDEX_PREU_PER_SEGON][0] = get_preu_kbd ();

	    lcd_clear ();
	    printf_xy (0, 1, "T2 - B.b.:");
	    tarifa1_2[INDEX_PREU_BAIXADA_BANDERA][1] = get_preu_kbd ();

	    lcd_clear ();
	    printf_xy (0, 1, "T2 - P/Km:");
	    tarifa1_2[INDEX_PREU_PER_KM][1] = get_preu_kbd ();

	    lcd_clear ();
	    printf_xy (0, 1, "T2 - H.Espera:");
	    tarifa1_2[INDEX_PREU_PER_SEGON][1] = get_preu_kbd ();

	    lcd_clear ();
	    printf_xy (0, 1, "T3 - B.b.:");
	    tarifa3[INDEX_PREU_BAIXADA_BANDERA] = get_preu_kbd ();

	    lcd_clear ();
	    printf_xy (0, 1, "T3 - P/Km:");
	    tarifa3[INDEX_PREU_PER_KM] = get_preu_kbd ();

	    lcd_clear ();
	    printf_xy (0, 1, "T3 - H.Espera:");
	    tarifa3[INDEX_PREU_PER_SEGON] = get_preu_kbd ();

	    lcd_clear ();
	    printf_xy (0, 1, "T3 - S. Noct:");
	    tarifa3[INDEX_SUPLEMENT_HORARI_NOCT] = get_preu_kbd ();

	    bloc = REPOS;
	    break;
	  }

	case OCUPAT:
	  //ROBERT
	  lcd_clear ();
	  printf_xy (0, 1, "Ocupat");
	  import = (tarifa == 3 ?
		    tarifa3[INDEX_PREU_BAIXADA_BANDERA] :
		    tarifa1_2[INDEX_PREU_BAIXADA_BANDERA][tarifa - 1]);	/* FIXME: resta supèrflua */
	  comptador_import = ON;	/* Demanem a l'RSI que incrementi 'import' */

	  sw7 = OFF;
	  INTE = ON;
	  while (!sw7)
	    {
	      printf_xy_import (X_IMPORT, Y_IMPORT, import);
	      printf_xy_hora (X_HORA, Y_HORA);
	    }
	  INTE = OFF;
	  comptador_import = OFF;
	  bloc = IMPORT_;
	  break;

	case IMPORT_:
	  {
	    //ROBERT
	    short sw5_inicial;
	    char c, k;
	    char suplements_emprats = 0;
	    led_bandera (BANDERA_PAMPALLUGUES_);
	    print_tarifa (4);
	    printf_xy (0, 1, "Import");
	    sw5_inicial = sw5;

	    while (1)
	      {
		if (sw5 != sw5_inicial)
		  {
		    bloc = LLIURE;
		    ganancies_avui += import;
		    goto canvia_d_estat;
		  }

		c = keyScan_nobloca ();
		if (c != 0x80)
		  {
		    while (keyScan_nobloca () != 0x80);

		    k = suplement_ascii_to_index (c);
		    if (k != SUPLEMENT_MALETA)
		      {
			char j;
			/* La gràcia d'aquesta collonada és que `suplements_que_ja_hem_activat'
			   només ens ocupa un byte (el codi de flagelació ja ocupa més d'un
			   byte, però de memòria de codi anem sobrats).  */
			j = int_to_flag (k);
			if ((suplements_emprats & j) != 0)
			  continue;
			else
			  suplements_emprats |= j;
		      }
		    import += suplement_index_to_preu[k];
		  }
	      }
	  }
	  break;

	default:
	  printf_xy (0, 0, "bloc?");
	  break;
	}
    }
}

static uint16_t
get_preu_kbd ()
{
  char x, i;

ini_funcio_gpreu:
  x = 0;

  printf_xy (0, 0, "00,00");

  while (x < 5)
    {
      lcd_gotoxy (x, 0);
      i = keyScan ();

      //Opcio de rectificar amb la tecla B de Borrar
      if (i == 'B')
	{
	  if (x == 3)
	    x -= 2;
	  else if (x != 0)
	    x--;
	  printf_xy (x, 0, '0');
	}
      else if (i == 'C')
	goto end_gpreu;
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

  /*Si estem d'acord, pitjem C de continue
     Si volem tornar a introduir dades, pitjem * */
espera_confirmacio:
  i = keyScan ();

  if (i == 'C')			//C de continue
    goto end_gpreu;

  if (i == '*')			//* de tornar a intro
    goto ini_funcio_gpreu;

  i = 0x80;			//Tecla erronia, no fem res
  goto espera_confirmacio;

end_gpreu:
  {
    //Acabarem retornant el preu
    char preu[6];
    sw7 = OFF;
    scanf_xy (0, 0, preu, 5);

    //Preu en centims
    return
      mul ((uint16_t) (preu[0] - '0'), 1000) +
      mul ((uint16_t) (preu[1] - '0'), 100) +
      mul ((uint16_t) (preu[3] - '0'), 10) +
      (uint16_t) (preu[4] - '0');
  }
}

static void
get_time_input ()
{
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

bucle_time:
  while (x < 5)
    {
      char aux;
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

	  //Hem de fer que no es puguin fer mes de 23:59
	  switch (x)
	    {
	    case 0:
	      if (i > '2')
		{
		  printf_xy (15, 1, 'B');
		  goto bucle_time;
		}
	      aux = i;
	      break;

	    case 1:
	      {
		if ((aux == '2') && (i > '3'))
		  {
		    printf_xy (15, 1, 'B');
		    goto bucle_time;
		  }
	      }
	      break;

	    case 3:
	      if (i > '5')
		{
		  printf_xy (15, 1, 'B');
		  goto bucle_time;
		}
	      break;
	    default:
	      break;

	    }
	  printf_xy (x, 0, i);
	  x++;
	  if (x == 2)
	    x++;
	}
    }

  /*Si estem d'acord, pitjem C de continue
     Si volem tornar a introduir dades, pitjem * */
espera_confirmacio2:
  i = keyScan ();

  if (i == 'C')			//C de continue
    goto end_gtime;

  if (i == '*')			//* de tornar a intro
    goto ini_funcio_gtime;

  i = 0x80;			//Tecla erronia, no fem res
  goto espera_confirmacio2;

end_gtime:
  {
    char hora[6];
    uint16_t hora_en_hores;
    //Acabarem desant l'hora
    scanf_xy (0, 0, hora, 5);
    am_pm = 0;
    hora_en_hores = (hora[0] - '0') * 10 + (hora[1] - '0');
    if (hora_en_hores >= 12)
      {
	am_pm = 1;
	hora_en_hores -= 12;
      }
    hora_en_segons =
      hora_en_hores * 3600 +
      (((uint16_t) (hora[3] - '0') * 10 + (uint16_t) (hora[4] - '0')) * 60);
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

static inline void
printf_int (int x, int y, uint16_t s, int ndigits)
{
  signed int i;
  x += ndigits - 1;
  for (i = ndigits; i > 0; i--)
    {
      printf_xy (x--, y, mod (s, 10) + '0');
      s = div (s, 10);
    }
}
