
/*
 * Pocket I-Ching 2.0
 * CodePoetry by L. Christopher Bird zenmondo@gmail.com
 * 
 * An electronic oracle based on the Book of Changes.
 * 
 * Uses the coin method for constructing a hexagram.
 * 
 * For use on an Arduino Uno R3 and a 2.8" TFT Touch Shield for Arduino With Resisitive Touch Screen
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


#include <Adafruit_GFX.h>    // Core graphics library
#include "Adafruit_ILI9341.h" // Hardware-specific library
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_STMPE610.h>

// TFT display and SD card will share the hardware SPI interface.
// Hardware SPI pins are specific to the Arduino board type and
// cannot be remapped to alternate pins.  For Arduino Uno,
// Duemilanove, etc., pin 11 = MOSI, pin 12 = MISO, pin 13 = SCK.

#define TFT_DC 9
#define TFT_CS 10
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

#define SD_CS 4

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 150
#define TS_MINY 130
#define TS_MAXX 3800
#define TS_MAXY 4000

// The STMPE610 uses hardware SPI on the shield, and #8
#define STMPE_CS 8
Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);

//Globals

int hexlines[6];
int hexanum;

File myFile;
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
  if (!SD.begin(SD_CS)) {
    Serial.println("failed!");
  }
  Serial.println("OK!");

  tft.setRotation(0);
  welcome_screen();
  mode = -1;

}

void loop() {
  // put your main code here, to run repeatedly:

  if (! ts.touched()) 
  {
    return;
  }

  if(mode == -1) //Basic Instructions
  {
    tft.fillScreen(ILI9341_WHITE);
    
    bmpDraw("query.bmp", 0, 0); //This takes up less dynamic memory than displaying text to the screen.
    
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
    //Serial.println(hexanum);

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
    bmpDraw("yin.bmp", 0, 64);
  }
  

  if(coin1 == 0){
    tft.println("YANG");
    bmpDraw("yang.bmp", 0, 64);
  }

 initCoinScreen();
 
 //coin2=random(2);
 coin2=faircoin();
 if(coin2 == 1){
    tft.println("YIN");
    bmpDraw("yin.bmp", 0, 64);
  }
  

  if(coin2 == 0){
    tft.println("YANG");
    bmpDraw("yang.bmp", 0, 64);
  }

 initCoinScreen();
 
 //coin3=random(2);
 coin3=faircoin();
 if(coin3 == 1){
    tft.println("YIN");
    bmpDraw("yin.bmp", 0, 64);
  }
  

  if(coin3 == 0){
    tft.println("YANG");
    bmpDraw("yang.bmp", 0, 64);
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
 return hexlines;
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

  //tft.println("\n\n Touch anywhere to continue...");  //Had to shave bytes or the bitmaps wouldn't load. Don't uncomment this.
  
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


// This function opens a Windows Bitmap (BMP) file and
// displays it at the given coordinates.  It's sped up
// by reading many pixels worth of data at a time
// (rather than pixel by pixel).  Increasing the buffer
// size takes more of the Arduino's precious RAM but
// makes loading a little faster.  20 pixels seems a
// good balance.

#define BUFFPIXEL 20

void bmpDraw(char *filename, int16_t x, int16_t y) {

  File     bmpFile;
  int      bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3*BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int      w, h, row, col, x2, y2, bx1, by1;
  uint8_t  r, g, b;
  uint32_t pos = 0, startTime = millis();

  if((x >= tft.width()) || (y >= tft.height())) return;

  Serial.println();
  Serial.print(F("Loading image '"));
  Serial.print(filename);
  Serial.println('\'');

  // Open requested file on SD card
  if ((bmpFile = SD.open(filename)) == NULL) {
    Serial.print(F("File not found"));
    return;
  }

  // Parse BMP header
  if(read16(bmpFile) == 0x4D42) { // BMP signature
    Serial.print(F("File size: ")); Serial.println(read32(bmpFile));
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    Serial.print(F("Image Offset: ")); Serial.println(bmpImageoffset, DEC);
    // Read DIB header
    Serial.print(F("Header size: ")); Serial.println(read32(bmpFile));
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if(read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      Serial.print(F("Bit Depth: ")); Serial.println(bmpDepth);
      if((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed

        goodBmp = true; // Supported BMP format -- proceed!
        Serial.print(F("Image size: "));
        Serial.print(bmpWidth);
        Serial.print('x');
        Serial.println(bmpHeight);

        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if(bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }

        // Crop area to be loaded
        x2 = x + bmpWidth  - 1; // Lower-right corner
        y2 = y + bmpHeight - 1;
        if((x2 >= 0) && (y2 >= 0)) { // On screen?
          w = bmpWidth; // Width/height of section to load/display
          h = bmpHeight;
          bx1 = by1 = 0; // UL coordinate in BMP file
          if(x < 0) { // Clip left
            bx1 = -x;
            x   = 0;
            w   = x2 + 1;
          }
          if(y < 0) { // Clip top
            by1 = -y;
            y   = 0;
            h   = y2 + 1;
          }
          if(x2 >= tft.width())  w = tft.width()  - x; // Clip right
          if(y2 >= tft.height()) h = tft.height() - y; // Clip bottom
  
          // Set TFT address window to clipped image bounds
          tft.startWrite(); // Requires start/end transaction now
          tft.setAddrWindow(x, y, w, h);
  
          for (row=0; row<h; row++) { // For each scanline...
  
            // Seek to start of scan line.  It might seem labor-
            // intensive to be doing this on every line, but this
            // method covers a lot of gritty details like cropping
            // and scanline padding.  Also, the seek only takes
            // place if the file position actually needs to change
            // (avoids a lot of cluster math in SD library).
            if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
              pos = bmpImageoffset + (bmpHeight - 1 - (row + by1)) * rowSize;
            else     // Bitmap is stored top-to-bottom
              pos = bmpImageoffset + (row + by1) * rowSize;
            pos += bx1 * 3; // Factor in starting column (bx1)
            if(bmpFile.position() != pos) { // Need seek?
              tft.endWrite(); // End TFT transaction
              bmpFile.seek(pos);
              buffidx = sizeof(sdbuffer); // Force buffer reload
              tft.startWrite(); // Start new TFT transaction
            }
            for (col=0; col<w; col++) { // For each pixel...
              // Time to read more pixel data?
              if (buffidx >= sizeof(sdbuffer)) { // Indeed
                tft.endWrite(); // End TFT transaction
                bmpFile.read(sdbuffer, sizeof(sdbuffer));
                buffidx = 0; // Set index to beginning
                tft.startWrite(); // Start new TFT transaction
              }
              // Convert pixel from BMP to TFT format, push to display
              b = sdbuffer[buffidx++];
              g = sdbuffer[buffidx++];
              r = sdbuffer[buffidx++];
              tft.writePixel(tft.color565(r,g,b));
            } // end pixel
          } // end scanline
          tft.endWrite(); // End last TFT transaction
        } // end onscreen
        Serial.print(F("Loaded in "));
        Serial.print(millis() - startTime);
        Serial.println(" ms");
      } // end goodBmp
    }
  }

  bmpFile.close();
  if(!goodBmp) Serial.println(F("BMP format not recognized."));
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}

