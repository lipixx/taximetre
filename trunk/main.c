#include <16F876_CCS.h>
#include <constants.h>
#include <lcd_lab.c>
#include <teclat_lab.c>
#org 0x1F00, 0x1FFF void loader16F876(void) {}

char sw6,sw7;
char bloc;

#int_global
void
ext_int()
{
  if (INTF == 1 && INTE == 1)
    {
      sw7 = ON;
    }
  /* Rellotge:
     actualitzar segons;
     comprovar sw6;
   */
  INTF = 0;
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
	  break;

	case CONTROLS:
	  //FELIP
	  break;

	case OCUPAT:
	  //ROBERT
	  break;

	case IMPORT:
	  //ROBERT
	  break;

	default:
	  break;
	}
    }
}
 
