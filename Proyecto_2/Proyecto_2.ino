
/* Librería para el uso de la pantalla ILI9341 en modo 8 bits
   Basado en el código de martinayotte - https://www.stm32duino.com/viewtopic.php?t=637
   Adaptación, migración y creación de nuevas funciones: Pablo Mazariegos y José Morales
   Con ayuda de: José Guerra
   IE3027: Electrónica Digital 2 - 2019
*/

// LIBRERIAS
//***************************************************************************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include <TM4C123GH6PM.h>

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"

#include "bitmaps.c"
#include "font.h"
#include "lcd_registers.h"


#include <SPI.h>
#include <SD.h>

//DEFINES
//***************************************************************************************************************************************
#define LCD_RST PD_0
#define LCD_CS PD_1
#define LCD_RS PD_2
#define LCD_WR PD_3
#define LCD_RD PE_1
int DPINS[] = {PB_0, PB_1, PB_2, PB_3, PB_4, PB_5, PB_6, PB_7};

//Botones
#define SW1 PF_4                    //Definición de pin F4 para utilizacion de boton 1
#define SW2 PF_0                    //Definicion de pin F0 para utilizacion de boton 2

//SD
Sd2Card card;
SdVolume volume;
SdFile root;
int lecturaserial;
const int chipSelect = PA_3;
File pic;

// PROTOTIPOS DE FUNCIONES
//***************************************************************************************************************************************
void LCD_Init(void);
void LCD_CMD(uint8_t cmd);
void LCD_DATA(uint8_t data);
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
void LCD_Clear(unsigned int c);
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void LCD_Print(String text, int x, int y, int fontSize, int color, int background);
void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]);
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[], int columns, int index, char flip, char offset);

void Menu();                        //Función encargada de los gráficos para menú de inicio
void sw1();                         //Funcion que inicia el juego
void sw2();                         //Funcion que permite ver el leaderboard
void ObtenerImagen(char documento[]);
int AS_HE(int a);

//VARIABLES
//***************************************************************************************************************************************
int contador;                       //Variable usada para menu de personajes
int buttonState;                    //Estado actual del boton
int lastButtonState = LOW;          //Estado anterior del boton
long lastDebounceTime = 0;          //Última vez que el pin fue toggled
long debounceDelay = 50;            //Tiempo de debounce
int PJ1 = 0;                        //Bandera para personaje seleccionado jugador 1
String P;
String P2;
int PJ2 = 0;                        //Bandera para personaje seleccionado jugador 2
int M;                              //Variable para saber en que parte del menú se está
int Pelegido1 = 0;                  //Variable para saber si ya se eligió un jugador 1
int Pelegido2 = 0;                  //Variable para saber si ya se eligió un jugador 2

// SETUP
//***************************************************************************************************************************************
void setup() {

  // Inicializacion de la pantalla
  //***************************************************************************************************************************************
  SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
  Serial.begin(9600);
  GPIOPadConfigSet(GPIO_PORTB_BASE, 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);  // Configuración del puerto (utiliza TivaWare)
  Serial.println("Inicio");

  // Inicialización de la pantalla
  //***************************************************************************************************************************************
  LCD_Init();
  LCD_Clear(0x0000);

  //Pushbuttons
  //***************************************************************************************************************************************
  pinMode(SW1, INPUT_PULLUP);
  pinMode(SW2, INPUT_PULLUP);

  //Interrupciones
  attachInterrupt(digitalPinToInterrupt(SW1), sw1, RISING);
  attachInterrupt(digitalPinToInterrupt(SW2), sw2, RISING);

  //SD
  //***************************************************************************************************************************************
  SPI.setModule(0);

  Serial.print("Initializing SD card...");
  pinMode(PA_3, OUTPUT);

  if (!SD.begin(3)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  //Imagen de inicio
  //***************************************************************************************************************************************
  LCD_Clear(0xFFFF);
  ObtenerImagen("Fondo.txt");

  //Pantalla de Menu
  //***************************************************************************************************************************************
  delay(50);
  Menu();

  // Controles
  //***************************************************************************************************************************************
  Serial2.begin(9600);
  Serial3.begin(9600);
}

void loop() {

  if (Pelegido1 == 2 && Pelegido2 == 1) {
    Menu();
    Pelegido1 = 0;
    Pelegido2 = 0;
    M = 0;
    //LCD_Print(P, (320 / 2) - 40, (240 / 8) - 8 , 2, 0xCA2E, 0x0000);
    //LCD_Print(P2, (320 / 2) - 32, (240 / 4) - 8  , 2, 0x659C, 0x0000);
  }

  if (Serial2.available()) {

    int inByte = Serial2.read();
    Serial.write(inByte);
  }

  if (Serial3.available()) {
    int inByte2 = Serial3.read();
    Serial.write(inByte2);
  }

}


//***************************************************************************************************************************************
// Función para Menu
//***************************************************************************************************************************************
void Menu() {

  LCD_Clear(0x0000);
  Rect((320 / 2) - 50, (240 / 8) - 10, 100, 20, 0xD67D);
  String Gamename = "SUPER";
  LCD_Print(Gamename, (320 / 2) - 40, (240 / 8) - 8 , 2, 0xCA2E, 0x0000);
  H_line (20, (240 / 4), 280, 0xD67D);
  String Gamename1 = "JOSE";
  LCD_Print(Gamename1, (320 / 2) - 32, (240 / 4) - 8  , 2, 0x659C, 0x0000);
  String Gamename2 = "PONG";
  LCD_Print(Gamename2, (320 / 2) - 32, (240 / 2 ) - 36   , 2, 0xCA2E, 0x0000);
  LCD_Bitmap(60, 240 - 100, 35, 49, (unsigned char*)starticon);
  String Startgame = "Iniciar";
  LCD_Print(Startgame, 20, 240 - 40, 2, 0xD67D, 0x0000);
  LCD_Bitmap(220, 240 - 100, 53, 49, (unsigned char*)leaderboardicon);
  String Leaderboard = "Scores";
  LCD_Print(Leaderboard, 200, 240 - 40, 2, 0xD67D, 0x0000);

}

//***************************************************************************************************************************************
// Función para iniciar juego
//***************************************************************************************************************************************
void sw1() {

  M = 1;

  if (Pelegido1 == 0) {

    int reading = digitalRead(SW1);
    if (reading != lastButtonState) {
      lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {

      if (contador <= 2) {
        contador ++;
        delay(300);
      } else {
        contador = 0;
      }

      switch (contador) {

        case 1: {
            LCD_Clear(0x0000);
            String Menupersonajes = "Jugador 1:";
            LCD_Print(Menupersonajes, (320 / 2) - 80, (240 / 10), 2, 0xD67D, 0x0000);
            LCD_Bitmap((320 / 2) - 39, (240 / 2) - 60, 78, 120, (unsigned char*)personaje1);
            PJ1 = 1;
            if (SW2 == HIGH) {
              sw2();
            }
          } break;
        case 2: {
            LCD_Clear(0x0000);
            String Menupersonajes = "Jugador 1:";
            LCD_Print(Menupersonajes, (320 / 2) - 80, (240 / 10), 2, 0xD67D, 0x0000);
            LCD_Bitmap((320 / 2) - 42, (240 / 2) - 60, 85, 120, (unsigned char*)personaje2);
            PJ1 = 2;
            if (SW2 == HIGH) {
              sw2();
            }
          } break;
        case 3: {
            LCD_Clear(0x0000);
            String Menupersonajes = "Jugador 1:";
            LCD_Print(Menupersonajes, (320 / 2) - 80, (240 / 10), 2, 0xD67D, 0x0000);
            LCD_Bitmap((320 / 2) - 35, (240 / 2) - 60, 71, 120, (unsigned char*)personaje3);
            PJ1 = 3;
            if (SW2 == HIGH) {
              sw2();
            }
          } break;

      }

      buttonState = reading;

    }

    lastButtonState = reading;


  } else if (Pelegido1 != 0 && Pelegido2 == 0) {

    Pelegido1 = 2;

    int reading = digitalRead(SW1);
    if (reading != lastButtonState) {
      lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {

      if (contador <= 2) {
        contador ++;
        delay(300);
      } else {
        contador = 0;
      }

      switch (contador) {

        case 1: {
            LCD_Clear(0x0000);
            String Menupersonajes = "Jugador 2:";
            LCD_Print(Menupersonajes, (320 / 2) - 80, (240 / 10), 2, 0xD67D, 0x0000);
            LCD_Bitmap((320 / 2) - 39, (240 / 2) - 60, 78, 120, (unsigned char*)personaje1);
            PJ2 = 1;
            if (SW2 == HIGH) {
              sw2();
            }
          } break;
        case 2: {
            LCD_Clear(0x0000);
            String Menupersonajes = "Jugador 2:";
            LCD_Print(Menupersonajes, (320 / 2) - 80, (240 / 10), 2, 0xD67D, 0x0000);
            LCD_Bitmap((320 / 2) - 42, (240 / 2) - 60, 85, 120, (unsigned char*)personaje2);
            PJ2 = 2;
            if (SW2 == HIGH) {
              sw2();
            }
          } break;
        case 3: {
            LCD_Clear(0x0000);
            String Menupersonajes = "Jugador 2:";
            LCD_Print(Menupersonajes, (320 / 2) - 80, (240 / 10), 2, 0xD67D, 0x0000);
            LCD_Bitmap((320 / 2) - 35, (240 / 2) - 60, 71, 120, (unsigned char*)personaje3);
            PJ2 = 3;
            if (SW2 == HIGH) {
              sw2();
            }
          } break;

      }

      buttonState = reading;

    }

    lastButtonState = reading;

  }
}

//***************************************************************************************************************************************
// Función para: mostrar leaderboard o para elegir personaje
//***************************************************************************************************************************************

void sw2() {

  if (M == 1 ) {
    if (Pelegido1 == 0) {
      Pelegido1 = 1;
      LCD_Clear(0x0000);
      P = (String)PJ1;
      LCD_Print(P, (320 / 2) - 80, (240 / 10), 2, 0xD67D, 0x0000);
      SW2 == LOW;
    }
    if (Pelegido1 == 2 && Pelegido2 == 0) {
      Pelegido2 = 1;
      LCD_Clear(0x0000);
      P2 = (String)PJ2;
      LCD_Print(P2, (320 / 2) - 80, (240 / 10), 2, 0xD67D, 0x0000);
      SW2 == LOW;
    }
  } else {
    LCD_Clear(0x0000);
    Rect((320 / 2) - 100, (240 / 8) - 10, 200, 20, 0xD67D);
    String Leaderboardtitle = "LEADERBOARD";
    LCD_Print(Leaderboardtitle, (320 / 2) - 88, (240 / 8) - 8 , 2, 0xCA2E, 0x0000);
    String P1leaderboard = "Jugador 1: ";
    LCD_Print(P1leaderboard, (320 / 4) - 44, (240 / 2 ) - 36   , 2, 0x659C, 0x0000);
    String P2leaderboard = "Jugador 2: ";
    LCD_Print(P2leaderboard, (320 / 4) - 44, (240 / 2 ) + 36   , 2, 0x659C, 0x0000);
  }

}

//***************************************************************************************************************************************
// Función para obtener imagen de la SD
//***************************************************************************************************************************************
void ObtenerImagen(char documento[]) {

  pic = SD.open(documento, FILE_READ);
  int hex1 = 0;
  int val1 = 0;
  int val2 = 0;
  int mapear = 0;
  int vert = 0;

  unsigned char maps[640];
  if (pic) {
    Serial.println("Leyendo el archivo...");
    while (pic.available()) {
      mapear = 0;
      while (mapear < 640) {
        hex1 = pic.read();
        if (hex1 == 120) {
          val1 = pic.read();
          val2 = pic.read();
          val1 = AS_HE(val1);
          val2 = AS_HE(val2);
          maps[mapear] = val1 * 16 + val2;
          mapear++;
        }
      }
      LCD_Bitmap(0, vert, 320, 1, maps);
      vert++;
    }
    pic.close();
  }
  else {
    Serial.println("No se pudo encontrar el archivo.");
    pic.close();
  }
}

//***************************************************************************************************************************************
// Función para conversión hex
//***************************************************************************************************************************************
int AS_HE(int a) {

  switch (a) {
    case 48:
      return 0;
    case 49:
      return 1;
    case 50:
      return 2;
    case 51:
      return 3;
    case 52:
      return 4;
    case 53:
      return 5;
    case 54:
      return 6;
    case 55:
      return 7;
    case 56:
      return 8;
    case 57:
      return 9;
    case 97:
      return 10;
    case 98:
      return 11;
    case 99:
      return 12;
    case 100:
      return 13;
    case 101:
      return 14;
    case 102:
      return 15;
  }
}


//***************************************************************************************************************************************
// Función para inicializar LCD
//***************************************************************************************************************************************
void LCD_Init(void) {
  pinMode(LCD_RST, OUTPUT);
  pinMode(LCD_CS, OUTPUT);
  pinMode(LCD_RS, OUTPUT);
  pinMode(LCD_WR, OUTPUT);
  pinMode(LCD_RD, OUTPUT);
  for (uint8_t i = 0; i < 8; i++) {
    pinMode(DPINS[i], OUTPUT);
  }
  //****************************************
  // Secuencia de Inicialización
  //****************************************
  digitalWrite(LCD_CS, HIGH);
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, HIGH);
  digitalWrite(LCD_RD, HIGH);
  digitalWrite(LCD_RST, HIGH);
  delay(5);
  digitalWrite(LCD_RST, LOW);
  delay(20);
  digitalWrite(LCD_RST, HIGH);
  delay(150);
  digitalWrite(LCD_CS, LOW);
  //****************************************
  LCD_CMD(0xE9);  // SETPANELRELATED
  LCD_DATA(0x20);
  //****************************************
  LCD_CMD(0x11); // Exit Sleep SLEEP OUT (SLPOUT)
  delay(100);
  //****************************************
  LCD_CMD(0xD1);    // (SETVCOM)
  LCD_DATA(0x00);
  LCD_DATA(0x71);
  LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0xD0);   // (SETPOWER)
  LCD_DATA(0x07);
  LCD_DATA(0x01);
  LCD_DATA(0x08);
  //****************************************
  LCD_CMD(0x36);  // (MEMORYACCESS)
  LCD_DATA(0x40 | 0x80 | 0x20 | 0x08); // LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0x3A); // Set_pixel_format (PIXELFORMAT)
  LCD_DATA(0x05); // color setings, 05h - 16bit pixel, 11h - 3bit pixel
  //****************************************
  LCD_CMD(0xC1);    // (POWERCONTROL2)
  LCD_DATA(0x10);
  LCD_DATA(0x10);
  LCD_DATA(0x02);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC0); // Set Default Gamma (POWERCONTROL1)
  LCD_DATA(0x00);
  LCD_DATA(0x35);
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC5); // Set Frame Rate (VCOMCONTROL1)
  LCD_DATA(0x04); // 72Hz
  //****************************************
  LCD_CMD(0xD2); // Power Settings  (SETPWRNORMAL)
  LCD_DATA(0x01);
  LCD_DATA(0x44);
  //****************************************
  LCD_CMD(0xC8); //Set Gamma  (GAMMASET)
  LCD_DATA(0x04);
  LCD_DATA(0x67);
  LCD_DATA(0x35);
  LCD_DATA(0x04);
  LCD_DATA(0x08);
  LCD_DATA(0x06);
  LCD_DATA(0x24);
  LCD_DATA(0x01);
  LCD_DATA(0x37);
  LCD_DATA(0x40);
  LCD_DATA(0x03);
  LCD_DATA(0x10);
  LCD_DATA(0x08);
  LCD_DATA(0x80);
  LCD_DATA(0x00);
  //****************************************
  LCD_CMD(0x2A); // Set_column_address 320px (CASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x3F);
  //****************************************
  LCD_CMD(0x2B); // Set_page_address 480px (PASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0xE0);
  //  LCD_DATA(0x8F);
  LCD_CMD(0x29); //display on
  LCD_CMD(0x2C); //display on

  LCD_CMD(ILI9341_INVOFF); //Invert Off
  delay(120);
  LCD_CMD(ILI9341_SLPOUT);    //Exit Sleep
  delay(120);
  LCD_CMD(ILI9341_DISPON);    //Display on
  digitalWrite(LCD_CS, HIGH);
}

//***************************************************************************************************************************************
// Función para enviar comandos a la LCD - parámetro (comando)
//***************************************************************************************************************************************
void LCD_CMD(uint8_t cmd) {
  digitalWrite(LCD_RS, LOW);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = cmd;
  digitalWrite(LCD_WR, HIGH);
}

//***************************************************************************************************************************************
// Función para enviar datos a la LCD - parámetro (dato)
//***************************************************************************************************************************************
void LCD_DATA(uint8_t data) {
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = data;
  digitalWrite(LCD_WR, HIGH);
}

//***************************************************************************************************************************************
// Función para definir rango de direcciones de memoria con las cuales se trabajara (se define una ventana)
//***************************************************************************************************************************************
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
  LCD_CMD(0x2a); // Set_column_address 4 parameters
  LCD_DATA(x1 >> 8);
  LCD_DATA(x1);
  LCD_DATA(x2 >> 8);
  LCD_DATA(x2);
  LCD_CMD(0x2b); // Set_page_address 4 parameters
  LCD_DATA(y1 >> 8);
  LCD_DATA(y1);
  LCD_DATA(y2 >> 8);
  LCD_DATA(y2);
  LCD_CMD(0x2c); // Write_memory_start
}

//***************************************************************************************************************************************
// Función para borrar la pantalla - parámetros (color)
//***************************************************************************************************************************************
void LCD_Clear(unsigned int c) {
  unsigned int x, y;
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  SetWindows(0, 0, 319, 239); // 479, 319);
  for (x = 0; x < 320; x++)
    for (y = 0; y < 240; y++) {
      LCD_DATA(c >> 8);
      LCD_DATA(c);
    }
  digitalWrite(LCD_CS, HIGH);
}

//***************************************************************************************************************************************
// Función para dibujar una línea horizontal - parámetros ( coordenada x, cordenada y, longitud, color)
//***************************************************************************************************************************************
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {
  unsigned int i, j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + x;
  SetWindows(x, y, l, y);
  j = l;// * 2;
  for (i = 0; i < l; i++) {
    LCD_DATA(c >> 8);
    LCD_DATA(c);
  }
  digitalWrite(LCD_CS, HIGH);
}

//***************************************************************************************************************************************
// Función para dibujar una línea vertical - parámetros ( coordenada x, cordenada y, longitud, color)
//***************************************************************************************************************************************
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {
  unsigned int i, j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + y;
  SetWindows(x, y, x, l);
  j = l; //* 2;
  for (i = 1; i <= j; i++) {
    LCD_DATA(c >> 8);
    LCD_DATA(c);
  }
  digitalWrite(LCD_CS, HIGH);
}

//***************************************************************************************************************************************
// Función para dibujar un rectángulo - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  H_line(x  , y  , w, c);
  H_line(x  , y + h, w, c);
  V_line(x  , y  , h, c);
  V_line(x + w, y  , h, c);
}

//***************************************************************************************************************************************
// Función para dibujar un rectángulo relleno - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);

  unsigned int x2, y2;
  x2 = x + w;
  y2 = y + h;
  SetWindows(x, y, x2 - 1, y2 - 1);
  unsigned int k = w * h * 2 - 1;
  unsigned int i, j;
  for (int i = 0; i < w; i++) {
    for (int j = 0; j < h; j++) {
      LCD_DATA(c >> 8);
      LCD_DATA(c);

      //LCD_DATA(bitmap[k]);
      k = k - 2;
    }
  }
  digitalWrite(LCD_CS, HIGH);
}

//***************************************************************************************************************************************
// Función para dibujar texto - parámetros ( texto, coordenada x, cordenada y, color, background)
//***************************************************************************************************************************************
void LCD_Print(String text, int x, int y, int fontSize, int color, int background) {
  int fontXSize ;
  int fontYSize ;

  if (fontSize == 1) {
    fontXSize = fontXSizeSmal ;
    fontYSize = fontYSizeSmal ;
  }
  if (fontSize == 2) {
    fontXSize = fontXSizeBig ;
    fontYSize = fontYSizeBig ;
  }

  char charInput ;
  int cLength = text.length();
  Serial.println(cLength, DEC);
  int charDec ;
  int c ;
  int charHex ;
  char char_array[cLength + 1];
  text.toCharArray(char_array, cLength + 1) ;
  for (int i = 0; i < cLength ; i++) {
    charInput = char_array[i];
    Serial.println(char_array[i]);
    charDec = int(charInput);
    digitalWrite(LCD_CS, LOW);
    SetWindows(x + (i * fontXSize), y, x + (i * fontXSize) + fontXSize - 1, y + fontYSize );
    long charHex1 ;
    for ( int n = 0 ; n < fontYSize ; n++ ) {
      if (fontSize == 1) {
        charHex1 = pgm_read_word_near(smallFont + ((charDec - 32) * fontYSize) + n);
      }
      if (fontSize == 2) {
        charHex1 = pgm_read_word_near(bigFont + ((charDec - 32) * fontYSize) + n);
      }
      for (int t = 1; t < fontXSize + 1 ; t++) {
        if (( charHex1 & (1 << (fontXSize - t))) > 0 ) {
          c = color ;
        } else {
          c = background ;
        }
        LCD_DATA(c >> 8);
        LCD_DATA(c);
      }
    }
    digitalWrite(LCD_CS, HIGH);
  }
}

//***************************************************************************************************************************************
// Función para dibujar una imagen a partir de un arreglo de colores (Bitmap) Formato (Color 16bit R 5bits G 6bits B 5bits)
//***************************************************************************************************************************************
void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);

  unsigned int x2, y2;
  x2 = x + width;
  y2 = y + height;
  SetWindows(x, y, x2 - 1, y2 - 1);
  unsigned int k = 0;
  unsigned int i, j;

  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k + 1]);
      //LCD_DATA(bitmap[k]);
      k = k + 2;
    }
  }
  digitalWrite(LCD_CS, HIGH);
}

//***************************************************************************************************************************************
// Función para dibujar una imagen sprite - los parámetros columns = número de imagenes en el sprite, index = cual desplegar, flip = darle vuelta
//***************************************************************************************************************************************
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[], int columns, int index, char flip, char offset) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);

  unsigned int x2, y2;
  x2 =   x + width;
  y2 =    y + height;
  SetWindows(x, y, x2 - 1, y2 - 1);
  int k = 0;
  int ancho = ((width * columns));
  if (flip) {
    for (int j = 0; j < height; j++) {
      k = (j * (ancho) + index * width - 1 - offset) * 2;
      k = k + width * 2;
      for (int i = 0; i < width; i++) {
        LCD_DATA(bitmap[k]);
        LCD_DATA(bitmap[k + 1]);
        k = k - 2;
      }
    }
  } else {
    for (int j = 0; j < height; j++) {
      k = (j * (ancho) + index * width + 1 + offset) * 2;
      for (int i = 0; i < width; i++) {
        LCD_DATA(bitmap[k]);
        LCD_DATA(bitmap[k + 1]);
        k = k + 2;
      }
    }


  }
  digitalWrite(LCD_CS, HIGH);
}
