

#define ILI9341_DRIVER


#define TFT_WIDTH  320
#define TFT_HEIGHT 240

#define TOUCH_IRQ 4
#define TOUCH_CS  7


#define TFT_CS    18
#define TFT_DC    16
#define TFT_RST   17

#define TFT_MISO  5  //T_DO
#define TFT_SCLK  15  //T_CLK
#define TFT_MOSI  6  //T_DIN

//#define SD_CS 6
//#define SD_MOSI 45
//#define SD_MISO 47
//#define SD_SCK 39





/*
#define TFT_CS    5
#define TFT_DC    16
#define TFT_RST   3

#define TFT_MISO  8//21//12
#define TFT_SCLK  41//20//19//14
#define TFT_MOSI  21//20//13

//#define SD_CS 6
//#define SD_MOSI 45
//#define SD_MISO 47
//#define SD_SCK 39

#define TOUCH_CS  4
#define TOUCH_IRQ 15*/
//#define TFT_SPI_HOST SPI2_HOST
//#define USE_HSPI_PORT

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_GFXFF

#define SPI_FREQUENCY  27000000
#define SPI_READ_FREQUENCY 20000000

//#define USE_HSPI_PORT
// #define NO_PSRAM