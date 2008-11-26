///////////////////////////////////////////////////////////////////
///Fitxer de definicio de constants.
///Practica Final SDMI - QT2008
///
///Felip Moll
///Robert Millan
///////////////////////////////////////////////////////////////////

#define LCD_SIZE_X 16
#define LCD_SIZE_Y  2
#define X_HORA	   10
#define Y_HORA      1

#define LLIURE		0
#define REPOS		1
#define CONTROLS	2
#define OCUPAT		3
#define IMPORT		4

#define ON	1
#define OFF	0

#define BANDERA_OFF	     0
#define BANDERA_ON	     1
#define BANDERA_PAMPALLUGUES 2

/* Important: l'enum i l'array que hi ha a continuació han d'estar en el
   mateix ordre.  */
enum suplements
  {
    SUPLEMENT_AERUPORT,
    SUPLEMENT_MALETA,
    SUPLEMENT_MOLL,
    SUPLEMENT_ASPECIAL,
    SUPLEMENT_FIRA,
    SUPLEMENT_GOS,
  };
static const uint16_t suplement_index_to_preu[] =
  {
    300,	/* aeruport */
    90,		/* maleta */
    200,	/* moll */
    300,	/* aspecial */  //-------------------------S'HA DE MIRAR LA DATA?------------------*WARN*
    200,	/* fira */
    100,	/* gos */
  };

#define TICS_PER_SEGON	    78125
#define TICS_PER_PAMPALLUGA TICS_PER_SEGON * 2

#define NCHARS_PASSWD sizeof(passwd)
const char passwd[] = "PITOTE";
const char hora[] = "00:00";
