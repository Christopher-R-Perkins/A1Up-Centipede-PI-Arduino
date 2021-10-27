#define _SS_MAX_RX_BUFF 64
#include <SoftwareSerial7N1.h>
#include <HID-Project.h>

//                  a      b      c      p1     p2     vup    vdown  up     right  down   left
byte checks[] =    {false, false, false, false, false, false, false, false, false, false, false};
const int pins[] = {2    , 3    , 4    , 5    , 6    , 7    , 8};
// We don't use 0 and 1 as we'll be using the uart to communicate with pi.
byte mode = 'j';
// 'j' Joypad, 't' trackball, 'm' SCUMMVM


unsigned long joypad_time = 0;
int x = 0;
int y = 0;

const uint8_t keys[] = {KEY_A, KEY_S, KEY_D, KEY_ENTER, KEY_DELETE};

SoftwareSerial7N1 trackballSerial(10,11);

unsigned long exit_time = 0;
bool check_exit = false;

void setup() {
    // Five Buttons
    pinMode(2, INPUT_PULLUP);
    pinMode(3, INPUT_PULLUP);
    pinMode(4, INPUT_PULLUP);
    pinMode(5, INPUT_PULLUP);
    pinMode(6, INPUT_PULLUP);
    // Volume Switch
    pinMode(7, INPUT_PULLUP);
    pinMode(8, INPUT_PULLUP);
    // UART
    pinMode(10, INPUT);
    pinMode(11, OUTPUT);
    
    Keyboard.begin();
    Mouse.begin();
    Consumer.begin();
    
    Serial.begin(115200);
    //while(!Serial);
    
    trackballSerial.begin(115200);
    trackballSerial.listen();
}

void input_outputter(int check_index, byte device, uint8_t keycode) { // my way to debounce without tracking time
  switch(checks[check_index]) {
  case 0: // Unpressed
    if (digitalRead(pins[check_index]) == LOW) checks[check_index]++;
      break;
  case 1:  // pressed for 1/120th of a second
    if (digitalRead(pins[check_index]) == LOW) {
      checks[check_index]++;
      if (device == 'k') Keyboard.press((KeyboardKeycode)keycode);
      else if (device == 'm') Mouse.press(keycode);
      else Consumer.press(keycode);
    } else {
      checks[check_index]--;
    }
    break;
  case 2: // pressed for 1/60th and above.
    if (digitalRead(pins[check_index]) == HIGH) checks[check_index]++;
    break;
  case 3: // unpressed for 1/20th of a second
    if (digitalRead(pins[check_index]) == LOW) checks[check_index]--;
    else {
      checks[check_index] = 0;
      if (device == 'k') Keyboard.release((KeyboardKeycode)keycode);
      else if (device == 'm') Mouse.release(keycode);
      else Consumer.release(keycode);
    }
    break;
  default:
    break;
  }
}

int new_mode = 'j';

void loop() {
  unsigned long currentMicros = micros();
  
  // We need to be able to change the mode via serial port
  while(Serial.available() > 0) {
    new_mode = Serial.read();
  }
  if (new_mode != mode) {
    mode = new_mode;
    
    // Since bindings might change we got clear our outputs
    Mouse.releaseAll();
    Keyboard.releaseAll();
    x = 0;
    y = 0;
    for(int i = 0; i < 11; i++) checks[i] = 0;
    
    // We may have exited to change modes
    check_exit = false;
  }
    
  int vertical = 0;
  int horizontal = 0;
    
  int bytesToRead = trackballSerial.available();
  for(int i = 0; i < bytesToRead-3; i++) {
    if (trackballSerial.read() == 127) {
      byte trackball[] = {0, 0, 0};
      for(int j = 0; j<3; j++) {
        trackball[j] = trackballSerial.read(); 
        i++;
      }
      horizontal += dataConvert(trackball[0]);
      vertical += dataConvert(trackball[1]);
    }
  }

  // First Joystick/Mouse Movement.
  switch(mode){
    case 'j':
      // Basically we only do joypad stuff 50 times a second!
      if (millis() - joypad_time < 20) {
        x+= horizontal;
        y+= vertical;
        break;
      }
      
      if (y < -2) {
        if (!checks[7]) {
          checks[7] = true;
          Keyboard.press(KEY_UP);
        }
      } else if (checks[7]) {
        checks[7] = false;
        Keyboard.release(KEY_UP);
      }
      if (y > 2) {
        if (!checks[9]) {
          checks[9] = true;
          Keyboard.press(KEY_DOWN);
        }
      } else if (checks[9]) {
        checks[9] = false;
        Keyboard.release(KEY_DOWN);
      }

      if (x > 8) {
        if (!checks[8]) {
          checks[8] = true;
          Keyboard.press(KEY_RIGHT);
        }
      } else if (checks[8]) {
        checks[8] = false;
        Keyboard.release(KEY_RIGHT);
      }
      if (x < -8) {
        if (!checks[10]) {
          checks[10] = true;
          Keyboard.press(KEY_LEFT);
        }
      } else if (checks[10]) {
        checks[10] = false;
        Keyboard.release(KEY_LEFT);
      }
      x = 0;
      y = 0;
      joypad_time = millis();
      break;
    case 'm':
      if (abs(horizontal) > 4) horizontal = horizontal * 2;
      else if (abs(horizontal) > 2) horizontal = horizontal * 3/2;
      if (abs(vertical)  > 4) vertical = vertical * 2;
      else if (abs(vertical) > 2) vertical = vertical * 3/2;
    case 't':
      Mouse.move((char)horizontal, (char)vertical, 0);
      break;
    default:
      break;
  }

    // now we deal with buttons:)
    switch(mode) {
      case 'j':
      case 't':
        for(int i = 0; i < 5; i++) input_outputter(i,'k',keys[i]);
        if (checks[3] > 1) {
          if (!check_exit) {
            check_exit = true;
            exit_time = millis();
          }
          if ((millis()-exit_time >= 5000)) {
            Keyboard.write(KEY_ESC);
            new_mode = 'j';
          }
        } else {
          check_exit = false;
        }
        break;
      case 'm':
        input_outputter(0,'m',MOUSE_LEFT);
        input_outputter(1,'m',MOUSE_RIGHT);
        input_outputter(2,'k',KEY_ESC);
        input_outputter(3,'k',KEY_F5);
        input_outputter(4,'k',KEY_F7);
        
        if (checks[3] > 1) {
          if (!check_exit) {
            check_exit = true;
            exit_time = millis();
          }
          if (millis()-exit_time >= 5000) {
            Keyboard.press(KEY_LEFT_CTRL);
            Keyboard.write(KEY_Q);
            Keyboard.release(KEY_LEFT_CTRL);
            new_mode = 'j';
          }
        } else {
          check_exit = false;
        }
        break;
      default:
        break;
    }

    // Finally let's fix volume!
    input_outputter(5,'c',MEDIA_VOLUME_DOWN);
    input_outputter(6,'c',MEDIA_VOLUME_UP);
}

int dataConvert(byte b) 
{
    if (b == 126)
      return -1;
    if (b == 1) return 1;
    return 0;
}
