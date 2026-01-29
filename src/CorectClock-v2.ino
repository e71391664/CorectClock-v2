#include <Arduino.h>
#include "RtcManager.h"
#include "DisplayGyver.h"
#include <GyverOLED.h>
#include "EncoderManager.h"
#include "ReleManager.h"


#include <RTClib.h> // Потрібно для DateTime now;

// Визначаємо DEBUG_MODE для умовної компіляції Serial.print
//#define DEBUG_MODE

// Створюємо глобальний об'єкт класу
RtcManager rtcManager;
ReleManager releManager;
DisplayGyver displayManager(rtcManager);
EncoderManager encoderManager (displayManager, releManager, rtcManager);  

//Відображення на дисплеї, реакція на уставку через енкодер та обробка реле
void normalWorkStage(){
  bool isDisplayOn = displayManager.getDisplayOn();

  if (encoderManager.inEditMode()) {
    if (isDisplayOn) displayManager.showOffset(encoderManager.getOffset());
  } else if (releManager.isBusy()) {
    if (isDisplayOn) displayManager.showStartProcessing();
  } else {
      if (isDisplayOn) displayManager.showTimeAndTemperature();
      releManager.blinkOnNormalMode(rtcManager.now());
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); // Ініціалізація Serial-порту для виведення
    while (!Serial) { ; // Чекаємо підключення Serial-порту (тільки для деяких плат, напр. Leonardo)
    }

    rtcManager.init();// Ініціалізуємо RTC-модуль через наш клас
    displayManager.init();// Ініціалізація дисплея
    encoderManager.init();
    releManager.init();

    delay(20);  
}

void loop() {
  encoderManager.update();
  releManager.update();
  encoderManager.encoderCheck(); // Переміщено сюди, щоб завжди перевіряти енкодер
  displayManager.autoPowerOff(); // Переміщено сюди, щоб вимикати екран, коли він увімкнений
  
  // Перевірка кнопок для пробудження дисплея
  if (!displayManager.getDisplayOn()) {
     if (rtcManager.readConfirm() || rtcManager.readBack() || encoderManager.inEditMode()) {      
        displayManager.wakeDisplay();
     }
  }
  
  // --- Відображення тільки якщо дисплей увімкнений ---
  if (displayManager.getDisplayOn()) {
    if (displayManager.isEditing()){
       displayManager.updateEdit();
    } else {
       normalWorkStage();
       if (rtcManager.readConfirm()) { // BTN_CONN
         displayManager.startEdit();
         displayManager.wakeDisplay();
       }
    }
  } else {
     // Якщо дисплей вимкнений, все одно виконуємо фонові задачі (реле)
     normalWorkStage();
  }

  delay(50);
}
