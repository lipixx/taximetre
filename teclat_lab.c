///////////////////////////////////////////////////////////////////////
/// PIC 16F87X Teclat Driver                                                                            ///
/// Versio 5 setembre 2008                                                                                      ///
/// Autor: Pere Mar�s, Joan Climent, Joan Vidos, Joel Sala                      ///
/// DEPARTAMENT d'ESAII                                                                                         ///
/// Codificacio  les funcions principals pel tractament del teclat      ///
/// Compatibilitzat amb  proteus i la placa de desenvolupament          ///
/// Modificat i millorat per: Robert Mill�n, Felip Moll - 16/01/2009
///////////////////////////////////////////////////////////////////////


const char KeyCodeTable[] =
{
  '1', '2', '3', 'A',
  '4', '5', '6', 'B',
  '7', '8', '9', 'C',
  '*', '0', '#', 'D'
};

char
keyScan_nobloca ()			// Scan for keyboard press
{
  unsigned char tmpTRISB, tmpPORTB, tmpRBPU, tmpINTE;
  unsigned char keyCode = 0x80;
  unsigned char col, i, kbhit, temp;

  // Guardar estat de *
  tmpTRISB = TRISB;
  //Faltaven guardar certes variables
  tmpPORTB = PORTB;
  tmpRBPU = RBPU;
  tmpINTE = INTE;

  INTE = 0;
  TRISB = 0xF0;			// RB4-RB7 entradas; RB0-RB3 salidas
  RBPU = 0;			//Pullup's ON(OPCION REGISTER)

  PORTB_0 = 1;
  PORTB_1 = 1;
  PORTB_2 = 1;
  PORTB_3 = 1;

  kbhit = 0;
  i = 0;
  while (!kbhit && (i < 4))
    {
      switch (i)
	{
	case 0:
	  PORTB_0 = 0;
	  break;
	case 1:
	  PORTB_1 = 0;
	  break;
	case 2:
	  PORTB_2 = 0;
	  break;
	case 3:
	  PORTB_3 = 0;
	  break;
	}

      delay_us (2);
      temp = PORTB & 0xF0;

      switch (i)
	{
	case 0:
	  PORTB_0 = 1;
	  break;
	case 1:
	  PORTB_1 = 1;
	  break;
	case 2:
	  PORTB_2 = 1;
	  break;
	case 3:
	  PORTB_3 = 1;
	  break;
	}
      if (temp != 0xF0)
	{
	  col = i;
	  kbhit = 1;;
	}
      i++;

    }
  if (temp == 0xF0)
    {
      keyCode = 0x80;
      goto end;
    }
  else
    {
      switch (temp)
	{
	case 0b11100000:
	  temp = 0x00;
	  break;
	case 0b11010000:
	  temp = 0x01;
	  break;
	case 0b10110000:
	  temp = 0x02;
	  break;
	case 0b01110000:
	  temp = 0x03;
	  break;
	}
    }


  i = (temp << 2) + col;
  keyCode = KeyCodeTable[i];

  //Millora del codi que restaura els registres modificats
 end:
  TRISB = tmpTRISB;
  PORTB = tmpPORTB;
  RBPU = tmpRBPU;

  INTF = 0;
  INTE = tmpINTE;

  return keyCode;
}

inline char
keyScan ()			// Scan for keyboard press
{
  char c;

  do
    c = keyScan_nobloca ();
  while (c == 0x80);
  while (keyScan_nobloca () != 0x80);

  return c;
}
