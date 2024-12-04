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

// Shared Variables
volatile bool changeLEDPattern = false;
volatile int currentPattern = 0;

// LED Blinking Patterns (in milliseconds)
const int patterns[][2] = {
    {500, 500},  // Original: 500ms on, 500ms off
    {100, 100},  // Fast blinking
    {1000, 200}, // Long on, short off
    {200, 1000}  // Short on, long off
};
const int NUM_PATTERNS = sizeof(patterns) / sizeof(patterns[0]);

// Mutex for thread-safe pattern changing
SemaphoreHandle_t patternMutex = NULL;

// Task1: LED Blinking (Core 0)
void Task1_LEDBlink(void *pvParameters)
{
  pinMode(LED_PIN, OUTPUT);

  while (1)
  {
    // Check if pattern needs to be changed
    if (changeLEDPattern)
    {
      if (xSemaphoreTake(patternMutex, portMAX_DELAY) == pdTRUE)
      {
        currentPattern = (currentPattern + 1) % NUM_PATTERNS;
        changeLEDPattern = false;
        xSemaphoreGive(patternMutex);
      }
    }

    // Get current pattern
    int onTime, offTime;
    if (xSemaphoreTake(patternMutex, portMAX_DELAY) == pdTRUE)
    {
      onTime = patterns[currentPattern][0];
      offTime = patterns[currentPattern][1];
      xSemaphoreGive(patternMutex);
    }

    // Blink LED according to current pattern
    digitalWrite(LED_PIN, HIGH);
    vTaskDelay(onTime / portTICK_PERIOD_MS);
    digitalWrite(LED_PIN, LOW);
    vTaskDelay(offTime / portTICK_PERIOD_MS);

    Serial.print("LED Blinking on Core 0 - Pattern: ");
    Serial.println(currentPattern);
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
      // Debounce and pattern change
      vTaskDelay(50 / portTICK_PERIOD_MS); // Simple debounce
      if (digitalRead(BUTTON_PIN) == LOW)
      {
        // Signal pattern change
        if (xSemaphoreTake(patternMutex, portMAX_DELAY) == pdTRUE)
        {
          changeLEDPattern = true;
          xSemaphoreGive(patternMutex);
        }

        Serial.println("Button Pressed - Changing LED Pattern!");

        // Wait for button release to prevent multiple triggers
        while (digitalRead(BUTTON_PIN) == LOW)
        {
          vTaskDelay(10 / portTICK_PERIOD_MS);
        }
      }
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void setup()
{
  // Initialize Serial Communication
  Serial.begin(115200);

  // Create Mutex
  patternMutex = xSemaphoreCreateMutex();

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
  // Not used with FreeRTOS Tasks
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}