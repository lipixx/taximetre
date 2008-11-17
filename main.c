#include <16F876_CCS.h>
#include <constants.h>
#include <lcd_lab.c>
#include <teclat_lab.c>
#org 0x1F00, 0x1FFF void loader16F876(void) {}

char sw6,sw7;
char bloc;
char show_hour;
const char passwd[] = "PITOTE";
#define NCHARS_PASSWD sizeof(passwd)

inline void printc_xy(int x, int y, char c);

#int_global
void
ext_int()
{
  if (INTF == 1 && INTE == 1)
    {
      sw7 = ON;
    }

  if (som_en_una_interrupcio_de_rellotge)
    {
      if (comptador_hora && (fraccio_de_segon++ > TICS_PER_SEGON))
	{
	  hora_en_segons++;
	  fraccio_de_segon = 0;
	}
    }

  /*
     comprovar sw6;
   */
  INTF = 0;
}

static inline void
led_bandera (char on)
{
  d7 = on;
}

static inline void
lcd_clear ()
{
  char i, y;
  for (i = 0; i < LCD_SIZE_X; i++)
    for (y = 0; y < LCD_SIZE_Y; y++)
      printc_xy (x, y, ' ');
}

static inline void
printf_hora (int x, int y, char *s)
{
  /* FIXME: rapinyar codi practica anterior */
}

static inline void
printf_import (int x, int y, char *s)
{
  /* FIXME: 3/4 del mateix */
}

void
main()
{
  sw7 = OFF;
  bloc = REPOS; 
  GIE = 0; 
  
  while (1)
    {
      switch(bloc)
	{
	case LLIURE:
	  /*
	    FELIP
	  - bandera (led D7) =  ON
	  - indicador de tarifa = OFF
	  - mostrar_hora = ON
	  - esperar canvi d'estat:
	  */
	  while(sw7 != ON);
	  /*
	    · led D7 = OFF;
	    · llegir tarifa (SW2, SW3 o SW4)
	    Si baixadaBandera (SW1)  goto OCUPAT
	    sino goto REPOS
	   */
	  sw7 = OFF;
	  break;

	case REPOS:
	  mostrar_hora = OFF;
	  //Clean display???

	  while (bloc == REPOS)
	    {	    
	      char passwd = 1;
	      char c;
	      char buffer[NCHARS_PASSWD];
	      int i;

	      // sw1 = OFF; //ALERTA

	      //Get Passwd
	      for (i=0; i<NCHARS_PASSWD && !sw1; i++)
		{
		  do c = keyscan();
		  while (c==0x80 && !sw1);
		 
		  if (!sw1)
		    {
		      buffer[i] = c;
		      lcd_gotoxy(0+i,0); //alerta +i
		      lcd_putc('*');
		    }
		}
	      for (i=0; i<NCHARS_PASSWD && passwd; i++)
		  if (buffer[i] != passwd[i] || sw1) passwd = 0;
	      
	      if (passwd)
		  bloc = CONTROLS;
	      else 
		{
		  if (sw1 == ON)
		    bloc = LLIURE;
		  else
		    {
		      lcd_gotoxy(0,1);
		      lcd_putc('F');
		      lcd_putc('A');
		      lcd_putc('I');
		      lcd_putc('L');
		    }
		}
	    }
	  break;
	  
	case CONTROLS:
	  //FELIP

	  // per al modul OKUPAT em cal que CONTROLS activi `comptador_hora' tant
	  // bon punt l'hora s'hagi introduit -- robert
	  break;

	case OCUPAT:
	  //ROBERT
	  led_bandera (0);  /* FIXME: apaguem aqui o en sortir de lliure? */
	  lcd_clear ();
	  printc_xy (X_TARIFA, Y_TARIFA, tarifa);
	  import = PREU_FIX_BAIXADA_DE_BANDERA;
	  comptador_import = ON;  /* Demanem a l'RSI que incrementi `import' */
	  while (! SORTIR_D_OCUPAT)  /* FIXME: la condicio esta per definir */
	    {
	      printf_xy_import (X_IMPORT, Y_IMPORT, import);
	      printf_xy_hora (X_HORA, Y_HORA, hora_en_segons);
	    }
	  comptador_import = OFF;
	  break;

	case IMPORT:
	  //ROBERT
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

/**Agafa n caracters del teclat i els posa dins buffer*/
/*inline void get_key_string(char *buffer, int nchars)
{
  int i = nchars;
  char c;

  for (i=0; i<nchars; i++)
    {
      do c = keyscan();
      while (c==0x80);

      buffer[i] = c;
    }
    }*/

/*
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
*/
