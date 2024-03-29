///////////////////////////////////////////////////////////////////
///Fitxer de definició de constants.
///Pràctica Final SDMI - QT2008
///
///Felip Moll - lipixx at gmail.com
///Robert Millan - robert.millan at est.fib.upc.edu
///Sota llicència GPLv3
///16/01/2009
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

#define LCD_SIZE_X 16
#define LCD_SIZE_Y  2
#define X_HORA	    8
#define Y_HORA      1
#define X_IMPORT	0
#define Y_IMPORT	0

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

#define sw1 PORTA_0
#define sw2 PORTA_1
#define sw3 PORTA_2
#define sw4 PORTA_3
#define sw5 PORTA_4
#define sw6 PORTA_5

#define INDEX_PREU_BAIXADA_BANDERA	0
#define INDEX_PREU_PER_KM		1
#define INDEX_PREU_PER_SEGON		2
#define INDEX_SUPLEMENT_HORARI_NOCT	3	/* nomes amb la 3 */

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

enum kjf0d49wf
{
  FACT_PER_TEMPS,
  FACT_PER_POLSOS
};

enum possibles_estats_destat_lectura_litres
{
  LECTURA_LITRES_REPOS,
  LECTURA_LITRES_ESPERA1,
  LECTURA_LITRES_ESPERA2,
  LECTURA_LITRES_INICIA_CONVERSIO,
  LECTURA_LITRES_CONVERSIO_FINALITZADA
};

enum possibles_estats_de_la_lectura_de_litres_es_per_a
{
  LECTURA_LITRES_INICIAL,
  LECTURA_LITRES_ACTUAL
};


static const uint16_t suplement_index_to_preu[] =
  {
    300,	/* aeruport */
    90,		/* maleta */
    200,	/* moll */
    300,	/* aspecial */
    200,	/* fira */
    100		/* gos */
  };

/* Polsos per volta */
#define POLSOS_PER_VOLTA	1

#define TICS_PER_SEGON		1221 //DEBUG: Per fer proves a PROTEUS, canviar el valor
#define TICS_PER_PAMPALLUGA	TICS_PER_SEGON

// PI * TICS_PER_SEGON * POLSOS_PER_VOLTA 1/m * 8.333 m/s
#define TICS_PER_30KM_S		(460 * POLSOS_PER_VOLTA)

//(1000 / PI) * POLSOS_PER_VOLTA
#define POLSOS_PER_KM		(318 * POLSOS_PER_VOLTA)

#define NCHARS_PASSWD (sizeof (passwd) - 1)
char passwd[] = "123";
const char hora[] = "00:00";
const char taula_print_tarifa[] = {0x00, 0x06, 0x5A, 0x4E, 0x0F};

