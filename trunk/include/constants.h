///////////////////////////////////////////////////////////////////
///Fitxer de definicio de constants.
///Practica Final SDMI - QT2008
///
///Felip Moll
///Robert Millan
///////////////////////////////////////////////////////////////////

#define LCD_SIZE_X 16
#define LCD_SIZE_Y  2
#define X_HORA	   11
#define Y_HORA      1

#define LLIURE		0
#define REPOS		1
#define CONTROLS	2
#define OCUPAT		3
#define IMPORT_		4

#define ON	1
#define OFF	0

#define BANDERA_OFF	     0
#define BANDERA_ON	     1
#define BANDERA_PAMPALLUGUES_ 2

typedef unsigned long int uint16_t;

/* Important: l'enum i l'array que hi ha a continuació han d'estar en el
   mateix ordre.  */
enum suplements
  {
    SUPLEMENT_AERUPORT,
    SUPLEMENT_MALETA,
    SUPLEMENT_MOLL,
    SUPLEMENT_ASPECIAL,
    SUPLEMENT_FIRA,
    SUPLEMENT_GOS
  };

static const uint16_t suplement_index_to_preu[] =
  {
    300,	/* aeruport */
    90,		/* maleta */
    200,	/* moll */
    300,	/* aspecial */  //-------------------------S'HA DE MIRAR LA DATA?------------------*WARN*
    200,	/* fira */
    100		/* gos */
  };

#define TICS_PER_SEGON		1221
#define TICS_PER_PAMPALLUGA	TICS_PER_SEGON
#define TICS_PER_30KM		460

#define NCHARS_PASSWD (sizeof (passwd) - 1)
char passwd[] = "123";
const char hora[] = "00:00";
const char taula_print_tarifa[] = {0x00, 0x06, 0x5A, 0x4E, 0x0F};

#define X_IMPORT	0
#define Y_IMPORT	0
