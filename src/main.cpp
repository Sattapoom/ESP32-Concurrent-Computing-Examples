#include <Arduino.h>
#include <Wire.h>
#include <Update.h>
#include "SPIFFS.h"

// Pin Configuration
#define LED_PIN 15
#define BUTTON_PIN 18

// Task Handles
TaskHandle_t Task1Handle = NULL;
TaskHandle_t Task2Handle = NULL;

// Task1: LED Blinking (Core 0)
void Task1_LEDBlink(void *pvParameters)
{
  pinMode(LED_PIN, OUTPUT);

  while (1)
  {
    digitalWrite(LED_PIN, HIGH);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    digitalWrite(LED_PIN, LOW);
    vTaskDelay(500 / portTICK_PERIOD_MS);

    Serial.println("LED Blinking on Core 0");
  }
}

// Task2: Button Reading (Core 1)
void Task2_ButtonRead(void *pvParameters)
{
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  while (1)
  {
    int buttonState = digitalRead(BUTTON_PIN);

    if (buttonState == LOW)
    {
      Serial.println("Button Pressed on Core 1!");
      vTaskDelay(200 / portTICK_PERIOD_MS);
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void setup()
{
  // Initialize Serial Communication
  Serial.begin(115200);

  // Create Task1 on Core 0
  xTaskCreatePinnedToCore(
      Task1_LEDBlink, // Task Function
      "LED Blink",    // Task Name
      10000,          // Stack Size
      NULL,           // Parameters
      1,              // Priority
      &Task1Handle,   // Task Handle
      0               // Core Number (0)
  );

  // Create Task2 on Core 1
  xTaskCreatePinnedToCore(
      Task2_ButtonRead, // Task Function
      "Button Read",    // Task Name
      10000,            // Stack Size
      NULL,             // Parameters
      1,                // Priority
      &Task2Handle,     // Task Handle
      1                 // Core Number (1)
  );
}

void loop()
{
  // ไม่จำเป็นต้องใช้งาน เนื่องจากใช้ FreeRTOS Tasks
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}