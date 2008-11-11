#include <16F876_CCS.h>
#include <lcd_lab.c>
#include <teclat_lab.c>
#org 0x1F00, 0x1FFF void loader16F876(void) {}

char sw7;


#int_global
void
ext_int()
{
  if (INTF == 1 && INTE == 1)
    {
      sw7 = ON;
    }
  INTF = 0;
}


void
main()
{
  sw7 = OFF;
  
  while (1)
    {
      switch(bloc)
	{
	case LLIURE:
	  /*
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
	  break;

	case CONTROLS:
	  break;

	case OCUPAT:
	  break;

	case IMPORT:
	  break;

	default:
	  break;
	}
    }
}
 
