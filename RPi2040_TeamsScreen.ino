#include <LittleFS.h>
#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include <Keyboard.h>
#include <Mouse.h>

TFT_eSPI tft = TFT_eSPI();       // Invoke custom library, select 

// This is the file name used to store the touch coordinate
// calibration data. Cahnge the name to start a new calibration.
#define CALIBRATION_FILE "/TouchCalData3"

// Set REPEAT_CAL to true instead of false to run calibration
// again, otherwise it will only be done once.
// Repeat calibration if you change the screen rotation.
#define REPEAT_CAL false

#define BUTTON_DELAY    250
#define MOUSE_DELAY     240

unsigned long btnTimer;
unsigned long mouseTimer;
unsigned long mouseBlink;
boolean mouseMove = true;
boolean mouseBlinkState = false;


//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void setup(void)
{
  tft.init();

  // Set the rotation before we calibrate
  tft.setRotation(0);

  // call screen calibration
  touch_calibrate();

  // clear screen
  uint16_t bkgdclr = tft.color565(24, 24, 48);
  tft.fillScreen(bkgdclr);
  drawBmp("/buttons2.bmp", 0, 0);
/*  
  paintButton(15, 10, "MOUSE");
  paintButton(125, 10, "LIGHT");
  paintButton(15, 110, "LOCK");
  paintButton(125, 110, ".");
  paintButton(15, 210, "CAM");
  paintButton(125, 210, "MUTE");
*/
  Keyboard.begin();
  Mouse.begin();
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void loop()
{
  uint16_t x, y;
  
  // See if there's any touch data for us
  if (tft.getTouch(&x, &y))
  {
    if(millis() - btnTimer >= BUTTON_DELAY) {
      btnTimer = millis();
      
      int btnCode = 0;
      boolean yValid = false;
      if(y > 10 && y < 90) {
        btnCode = 1;
        yValid = true;
      } else if (y > 110 && y < 190) {
        btnCode = 3;
        yValid = true;
      } else if(y > 10 && y < 290) {
        btnCode = 5;
        yValid = true;
      }
      if(x > 15 && x < 115 && yValid) {
        btnCode += 0;
        yValid = true;
      } else if(x > 125 && x < 225 && yValid) {
        btnCode += 1;
        yValid = true;
      } else {
        yValid = false;
      }
      if(!yValid) 
        btnCode = 0;

      // Process key presses
      switch(btnCode) {
        case 1:
              mouseMove = !mouseMove;
              break;
        case 2:
              Keyboard.press(KEY_LEFT_GUI);
              delay(15);
              Keyboard.press('l');
              delay(200);
              Keyboard.release('l');
              delay(100);
              Keyboard.release(KEY_LEFT_GUI);
              delay(15);
              break;
        case 3:
              // Some Philips Hue light settings
              break;
        case 4:
              mouseLooping();	// Undocumented feature ;)
              break;
        case 5:
              Keyboard.press(KEY_LEFT_SHIFT);
              delay(15);
              Keyboard.press(KEY_LEFT_CTRL);
              delay(15);
              Keyboard.press('o');
              delay(200);
              Keyboard.release('o');
              delay(100);
              Keyboard.release(KEY_LEFT_CTRL);
              delay(15);
              Keyboard.release(KEY_LEFT_SHIFT);
              delay(15);
              break;
        case 6:
              Keyboard.press(KEY_LEFT_SHIFT);
              delay(15);
              Keyboard.press(KEY_LEFT_CTRL);
              delay(15);
              Keyboard.press('m');
              delay(200);
              Keyboard.release('m');
              delay(100);
              Keyboard.release(KEY_LEFT_CTRL);
              delay(15);
              Keyboard.release(KEY_LEFT_SHIFT);
              delay(15);
              break;
        default:
              // Bruh, do nothing
              ;
      }
    }
  }
  if(mouseMove && millis() - mouseTimer >= MOUSE_DELAY * 1000) {
    mouseTimer = millis();
    Mouse.move(5, 5, 0);
    delay(5);
    Mouse.move(-5, -5, 0);
  }
  if(millis() - mouseBlink >= 1000) {
    mouseBlink = millis();
    mouseBlinkState = !mouseBlinkState;
    if(mouseMove && mouseBlinkState) {
      tft.fillCircle(95, 85, 5, tft.color565(131, 173, 215));
    } else {
      tft.fillCircle(95, 85, 5, tft.color565(17, 104, 212));
    }
  }
}
//------------------------------------------------------------------------------------------

void touch_calibrate()
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // check file system exists
  if (!LittleFS.begin()) {
    LittleFS.format();
    LittleFS.begin();
  }

  // check if calibration file exists and size is correct
  if (LittleFS.exists(CALIBRATION_FILE)) {
    if (REPEAT_CAL)
    {
      // Delete if we want to re-calibrate
      LittleFS.remove(CALIBRATION_FILE);
    }
    else
    {
      File f = LittleFS.open(CALIBRATION_FILE, "r");
      if (f) {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL) {
    // calibration data valid
    tft.setTouch(calData);
  } else {
    // data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    if (REPEAT_CAL) {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Set REPEAT_CAL to false to stop this running again!");
    }

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    // store data
    File f = LittleFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}

void paintButton(int x, int y, char* txt) {
  uint16_t bkgdclr = tft.color565(24, 24, 48);
  uint16_t btnbaseclr = tft.color565(32, 40, 96);

  tft.fillRoundRect(x, y, 100, 90, 5, TFT_BLACK);
  tft.fillRoundRect(x + 2, y + 2, 96, 86, 5, btnbaseclr);

  tft.setCursor(x + 10, y + 25);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, btnbaseclr);
  tft.print(txt);
}


void listAllFilesInDir(String dir_path)
{
  Dir dir = LittleFS.openDir(dir_path);
  while(dir.next()) {
    if (dir.isFile()) {
      // print file names
      //Serial.print("File: ");
      //Serial.println(dir_path + dir.fileName());
    }
    if (dir.isDirectory()) {
      // print directory names
      //Serial.print("Dir: ");
      //Serial.println(dir_path + dir.fileName() + "/");
      // recursive file listing inside new directory
      listAllFilesInDir(dir_path + dir.fileName() + "/");
    }
  }
}


// Bodmers BMP image rendering function
void drawBmp(const char *filename, int16_t x, int16_t y) {

  if ((x >= tft.width()) || (y >= tft.height())) return;

  fs::File bmpFS;

  // Open requested file on SD card
  bmpFS = LittleFS.open(filename, "r");

  if (!bmpFS)
  {
    //Serial.print("File not found");
    return;
  }

  uint32_t seekOffset;
  uint16_t w, h, row, col;
  uint8_t  r, g, b;

  uint32_t startTime = millis();

  if (read16(bmpFS) == 0x4D42)
  {
    read32(bmpFS);
    read32(bmpFS);
    seekOffset = read32(bmpFS);
    read32(bmpFS);
    w = read32(bmpFS);
    h = read32(bmpFS);

    if ((read16(bmpFS) == 1) && (read16(bmpFS) == 24) && (read32(bmpFS) == 0))
    {
      y += h - 1;

      bool oldSwapBytes = tft.getSwapBytes();
      tft.setSwapBytes(true);
      bmpFS.seek(seekOffset);

      uint16_t padding = (4 - ((w * 3) & 3)) & 3;
      uint8_t lineBuffer[w * 3 + padding];

      for (row = 0; row < h; row++) {
        
        bmpFS.read(lineBuffer, sizeof(lineBuffer));
        uint8_t*  bptr = lineBuffer;
        uint16_t* tptr = (uint16_t*)lineBuffer;
        // Convert 24 to 16 bit colours
        for (uint16_t col = 0; col < w; col++)
        {
          b = *bptr++;
          g = *bptr++;
          r = *bptr++;
          *tptr++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        }

        // Push the pixel row to screen, pushImage will crop the line if needed
        // y is decremented as the BMP image is drawn bottom up
        tft.pushImage(x, y--, w, 1, (uint16_t*)lineBuffer);
      }
      tft.setSwapBytes(oldSwapBytes);
      //Serial.print("Loaded in "); Serial.print(millis() - startTime);
      //Serial.println(" ms");
    }
    //else Serial.println("BMP format not recognized.");
  }
  bmpFS.close();
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(fs::File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(fs::File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}


void mouseLooping() {
  float r = 150.0;
  float ox = 0.0;
  float oy = 0.0;
  for (float a = 0.0; a < 2.0 * 3.141592; a += 3.141592 / 30.0) {
	float ax = r - r * cos(a);
	float ay = r * sin(a);
	float dx = ax - ox;
	float dy = ay - oy;
	Mouse.move(dx, dy, 0);
	ox = ax;
	oy = ay;
	delay(10);
  }
}
