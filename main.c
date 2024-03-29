/*
 * Taxímetre -- Pràctica Final SDMI - QT2008
 * Copyright (C) 2008,2009  Felip Moll <lipixx at gmail.com>
 * Copyright (C) 2008,2009  Robert Millan <rmh@aybabtu.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "include/16F876_CCS.h"
#include "include/constants.h"
#include "lcd_lab.c"
#include "teclat_lab.c"
#org 0x1F00, 0x1FFF void loader16F876(void) {}


/*Variables globals*/
short sw7;			//Sw7 ha estat premut
short bandera_pampallugues;	//Led fa pampalluges
short comptador_hora;		//Comptem la hora
short am_pm;			//AM o PM
short comptador_import;		//Hem d'incrementar import
short tipus_de_fact;
short sw6_inhibit;
short la_lectura_de_litres_es_per_a;
char estat_lectura_litres;
char tarifa;			//1, 2 o 3
char bloc;			//BLOC d'execució
int ganancies_avui, kms_avui;	//Estadistiques
uint16_t hora_en_segons, hora_darrer_sw7, tics_pols;
uint16_t litres, litres_inicial, import;	//Estadistiques
uint16_t fraccio_de_segon, fraccio_de_pampalluga, fraccio_de_km;
uint16_t tarifa1_2[3][2];	//Preus de les tarifes
uint16_t tarifa3[4];

/*Capceleres de funcions*/
#define printf_xy(x,y,s)   { lcd_gotoxy(x,y); lcd_putc(s); }
static inline void printf_int (int x, int y, uint16_t s, char ndigits);
static void printf_xy_hora (int x, int y);
static inline void printf_xy_import (int x, int y, uint16_t s);
static void lcd_clear ();
static inline void scanf_xy (char x, char y, char *buffer, char len);
static void get_time_input ();
static uint16_t get_preu_kbd ();
static inline void print_tarifa (char i);

/*####################################################################*/

/*Funcions bàsiques reimplementades
  Aquestes funcions de mul, div i mod
  han estat reimplementades amb l'objectiu de substituïr les que
  fa servir el compilador amb els operands *,/,%, ja que aquestes
  funcionaven de tal manera que ens corrompien dades i ocupaven
  massa espai en ram. Per això les hem convertit en algorismes iteratius.
*/
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

/*Funcions senzilles sense capcelera*/
/*Únicament realitzen operacions de modificació/traducció de variables i
  de flags*/
inline void
engega_conversio_ad (short arg)
{
  la_lectura_de_litres_es_per_a = arg;
  estat_lectura_litres = LECTURA_LITRES_ESPERA1;
  ADON = ON;
  ADIF = OFF;
  ADIE = ON;
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
  /*El led D7 surt per l'RB7 */
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

/*####################################################################*/
/*CODE START*/

/** Interrupcions **/
/* En aquest apartat definim tots els handlers de les interrupcions,
així com el dispatcher que hem creat amb l'objectiu de guardar el context
d'execució en el moment que es rep una interrupció*/

#int_global			/*És important veure el significat d'aquesta directiva 
				   al datasheet */
void
interrupcions ()
{
  /* Ens reservem 3 posicions de memòria que ens serviràn, estem en el
     banc que estem, per salvar l'acumulador, l'status i el pclath. Aquestes 3
     posicions (20,21 i 22) estan reservades perquè ningú més les pugui fer servir.
     Ho hem reservat al fitxer "include/16F876_CCS.h", i com es podrà observar
     es reserven per cada banc; això vol dir que farem servir posicions relatives
     i que per exemple la 0x20 es corerspondrà a la 0xa0 del bank1 i a la 0x20
     del bank0
   */
#define W_OLD		0x20
#define STATUS_OLD	0x21
#define PCLATH_OLD	0x22

  /*Dispatcher a l'inici de la RSI */
#asm
  movwf W_OLD;			//Guardem el valor de l'acumulador, hauriem d'utilitzar swap?
  //W_OLD es trobarà a 0x20, 0xa0, 0x120 o 0x1a0 depenent del bank en que
  //ens trobem

  swapf STATUS, W;		//Guardem el valor de l'STATUS a l'acumulador
  clrf STATUS;			//Netejem STATUS, cosa que fa que canviem al bank0
  movwf STATUS_OLD;		//Guardam l'STATUS anterior a 0x21 de bank0

  movf PCLATH, W;		//Guardem el PCLATH
  movwf PCLATH_OLD;		//a 0x22 del bank0
  clrf PCLATH;
#endasm

  /*Gestors individuals de les interrupcions: comproven el flag d'interrupció
     i el bit d'enable */

  /* Defineix el valor del depòsit al engegar el taxi.
     L'activació d'sw6 per sí no genera cap interrupció, 
     però qualsevol interrupció
     és bona per atendre aquest esdeveniment. */
  if (sw6 && !sw6_inhibit)
    {
      sw6_inhibit = 1;
      engega_conversio_ad (LECTURA_LITRES_INICIAL);
    }

  //Interrupció del generador de Polsos
  if (TMR1IF == 1 && TMR1IE == 1)
    {
      /*Definim segons la velocitat, com facturar.
         tics_pols s'incrementa a cada tic de rellotge TMR0
         i tenim calculades i explicades les constants 
         a "include/constants.h" */
      if (tics_pols >= TICS_PER_30KM_S)
	tipus_de_fact = FACT_PER_TEMPS;
      else
	tipus_de_fact = FACT_PER_POLSOS;
      tics_pols = 0;

      if (fraccio_de_km++ == POLSOS_PER_KM)
	{
	  //Si entrem aqui hem fet un KM!
	  //Increment de l'import segons la tarifa
	  if (comptador_import && (tipus_de_fact == FACT_PER_POLSOS))
	    import +=
	      (tarifa ==
	       3 ? tarifa3[INDEX_PREU_PER_KM] :
	       tarifa1_2[INDEX_PREU_PER_KM][tarifa - 1]);
	  //Incrementem els kms fets avui
	  kms_avui++;
	  fraccio_de_km = 0;
	}

      //DEBUG: Per proteus, per fer que desbordi sense haver d'esperar
      TMR1L = 0xFE;
      TMR1H = 0xFF;
      //END DEBUG

      //Deshabilitar flag
      TMR1IF = 0;
    }

  /**Interrupció externa:
     Ens hem limitat a activar un bit que ens indicarà si el sw7
     ha estat premut o no. Haurem de fer polling per detectar-ho.
  */
  if (INTF == 1 && INTE == 1)
    {
      /* Per filtrar els rebots, comptem 1 segon entre ints */
      if (hora_darrer_sw7 != hora_en_segons)
	{
	  sw7 = ON;
	  hora_darrer_sw7 = hora_en_segons;
	}
      INTF = 0;
    }

  /**Rellotge del sistema!*/
  if (TMR0IF == 1 && TMR0IE == 1)
    {
      tics_pols++;

      //Iniciarem conversió AD si ens ho han demanat
      if (estat_lectura_litres == LECTURA_LITRES_INICIA_CONVERSIO)
	GO = 1;
      //Si ja estava iniciada farem una espera suficient per donar
      //temps a l'AD que adquireixi i converteixi. Les dues constants
      //serveixen com a canvi d'estat. Veure l'enum a "include/constants.h"
      else if (estat_lectura_litres == LECTURA_LITRES_ESPERA1
	       || estat_lectura_litres == LECTURA_LITRES_ESPERA2)
	estat_lectura_litres++;	//Acaba l'espera i lectura finalitzada

      //Ha passat 1 segon? (postscaler fraccio_de_segon)
      if ((fraccio_de_segon++ == TICS_PER_SEGON))
	{
	  //Si ha passat 1 hora
	  if (hora_en_segons == 43200)
	    {
	      am_pm++;		//Si han passat 12h canvi am_pm
	      hora_en_segons = 0;
	    }

	  if (am_pm)		//Si som les 00:00h AM, nou dia
	    {
	      ganancies_avui = 0;
	      kms_avui = 0;
	    }

	  //Hem de incrementar import cada segon segons la tarifa.
	  if (comptador_import && (tipus_de_fact == FACT_PER_TEMPS))
	    import +=
	      (tarifa ==
	       3 ? tarifa3[INDEX_PREU_PER_SEGON] :
	       tarifa1_2[INDEX_PREU_PER_SEGON][tarifa - 1]);

	  hora_en_segons++;
	  fraccio_de_segon = 0;
	}

      //Volem que el led parpadeigi
      if (bandera_pampallugues
	  && (fraccio_de_pampalluga++ == TICS_PER_PAMPALLUGA))
	{
	  if ((PORTB & 0x80) == 0)
	    PORTB = PORTB | 0x80;
	  else
	    PORTB = PORTB & 0x7F;
	  fraccio_de_pampalluga = 0;
	}

      //Desactivar flag
      TMR0IF = 0;
    }

  /**Interrupció de final de conversió*/
  if (ADIF == 1 && ADIE == 1)
    {
      //El diposit te 1024 litres
      litres = (ADRESH << 8) & ADRESL;
      if (la_lectura_de_litres_es_per_a == LECTURA_LITRES_INICIAL)
	litres_inicial = litres;
      estat_lectura_litres = LECTURA_LITRES_CONVERSIO_FINALITZADA;
      ADIF = 0;
    }

  /**Dispatcher: Restaurem el context amb l'ordre invers al que l'hem
     salvat. Com podem veure, després de moure el valor de l'STATUS_OLD
     a l'STATUS, hem canviat de bank, concretament al que estavem anteriorment.
     És per això que podem tornar accedir a la posició 0x20 relativa (que
     pot ser 0x20, 0xa0, 0x120 o 0x1a0 absoluta) i restaurar el W **/
#asm
  movf PCLATH_OLD, W;
  movwf PCLATH;

  swapf STATUS_OLD, W;
  movwf STATUS;

  swapf W_OLD, F;
  swapf W_OLD, W;
#endasm
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

/**A partir de la variable global hora_en_segons, pinta al display
   l'hora en format 00:00 a partir de x,y*/
static void
printf_xy_hora (int x, int y)
{
  /*
     La funcio es generica, funcionara tant
     si tenim posat les 12h o les 24h.. sempre
     i quan a la RSI es posi a 0 hora_en_segons
     quan pertoqui.
   */

  uint16_t s;
  s = hora_en_segons;

  printf_xy (x + 2, y, ':');
  printf_xy (x + 5, y, ':');

  /** Les funcions de mod, div i mul han estat substituïdes
      ja que ens corrompia altres variables, és més lent però
      fent el càlcul de tics perduts gairebé no es nota (1Tic màxim)
  */
  x += 6;
  printf_int (x, y, mod (s, 60), 2);
  x -= 3;
  s = div (s, 60);
  printf_int (x, y, mod (s, 60), 2);
  x -= 3;
  s = div (s, 60);
  printf_int (x, y, mod (s, 23), 2);
}

/**Similar a l'anterior però escriu en format 00,00 per simular €uros*/
static inline void
printf_xy_import (int x, int y, uint16_t s)
{
  x += 4;
  printf_xy (x--, y, mod (s, 10) + '0');
  s = div (s, 10);
  printf_xy (x--, y, mod (s, 10) + '0');
  s = div (s, 10);

  printf_xy (x--, y, '.');

  printf_xy (x--, y, mod (s, 10) + '0');
  printf_xy (x, y, div (s, 10) + '0');
}

/**Treu per el PORTB al display de 7 segments el valor de la tarifa 
   que tenim actualment. No és veurà correctament perquè està multiplexat
   amb el display i el teclat*/
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
  TRISB = 0x01;			//RB0 Input, RB1:7 Output
  TRISA = 0x3D;			//TRISA = 0x3F;    RA0:5 Pins coma a Input
  PORTA_1 = 0;

  lcd_init ();

  /* Per algun motiu, avegades el primer caràcter que enviem es perd. */
  lcd_putc (' ');

  /*Init Variables */
  bandera_pampallugues = OFF;
  sw7 = OFF;
  sw6_inhibit = OFF;
  bloc = REPOS;
  hora_en_segons = 0;
  comptador_hora = ON;
  comptador_import = OFF;
  import = 0;
  tarifa = 0;
  ganancies_avui = 0;
  kms_avui = 0;
  litres = 0;
  litres_inicial = 0;
  fraccio_de_segon = 0;
  fraccio_de_pampalluga = 0;
  fraccio_de_km = 0;
  am_pm = 0;
  tics_pols = 0;
  tipus_de_fact = FACT_PER_TEMPS;
  estat_lectura_litres = LECTURA_LITRES_REPOS;

  //DEBUG: Proteus, per no haver d'introduir unes tarifes des de 0
  tarifa1_2[INDEX_PREU_BAIXADA_BANDERA][0] = 200;
  tarifa1_2[INDEX_PREU_PER_KM][0] = 250;
  tarifa1_2[INDEX_PREU_PER_SEGON][0] = 1;
  //END DEBUG

  /*Interrupcions */
  /*Timers */
  PSA = 0;
  PS0 = 1;			//Preescaler
  PS1 = 1;
  PS2 = 0;
  TOCS = 0;
  TMR0 = 0;			/*Reestablim Contador */
  TMR0IF = 0;
  TMR1IF = 0;
  TMR1CS = 1;
  T1CKPS0 = 0;
  T1CKPS1 = 0;
  TMR1ON = 1;

  //DEBUG: Proteus, desbordem ràpid
  TMR1L = 0xFF;
  TMR1H = 0xFF;
  //END DEBUG

  INTE = OFF;

  ADIF = OFF;
  ADIE = OFF;

  TMR0IE = ON;
  TMR1IE = ON;
  PEIE = ON;

  //Sortida de AD
  CHS0 = 0;
  CHS1 = 0;
  CHS2 = 0;

  //Tensio referencia i entrades com analogiques,
  //ens ha fallat algun cop
#if 0
  PCFG0 = 0;
  PCFG1 = 0;
  PCFG2 = 0;
  PCFG3 = 1;
#endif
  //Lectura normal
  ADFM = 1;

  //COMENÇEM!!!!
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
	      /*Problema: Els switchs de TARIFA estan multiplexats amb 
	         3 bits de l'LCD. L'lcd hi te valors posats i els llegim 
	         d'alla. No te solucio. Tarifa no es seleccionarà mai be */
	      bloc = OCUPAT;
	      lcd_clear ();
	      print_tarifa (tarifa);
	    }
	  led_bandera (BANDERA_OFF);
	  break;

	case REPOS:
	  {
	    //Podrem anar a mode controls o a lliure
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

	    /* S'ha premut sw7 */
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
		//Fer la mitja de consum
		engega_conversio_ad (LECTURA_LITRES_ACTUAL);
		while (estat_lectura_litres !=
		       LECTURA_LITRES_CONVERSIO_FINALITZADA);
		estat_lectura_litres = LECTURA_LITRES_REPOS;

		printf_int (0, 0,
			    mul (100,
				 div (litres - litres_inicial, kms_avui)), 4);
	      }

	    sw7 = OFF;
	    INTE = ON;
	    while (!sw7);
	    INTE = OFF;

	    //Podem decidir sortir d'aqui o anar a modificar les tarifes
	    //hora, passwd, etc.
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
	  //Puja un client, comptem pujada de bandera i començem a facturar

	  lcd_clear ();
	  printf_xy (0, 1, "Ocupat");
	  import = (tarifa == 3 ? tarifa3[INDEX_PREU_BAIXADA_BANDERA] : tarifa1_2[INDEX_PREU_BAIXADA_BANDERA][tarifa - 1]);	/* FIXME: resta supèrflua */
	  comptador_import = ON;	//Demanem a l'RSI que incrementi 'import'

	  sw7 = OFF;
	  INTE = ON;

	  //Mentre no s'ha arribat al destí...
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
	    short sw5_inicial;
	    char c, k;
	    char suplements_emprats = 0;

	    //Fem pampallugues a la bandera
	    led_bandera (BANDERA_PAMPALLUGUES_);
	    print_tarifa (4);
	    printf_xy (0, 1, "Import");
	    sw5_inicial = sw5;

	    while (1)
	      {
		if (sw5 != sw5_inicial)
		  {
		    bloc = LLIURE;
		    //Increment estadistiques
		    ganancies_avui += import;
		    goto canvia_d_estat;
		  }

		c = keyScan_nobloca ();
		if (c != 0x80)
		  {
		    //Apliquem suplements
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
	  //Apocalipsis
	  printf_xy (0, 0, "bloc?");
	  break;
	}
    }
}

/**Obté un valor numèric presentat a pantalla amb el format 00,00
   que es convertirà en centims d'euro dins un uint16_t*/
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
      mul ((uint16_t) (preu[3] - '0'), 10) + (uint16_t) (preu[4] - '0');
  }
}

/** Mateixa funció que l'anterior, però en aquest cas amb l'hora que es
    passarà a segons */
static void
get_time_input ()
{
  /*Bucle per introduïr dades al display, quan haguem acabat
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

/**Imprimeix un enter s amb n digits a l'LCD*/
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
