#define V_TEST A6
#define V_AMP A1
#define PWM_RES 12

int STEPS = pow(2,PWM_RES);
int MAX_STEPS = 0.250 * STEPS / 3.3; // 3.3 for Teensy, 5 for Arduino
float v_measured;
int sig = 0;


void setup() {
  Serial.begin(115200);
  
  pinMode(V_TEST, OUTPUT);
  pinMode(V_AMP, INPUT);
  
  analogWriteResolution(PWM_RES);
  analogReadResolution(PWM_RES);

  delay(1000);
}

void loop() {
  analogWrite(V_TEST, sig);
  v_measured = 5 * analogRead(V_TEST)/STEPS;
  Serial.print(sig); Serial.print(sig/MAX_STEPS * 3.3); Serial.print(","); Serial.println(v_measured);
  delay(500);
  ++sig;
  if (sig == MAX_STEPS)
    sig = 0;
}
