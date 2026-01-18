
/*
 * Pocket I-Ching 2.5
 * CodePoetry by L. Christopher Bird zenmondo@gmail.com
 * 
 * An electronic oracle based on the Book of Changes.
 * 
 * Uses the coin method for constructing a hexagram.
 * 
 * Developed for a Adafruit Metro RP2040 and a 2.8" TFT Touch Shield for Arduino With Resisitive Touch Screen V.2 using the Arduino IDEs
 * https://www.adafruit.com/product/5786
 * https://www.adafruit.com/product/1651
 * 
 * For more about the I-Ching see https://h2g2.com/edited_entry/A647840
 * 
 * The Bitmap Drawing functions were written by Limor Fried/Ladyada for Adafruit Industries.
 * 
 * MIT license, all text above must be included in any redistribution
 * 
 */

//Adafruit Library example Credits and License text:
/***************************************************
  This is our Bitmap drawing example for the Adafruit ILI9341 Breakout and Shield
  ----> http://www.adafruit.com/products/1651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/


#include <SPI.h>
#include <Wire.h>
#include <Adafruit_TSC2007.h>
#include <Adafruit_GFX.h>         // Core graphics library
#include <Adafruit_ILI9341.h>     // Hardware-specific library
#include <SdFat.h>                // SD card & FAT filesystem library
#include <Adafruit_ImageReader.h> // Image-reading functions

// TFT display and SD card will share the hardware SPI interface.
// Hardware SPI pins are specific to the Arduino board type and
// cannot be remapped to alternate pins.  For Arduino Uno,
// Duemilanove, etc., pin 11 = MOSI, pin 12 = MISO, pin 13 = SCK.

#define TFT_DC 9
#define TFT_CS 10
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

#define SD_CS 4
#define SD_FAT_TYPE 3

  SdFat                SD;         // SD card filesystem
  Adafruit_ImageReader reader(SD); // Image-reader object, pass in SD filesys
  FatVolume        filesys;

Adafruit_Image       img;        // An image loaded into RAM
int32_t              width  = 0, // BMP image dimensions
                     height = 0;





ImageReturnCode status; // Status from image-reading functions


// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 150
#define TS_MINY 130
#define TS_MAXX 3800
#define TS_MAXY 4000
#define TS_MIN_PRESSURE 200

Adafruit_TSC2007 ts;

//Globals

int hexlines[6];
int hexanum;

File32 myFile;
int mode;

void setup(void) {

  
  Serial.begin(9600);

  tft.begin();

  if (!ts.begin()) {
    Serial.println("Couldn't start touchscreen controller");
    while (1);
  }
  Serial.println("Touchscreen started");
  
  yield();

  Serial.print("Initializing SD card...");
   if(!SD.begin(SD_CS, SD_SCK_MHZ(25))) { // ESP32 requires 25 MHz limit
    Serial.println(F("SD begin() failed"));
    for(;;); // Fatal error, do not continue
  }

  tft.setRotation(0);
  welcome_screen();
  mode = -1;

}

void loop() {
  // put your main code here, to run repeatedly:


  uint16_t x, y, z1, z2;
  if (ts.read_touch(&x, &y, &z1, &z2) && (z1 > TS_MIN_PRESSURE)) {

    if(mode == -1) //Basic Instructions
    {
      tft.fillScreen(ILI9341_WHITE);
    
      reader.drawBMP("/query.bmp", tft, 0, 0); //This takes up less dynamic memory than displaying text to the screen.
    
      mode++;
    
    }
  
    else if(mode == 0)  //Throw coins, build and display the hexagram
    {
    
      // Retrieve a point  
      TS_Point p = ts.getPoint();
   
      /*
      Serial.print("p.x = "); Serial.print(p.x);
      Serial.print("\tp.y= "); Serial.print(p.y);
      Serial.print("\tPressure = "); Serial.println(p.z);  
      Serial.println("");
      */

      randomSeed(p.x^p.y^p.z); //where the touchscreen is touched is our entropy source
          
      tft.fillScreen(ILI9341_WHITE);

  
      build_hexagram();
     
      draw_hexagram();

      find_hex_num();
      //Serial.println(hexanum);

      printHexaName();

      mode++;
    
    }
  
    else if(mode == 1)  //Display Judgement and Image
    {
      printReading();
      mode++;
    }

    else if(mode == 2) //Change Old Lines
    {
      change_hexagram();
      draw_hexagram();

      find_hex_num();
      Serial.println(hexanum);

      printHexaName();

      mode++;
    }

    else if(mode == 3) //Display Judgment and Image of new Hexagram
    {
      printReading();
      mode++;
    }

    else if(mode == 4) //Back to the begining
    {
      welcome_screen();
      mode = 0;
    }
  
  } 
}

/*
throwcoins()
returns a value between 6 and 9
6 = old_yin
7 = young_yang
8 = young_yin
9 = old_yang
*/
int throwcoins()
{
 long coin1;
 long coin2; 
 long coin3;
 int hexline;


 initCoinScreen();
 
 //coin1=random(2);
 coin1=faircoin();
 if(coin1 == 1){
    tft.println("YIN");
    reader.drawBMP("yin.bmp", tft, 0, 64);
  }
  

  if(coin1 == 0){
    tft.println("YANG");
    reader.drawBMP("yang.bmp", tft, 0, 64);
  }

 initCoinScreen();
 
 //coin2=random(2);
 coin2=faircoin();
 if(coin2 == 1){
    tft.println("YIN");
    reader.drawBMP("yin.bmp", tft, 0, 64);
  }
  

  if(coin2 == 0){
    tft.println("YANG");
    reader.drawBMP("yang.bmp", tft,  0, 64);
  }

 initCoinScreen();
 
 //coin3=random(2);
 coin3=faircoin();
 if(coin3 == 1){
    tft.println("YIN");
    reader.drawBMP("yin.bmp", tft, 0, 64);
  }
  

  if(coin3 == 0){
    tft.println("YANG");
    reader.drawBMP("yang.bmp", tft,  0, 64);
  }

 hexline=coin1+coin2+coin3+6;
 
 return hexline;

}

int build_hexagram()
{
 int x=0;
 
 while (x<6)
   {
   hexlines[x]=throwcoins();
   x++;
  }
 return 0;
}

void draw_hexagram()
{
  tft.fillScreen(ILI9341_WHITE);

  int y = 20;
  int z = 5;
  
  do
  {
    if(hexlines[z] == 9)
    {
      drawYangOld(y); 
    }

    if(hexlines[z] == 8)
    {
      drawYin(y);
    }

    if(hexlines[z] ==  7)
    {
      drawYang(y);
    }

    if(hexlines[z] == 6)
    {
      drawYinOld(y);
    }

    z--;
    y = y+30;
    
  } while (z>=0);
  
}

void find_hex_num()
{
 int lower_trigram [3];
 int upper_trigram [3];
 int l_tri_num;
 int u_tri_num;
 
 int hex_array [64] = {1,34,5,26,11,9,14,43,25,51,3,27,24,42,21,17,6,40,29,4,7,59,64,47,33,62,39,52,15,53,56,31,12,16,8,23,2,20,35,45,44,32,48,18,46,57,50,28,13,55,63,22,36,37,30,49,10,54,60,41,19,61,38,58};
 int* hex_dex [8];  //treat hex_array as a 8x8 grid
 
 int x;
  
  

 // divide hexagram into two trigrams
  lower_trigram[0] = hexlines [0];
  lower_trigram[1] = hexlines [1];
  lower_trigram[2] = hexlines [2];
  upper_trigram[0] = hexlines [3];
  upper_trigram[1] = hexlines [4];
  upper_trigram[2] = hexlines [5];
 

 // convert 6 to 8 and 9 to 7
 
 for(x=0 ; x<3 ; x++)
   {
   if (lower_trigram[x] == 6){lower_trigram[x]=8;}
   if (lower_trigram[x] == 9){lower_trigram[x]=7;}
   if (upper_trigram[x] == 6){upper_trigram[x]=8;}
   if (upper_trigram[x] == 9){upper_trigram[x]=7;}
  }
 
 // assign tri_num for array index
 
 if ( (lower_trigram[0] == 7) && (lower_trigram[1] == 7) && (lower_trigram[2] ==7) )
  {l_tri_num = 0;}
  
 if ( (lower_trigram[0] == 7) && (lower_trigram[1] == 8) && (lower_trigram[2] ==8) )
  {l_tri_num = 1;}
  
 if ( (lower_trigram[0] == 8) && (lower_trigram[1] == 7) && (lower_trigram[2] ==8) )
  {l_tri_num = 2;}  
  
 if ( (lower_trigram[0] == 8) && (lower_trigram[1] == 8) && (lower_trigram[2] ==7) )
  {l_tri_num = 3;}  
 
 if ( (lower_trigram[0] == 8) && (lower_trigram[1] == 8) && (lower_trigram[2] ==8) )
  {l_tri_num = 4;}  
  
 if ( (lower_trigram[0] == 8) && (lower_trigram[1] == 7) && (lower_trigram[2] ==7) )
  {l_tri_num = 5;}  
  
 if ( (lower_trigram[0] == 7) && (lower_trigram[1] == 8) && (lower_trigram[2] ==7) )
  {l_tri_num = 6;}  
  
 if ( (lower_trigram[0] == 7) && (lower_trigram[1] == 7) && (lower_trigram[2] ==8) )
  {l_tri_num = 7;}  
  
 
 if ( (upper_trigram[0] == 7) && (upper_trigram[1] == 7) && (upper_trigram[2] ==7) )
  {u_tri_num = 0;}
  
 if ( (upper_trigram[0] == 7) && (upper_trigram[1] == 8) && (upper_trigram[2] ==8) )
  {u_tri_num = 1;}
  
 if ( (upper_trigram[0] == 8) && (upper_trigram[1] == 7) && (upper_trigram[2] ==8) )
  {u_tri_num = 2;}  
  
 if ( (upper_trigram[0] == 8) && (upper_trigram[1] == 8) && (upper_trigram[2] ==7) )
  {u_tri_num = 3;}  
 
 if ( (upper_trigram[0] == 8) && (upper_trigram[1] == 8) && (upper_trigram[2] ==8) )
  {u_tri_num = 4;}  
  
 if ( (upper_trigram[0] == 8) && (upper_trigram[1] == 7) && (upper_trigram[2] ==7) )
  {u_tri_num = 5;}  
  
 if ( (upper_trigram[0] == 7) && (upper_trigram[1] == 8) && (upper_trigram[2] ==7) )
  {u_tri_num = 6;}
  
 if ( (upper_trigram[0] == 7) && (upper_trigram[1] == 7) && (upper_trigram[2] ==8) )
  {u_tri_num = 7;}
  
 
  
 //intialize the array
  for (x=0;x<8;x++)
    {
     hex_dex[x]=hex_array + x*8; //pointer arithmatic
    }
 hexanum = hex_dex [l_tri_num] [u_tri_num];   
 
     
}

// change_hexagram()
// Change "old" lines into new
void change_hexagram()
{
int x;

for (x=0 ; x<6 ; x++)
  {
   if (hexlines[x] == 9)
    {
     hexlines[x]= 8;
    }
   if (hexlines[x] == 6)
    {
     hexlines[x] = 7;
    } 
  }
}

void printHexaName()
{
  String dir = "readings/";
  String ext = ".txt";
  String filename = dir + hexanum + ext;

  myFile = SD.open(filename, FILE_READ);
  if (myFile) {
    Serial.println(filename);
  
    int recNum = 0; // We have read 0 records so far
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      
    String list = myFile.readStringUntil('\r');
    //Serial.println(list);
    recNum++; // Count the record

    if(recNum == 1)
    {
       // Hey, we found the one that we want to do something with
        tft.setCursor(60,200);
        tft.setTextColor(ILI9341_BLACK);
        tft.setTextSize(3);

        tft.println(list);
    }

    if(recNum == 2)
    {
        //tft.setCursor(60,220);
        tft.setTextColor(ILI9341_BLACK);
        tft.setTextSize(1);

        tft.println(list);
        //tft.println("\n\n Touch anywhere to continue..."); //Had to shave bytes or else the bitmaps wouldn't load.  Don't uncomment this
    }
      
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("Error opening " + filename);
  }
}

void printReading()
{

  tft.fillScreen(ILI9341_WHITE);
  tft.setCursor(0,0);
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(1);
    
  String dir = "readings/";
  String ext = ".txt";
  String filename = dir + hexanum + ext;

  myFile = SD.open(filename, FILE_READ);
  if (myFile) {
    Serial.println(filename);
  

  // read from the file until there's nothing else in it:
    while (myFile.available()) {
      //Serial.write(myFile.read());
      String ching = myFile.readStringUntil('\n');
      //Serial.write(ching);

    
    tft.println(ching);
    
    }  
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("Error opening " + filename);
  }

  tft.println("\n\n Touch anywhere to continue...");
  
}

void initCoinScreen()
{
  tft.fillScreen(ILI9341_WHITE);
  tft.setCursor(64,10);
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(4);

}

void drawYin(uint16_t y0) 
{
  //broken line
  tft.fillRect(20, y0, 90, 20, ILI9341_BLACK);
  tft.fillRect(130, y0, 90, 20, ILI9341_BLACK);
  
}

void drawYang(uint16_t y0)
{
  //solid line

  tft.fillRect(20, y0, 200, 20, ILI9341_BLACK);
  
}

void drawYinOld(uint16_t y0) 
{
  //broken line
  tft.fillRect(20, y0, 90, 20, ILI9341_BLACK);
  tft.fillRect(130, y0, 90, 20, ILI9341_BLACK);

  tft.setCursor(115,y0+2);
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(2);
  tft.println("X");
  
}

void drawYangOld(uint16_t y0)
{
  //solid line

  tft.fillRect(20, y0, 200, 20, ILI9341_BLACK);
  tft.setCursor(115,y0+2);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.println("O");
  
}

// This function is a Von Neuman Extractor
// Because the Arduino Random Number Generation
// is not the best, I use this algorithm by
// John Von Neumon that allows a fair result
// from biased inputs.

long faircoin()
{
  int fair = 0;
  long biascoin1;
  long biascoin2;

  while(!fair)
  {
    biascoin1 = random(2);
    biascoin2 = random(2);

    if(biascoin1 != biascoin2)
    {
      fair = 1;
    }
  }

  return biascoin1;
 
}

void welcome_screen()
{

  //Load the array for the vaule of Hexagram 30 to use as a logo
  hexlines[0] = 7;
  hexlines[1] = 8;
  hexlines[2] = 7;
  hexlines[3] = 7;
  hexlines[4] = 8;
  hexlines[5] = 7;

  draw_hexagram();

  tft.setCursor(0,200);
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(3);

  tft.println("Pocket");
  tft.println("I-Ching");

  tft.setTextSize(2);

  tft.println("CodePoetry by:");
  tft.println("L. Christopher Bird");

  tft.setTextSize(1);

  tft.println("\n\nTouch Anywhere to Begin...");
}


