#include <Arduino.h>

// Deklarieren diverser Variablen
int TestDigInput = 2;
int TestAInput = 0;
int FirstValue = 0;
int SecondInt = 0;
int SecondValue = 0;
int TestValue = 0;
int HDDState = 0;
int HDDTemp = 0;
int PWM = 100;
int StandbyCount = 0;
int Standby = 0;

void setup() {
    // DigInput als Eingang deklarieren
    pinMode(TestDigInput, INPUT_PULLUP);
  Serial.begin(9600);  
  Serial.println("--- Start Serial Monitor SEND_RCVE ---");
}

void loop() {
     // **** Input über USB simulieren ****
    // DigInput zu erstem String
    if (digitalRead(TestDigInput) == HIGH) {
        FirstValue = 100;
    } else {
        FirstValue = 200;
    }

//    Serial.print("Status Digital Input: ");
//    Serial.print("\t");
//    Serial.print(FirstValue);
//    Serial.println();

    // AInput zu zweitem String
    SecondValue = (analogRead(TestAInput) / 25 + 10);

//    Serial.print("Status Analog Input String: ");
//    Serial.print("\t");
//    Serial.print(SecondValue);
//    Serial.println();
       
    // Beide Strings verknüpfen
    TestValue = FirstValue + SecondValue;

//    Serial.print("Status Combined: ");
//    Serial.print("\t");
//    Serial.print(TestValue);
//    Serial.println();
    
    // **** Input über USB simulieren ****

    // HDD Status anhand TestValue-Wert erkennen
    if (TestValue > 160) {
      HDDState = 1;
    } else {
      HDDState = 0;
    }

    Serial.print("HDD State: ");
    Serial.print("\t");
    Serial.print(HDDState);
    Serial.println();

    // Aus erkanntem HDD-Status die HDD Temperatur rausrechnen

    if (HDDState == 1) {
      HDDTemp = TestValue - 200;
    } else {
      HDDTemp = TestValue - 100;
    }

    Serial.print("HDD Temperature: ");
    Serial.print("\t");
    Serial.print(HDDTemp);
    Serial.println();
    


    // Zustand HDD ist eingeschaltet
    if (HDDState == 1) {

            // War vorher der Standby Modus aktiviert, Startwert zurücksetzen
            if (StandbyCount != 0) {
                PWM = 100;
            }

        // Standby Routine zurücksetzen
        Standby = 0;
        StandbyCount = 0;

        // Schleife für PWM - durch PID ersetzen
        if (PWM == 110) {
            PWM = 100;
        } else {
            PWM = PWM + 1;
        }


    // Zustand HDD ist ausgeschaltet
    } else {

        // Ist der vorherige Timer durchgelaufen, Lüfter ausschalten
        if (Standby == 1) {
            PWM = 0;
            
        // Lüfter auf x% laufen lassen
        } else {
            PWM = 25;
            
            // Timer für x%
            if (StandbyCount == 10) {
                Standby = 1;
            }
            StandbyCount = StandbyCount + 1;
        }
    }

    // 1sec Pause nach jedem Durchlauf
    delay(1000);

    Serial.print("PWM: ");
    Serial.print("\t");
    Serial.print(PWM);
    Serial.println();

}