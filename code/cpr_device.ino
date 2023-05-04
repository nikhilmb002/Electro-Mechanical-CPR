#include <LiquidCrystal_I2C.h>       // Library for I2C LCD display
#include <Wire.h>                    // I2C communication library
#include "MAX30100_PulseOximeter.h"  // Library for MAX30100 pulse oximeter sensor

#define REPORTING_PERIOD_MS 1000     // Update interval for displaying readings (1 second)

// Initialize LCD with I2C address 0x27, 16 columns, 2 rows
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Custom characters for emoticons (feedback based on SpO₂ levels)
byte smile[] = {    // 😀 (Good oxygen level)
B00000, B00000, B01010, B00000, B10001, B01110, B00000, B00000
};
byte mod[] = {      // 😐 (Moderate oxygen level)
B00000, B00000, B01010, B00000, B11111, B00000, B00000, B00000
};
byte sad[] = {      // ☹️ (Low oxygen level, danger)
B00000, B00000, B01010, B00000, B01110, B10001, B00000, B00000
};

// Pulse oximeter object
PulseOximeter pox;
uint32_t tsLastReport = 0;   // Variable to track last update time

// Callback function called whenever a heartbeat is detected
void onBeatDetected() 
{ 
    Serial.println("Beat!!!");  // Print heartbeat detection in serial monitor
} 

// Setup function runs once at startup
void setup() 
{ 
    Serial.begin(115200);   // Initialize serial communication for debugging
    
    // Initialize LCD
    lcd.init();             
    lcd.backlight();        // Turn on LCD backlight
    
    // Load custom characters into LCD
    lcd.createChar(1 , smile);
    lcd.createChar(2 , mod);
    lcd.createChar(3 , sad);
    
    // Welcome message on LCD
    lcd.setCursor(0, 0); 
    lcd.print(" Pluse");    // (Typo: Should be "Pulse")
    lcd.setCursor(0, 1); 
    lcd.print(" Oximeter");
    delay(2000);            // Display intro for 2 seconds

    // Initialize MAX30100 sensor
    if (!pox.begin()) { 
        Serial.println("FAILED");   // If sensor not detected
        for (;;);                   // Infinite loop to halt system
    } else { 
        Serial.println("SUCCESS");  // Sensor initialized successfully
    } 

    // Set IR LED current for pulse detection
    pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
    
    // Register callback for heartbeat detection
    pox.setOnBeatDetectedCallback(onBeatDetected);
} 

// Main loop: runs continuously
void loop() 
{ 
    pox.update();   // Update sensor readings
    
    // Report data every REPORTING_PERIOD_MS (1 second)
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) { 
        lcd.clear();   // Clear LCD for new data
        
        // Display heart rate (BPM) on LCD
        lcd.setCursor(0 , 0); 
        lcd.print("BPM : "); 
        lcd.print(pox.getHeartRate()); 
        
        // Display oxygen saturation (SpO₂ %) on LCD
        lcd.setCursor(0 , 1); 
        lcd.print("Sp02: "); 
        lcd.print(pox.getSpO2()); 
        lcd.print("%"); 
        
        tsLastReport = millis();  // Reset timer
        
        // Provide visual feedback based on SpO₂ level
        if (pox.getSpO2() >= 96) {       // Normal condition
            lcd.setCursor(15 , 1); 
            lcd.write(1);   // Display smile face
        } 
        else if (pox.getSpO2() <= 95 && pox.getSpO2() >= 91) {  // Borderline
            lcd.setCursor(15 , 1); 
            lcd.write(2);   // Display neutral face
        } 
        else if (pox.getSpO2() <= 90) {  // Critical (low oxygen level)
            lcd.setCursor(15 , 1); 
            lcd.write(3);   // Display sad face (alert)
        } 
    } 
}

