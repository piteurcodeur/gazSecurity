// ============================================================
// ATtiny85 - Interlock Sécurité Moteur avec Zone Morte (980 µs)
//
// BROCHES :
//   - PB1 (Pin 1 IDE / Broche phys. 6) : Sortie ESC
//   - PB2 (Pin 2 IDE / Broche phys. 7) : Entrée CH3 (Gaz)
//   - PB3 (Pin 3 IDE / Broche phys. 2) : Entrée CH6 (Sécurité)
// ============================================================

#include 

#define ESC_PIN 1 // PB1
#define CH3_PIN 2 // PB2
#define CH6_PIN 3 // PB3

#define MOTOR_OFF_US          980  // Signal forcé sous 1000 µs (moteur stoppé net)
#define THROTTLE_DEADBAND_US 1050  // Zone morte : sous 1050 µs, la sortie reste à 980 µs
#define THROTTLE_MIN_ARM_US  1080  // Seuil max des gaz pour valider l'armement à l'interrupteur
#define CH6_UNLOCK_THRESHOLD 1500  // Seuil de déverrouillage CH6 (> 1500 µs)

volatile uint16_t ch3_width = 1000;
volatile uint16_t ch6_width = 1000;
volatile uint32_t ch3_last_ms = 0;
volatile uint32_t ch6_last_ms = 0;

bool isArmed = false;
unsigned long armingStartTime;
unsigned long lastFrameTime = 0;

ISR(PCINT0_vect)
{
    static uint32_t ch3_start = 0;
    static uint32_t ch6_start = 0;
    static uint8_t last_pinb = 0;

    uint8_t pinb = PINB;
    uint8_t changed = pinb ^ last_pinb;
    last_pinb = pinb;

    uint32_t now = micros();
    uint32_t now_ms = millis();

    // Lecture CH3 (Gaz)
    if (changed & _BV(PB2))
    {
        if (pinb & _BV(PB2)) {
            ch3_start = now;
        } else {
            uint32_t width = now - ch3_start;
            if (width >= 800 && width <= 2200) {
                ch3_width = width;
                ch3_last_ms = now_ms;
            }
        }
    }

    // Lecture CH6 (Sécurité)
    if (changed & _BV(PB3))
    {
        if (pinb & _BV(PB3)) {
            ch6_start = now;
        } else {
            uint32_t width = now - ch6_start;
            if (width >= 800 && width <= 2200) {
                ch6_width = width;
                ch6_last_ms = now_ms;
            }
        }
    }
}

void setup()
{
    pinMode(ESC_PIN, OUTPUT);
    pinMode(CH3_PIN, INPUT);
    pinMode(CH6_PIN, INPUT);

    digitalWrite(ESC_PIN, LOW);

    GIMSK |= _BV(PCIE);
    PCMSK |= _BV(PCINT2) | _BV(PCINT3);

    sei();

    armingStartTime = millis();
}

void loop()
{
    // Trame PWM stricte de 20 ms (50 Hz)
    if (millis() - lastFrameTime >= 20)
    {
        lastFrameTime = millis();

        // Lecture atomique
        cli();
        uint16_t c3 = ch3_width;
        uint16_t c6 = ch6_width;
        uint32_t c3_age = millis() - ch3_last_ms;
        uint32_t c6_age = millis() - ch6_last_ms;
        sei();

        bool radioSignalValid = (c3_age < 150) && (c6_age < 150);
        bool switchUnlocked = radioSignalValid && (c6 >= CH6_UNLOCK_THRESHOLD);

        // 1. Logique d'armement après les 4s de boot
        if (millis() - armingStartTime > 4000)
        {
            if (!switchUnlocked)
            {
                isArmed = false; // Verrouillé si l'inter est coupé
            }
            else if (!isArmed && c3 <= THROTTLE_MIN_ARM_US)
            {
                isArmed = true; // S'arme uniquement si l'inter est ON ET manche en bas
            }
        }

        // 2. Traitement de la consigne vers l'ESC
        uint16_t outputPulse = MOTOR_OFF_US; // Par défaut : 980 µs

        if (isArmed && radioSignalValid)
        {
            // ZONE MORTE : Tant que le manche est sous 1050 µs, on force 980 µs
            if (c3 < THROTTLE_DEADBAND_US)
            {
                outputPulse = MOTOR_OFF_US;
            }
            else
            {
                outputPulse = c3; // Recopie normale si on monte le manche
            }
        }

        // 3. Émission de l'impulsion vers l'ESC
        digitalWrite(ESC_PIN, HIGH);
        delayMicroseconds(outputPulse);
        digitalWrite(ESC_PIN, LOW);
    }
}
