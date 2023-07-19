#include "cyhal.h"
#include "cybsp.h"
#include "cy_pdl.h"
#include "GUI.h"
#include "SPI_st7789v.h"
#include "LCDConf.h"
#include "DIALOG.h"
#include <stdio.h>

//Incluyo las bibliotecas necesarias para interactuar con el hardware,
//la GUI y otras funciones útiles.

#define LINE_HEIGHT 25    // Altura de cada elemento en la listwheel
#define Y_SIZE      210   // Ancho total de la listwheel

#define BUTTON_INC_PIN  P9_0  // Pulsador para incrementar
#define BUTTON_DEC_PIN  P5_7  // Pulsador para decrementar

//Creo un arreglo N con 10 elementos, que contiene diferentes patrones de bits utilizados
//para configurar las entradas JAM del CD4059, permitiendo establecer el valor de división requerido.

int N[10]={0x38,0x39,0x3A,0x3B,0x3C,0x40,0x41,0x42,0x43,0x44};

//Creo un arreglo Pines con 7 elementos, que contiene los pines GPIO utilizados para controlar el
//circuito CD4059 a traves de las entradas JAM

cyhal_gpio_t Pines[7]={P9_6,P8_0,P9_2,P9_1,P9_4,P9_7,P5_5};

// Función "escribir" que recibe un entero "w" como parámetro.
// Esta función se encarga de mostrar en los pines GPIO los bits almacenados en el arreglo N[w].
// Para cada pin en el arreglo Pines, se escribe el bit correspondiente de N[w].

void escribir(int w){
	    for(int k=0;k<7;k++)
		cyhal_gpio_write(Pines[k], (N[w] >> k)&1);
}

//Estructura tft_pins que define la configuración de los pines GPIO utilizados para controlar una
//pantalla TFT con el controlador ST7789V a través de SPI.

const SPI_st7789v_pins_t tft_pins =
{
    .MOSI = P10_0,
    .MISO = P10_1,
    .SCK = P10_2,
    .SS = P10_3,
    .dc = P12_0,
	.rst= P12_1,
	.frec= 25000000
};

//Se define un arreglo _acText que contiene los elementos que se mostrarán en el LISTWHEEL.

static const char * _acText[] = {
  "955",
  "965",
  "975",
  "985",
  "995",
  "1005",
  "1015",
  "1025",
  "1035",
  "1045",
  NULL
};

 static LISTWHEEL_Handle hListwheel;

 //Se crea una función estática _OwnerDrawListwheel, que es utilizada para personalizar el aspecto
 //visual de los elementos en una lista de rueda (listwheel) de la GUI.

static int _OwnerDrawListwheel(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo) {
  char     acBuffer[32];
  GUI_RECT Rect;

  switch (pDrawItemInfo->Cmd) {
  case WIDGET_ITEM_GET_XSIZE:
	  //
	  // Devuelve el tamaño en x del elemento
	  //
    return LISTWHEEL_OwnerDraw(pDrawItemInfo);
  case WIDGET_ITEM_GET_YSIZE:
	  //
	  // Devuelve el tamaño en y del elemento
	  //
    return LISTWHEEL_OwnerDraw(pDrawItemInfo);
  case WIDGET_ITEM_DRAW:
	  //
	  // Dibuja un elemento
	  //
	  //
	  // Obtiene el rectángulo de dibujo
	  //
    Rect.x0 = pDrawItemInfo->x0;
    Rect.x1 = pDrawItemInfo->x1;
    Rect.y0 = pDrawItemInfo->y0;
    Rect.y1 = pDrawItemInfo->y1;

    GUI_SetFont(&GUI_Font32B_ASCII);
    GUI_SetColor(GUI_WHITE);
    //
    // Muestra el texto del elemento centrado
    //
    GUI_SetTextMode(GUI_TM_TRANS);
    LISTWHEEL_GetItemText(pDrawItemInfo->hWin, pDrawItemInfo->ItemIndex, acBuffer, sizeof(acBuffer));
    GUI_DispStringInRect(acBuffer, &Rect, GUI_TA_CENTER);
    return 0;
  case WIDGET_ITEM_DRAW_BACKGROUND:
	  //
	  // Dibuja el fondo
	  //
    GUI_DrawGradientV(pDrawItemInfo->x0, pDrawItemInfo->y0, pDrawItemInfo->x1, pDrawItemInfo->y1, GUI_GRAY_7C, GUI_GRAY_AA);
    GUI_SetColor(GUI_BLACK);
    GUI_SetPenSize(2);
    GUI_DrawRect(pDrawItemInfo->x0, pDrawItemInfo->y0, pDrawItemInfo->x1, pDrawItemInfo->y1);
    return 0;
  case WIDGET_ITEM_DRAW_OVERLAY:
    GUI_SetColor(GUI_RED);
    GUI_DrawHLine(pDrawItemInfo->y0 + (LINE_HEIGHT * 1.8) + 1 , pDrawItemInfo->x0, pDrawItemInfo->x1);
    GUI_DrawHLine(pDrawItemInfo->y1 - (LINE_HEIGHT * 2) + 2, pDrawItemInfo->x0, pDrawItemInfo->x1);
    return 0;
  default:
    return LISTWHEEL_OwnerDraw(pDrawItemInfo);
  }
}


//Se crea una función estática _cbWin, que es el callback (controlador de eventos) para la ventana de la GUI. Esta función es responsable
// de manejar los eventos relacionados con la lista de rueda (listwheel) y su selección.

static void _cbWin(WM_MESSAGE * pMsg) {
  int                     NCode;
  int                     Sel;
  char                    acTemp[32];


  switch (pMsg->MsgId) {
  case WM_CREATE:
	//
	// Crea el widget de listwheel
	//
    hListwheel = LISTWHEEL_CreateEx(15, 10, Y_SIZE, 70, pMsg->hWin, WM_CF_SHOW, 0, GUI_ID_LISTWHEEL0, _acText);
    //
    // Establece la posición de ajuste (posición donde la rueda se detiene)
    //
    LISTWHEEL_SetSnapPosition(hListwheel, (Y_SIZE - LINE_HEIGHT) * 0.1);
    //
    // Establece algunas propiedades sobre la listwheel
    //
    LISTWHEEL_SetLineHeight(hListwheel, LINE_HEIGHT);
    LISTWHEEL_SetTextAlign(hListwheel, GUI_TA_CENTER);
    //
    // Establece la función de OwnerDraw
    //
    LISTWHEEL_SetOwnerDraw(hListwheel, _OwnerDrawListwheel);
    break;
  case WM_PAINT:
	  GUI_SetBkColor(GUI_WHITE);
	     GUI_Clear();
	     //
	     // Dibuja el valor seleccionado actualmente
	     //
	     GUI_SetColor(GUI_BLACK);
	     Sel = LISTWHEEL_GetSel(hListwheel);
	     LISTWHEEL_GetItemText(hListwheel, Sel, acTemp, sizeof(acTemp));
	     break;
  case WM_NOTIFY_PARENT:
    NCode = pMsg->Data.v;
    switch (NCode) {
    case WM_NOTIFICATION_SEL_CHANGED:
    	//
        // Obtiene la selección actual
        //
      Sel = LISTWHEEL_GetPos(hListwheel);
      LISTWHEEL_SetSel(hListwheel, Sel);
      break;
    }
    WM_InvalidateWindow(pMsg->hWin);
    break;
  default:
    WM_DefaultProc(pMsg);
    break;
  }
}


int main(void)
{
    cy_rslt_t result;

#if defined (CY_DEVICE_SECURE)
    cyhal_wdt_t wdt_obj;

    /* Borra el temporizador del watchdog para que no provoque un reinicio */
    result = cyhal_wdt_init(&wdt_obj, cyhal_wdt_get_max_timeout_ms());
    CY_ASSERT(CY_RSLT_SUCCESS == result);
    cyhal_wdt_free(&wdt_obj);
#endif

    /* Inicializa los periféricos del dispositivo y de la placa */
    result = cybsp_init();

    /* Si falla la inicialización de la placa, detiene la ejecución del programa */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Habilita las interrupciones globales */
    __enable_irq();

    result = SPI_st7789v_init8(&tft_pins);
    CY_ASSERT(result == CY_RSLT_SUCCESS);

    GUI_Init();
    WM_MULTIBUF_Enable(1);
    //
    // Crea una ventana
    //
      WM_CreateWindowAsChild(0, 0, LCD_GetXSize(), LCD_GetYSize(), WM_HBKWIN, WM_CF_SHOW, _cbWin, 0);
      for(int i=0;i<7;i++)
          cyhal_gpio_init(Pines[i], CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG , 0);

      //
      // Configurar pines de los pulsadores
      //

      cyhal_gpio_init(BUTTON_INC_PIN, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_PULLDOWN, 0);
      cyhal_gpio_init(BUTTON_DEC_PIN, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_PULLDOWN, 0);


      int pulsador_inc = 0, pulsador_dec = 0, pulsador_inc_previo = 0, pulsador_dec_previo = 0, s=0 , p=0;

      // Mostrar el primer patrón de bits almacenado en N[0].

      escribir(0);

    while(1){

    // Leer el estado de los pulsadores en los pines P9_0 y P5_7.

     pulsador_inc = cyhal_gpio_read(BUTTON_INC_PIN);
     pulsador_dec = cyhal_gpio_read(BUTTON_DEC_PIN);

    // Si se detecta una transición del estado "no presionado" a "presionado" en el pulsador de incremento (pulsador_inc_previo),
    // se incrementa la posición de la lista de rueda (s) y se mueve la lista de rueda a la nueva posición utilizando
	// LISTWHEEL_MoveToPos. Luego, se llama a escribir(s) mostrar el nuevo patrón de bits.

     if (pulsador_inc && !pulsador_inc_previo ) {
      s=(LISTWHEEL_GetPos(hListwheel) + 1) % 10;
      LISTWHEEL_MoveToPos(hListwheel,s);
      escribir(s);
    }

    // Similarmente, si se detecta una transición del estado "no presionado" a "presionado" en el pulsador de decremento
	// (pulsador_dec_previo), se decrementa la posición de la lista de rueda (p) y se mueve la lista de rueda a la nueva posición.
	// Se llama a escribir(p) para mostrar el nuevo patrón de bits.

     if (pulsador_dec && !pulsador_dec_previo ) {
      p=(LISTWHEEL_GetPos(hListwheel) - 1 + 10) % 10;
      LISTWHEEL_MoveToPos(hListwheel,p);
      escribir(p);
    }

    // Actualizar los valores previos de los pulsadores para la siguiente iteración del bucle.

      pulsador_inc_previo = pulsador_inc;
      pulsador_dec_previo = pulsador_dec;

      WM_InvalidateWindow(WM_HBKWIN);
      GUI_Exec();  // Procesar eventos de la GUI

    }
}
