#include <Arduino_APDS9960.h>
#include <Arduino_BMI270_BMM150.h>
#include <PDM.h>


const int SOUND_THRESHOLD = 50;       
const int LIGHT_THRESHOLD = 250;      
const float MOTION_THRESHOLD = 0.05;  
const int PROX_THRESHOLD = 100;       


int micValue = 0;
int clearValue = 0;
int proxValue = 0;
float ax, ay, az;
float motionValue = 0;


short sampleBuffer[256];
volatile int samplesRead = 0;


int sound = 0;
int dark = 0;
int moving = 0;
int near = 0;


void onPDMdata() {
  int bytesAvailable = PDM.available();
  PDM.read(sampleBuffer, bytesAvailable);
  samplesRead = bytesAvailable / 2;
}

void setup() {
  Serial.begin(115200);
  delay(1500);

  if (!APDS.begin()) { Serial.println("Failed to init APDS9960"); while(1); }
  if (!IMU.begin()) { Serial.println("Failed to init IMU"); while(1); }

  PDM.onReceive(onPDMdata);           
  if (!PDM.begin(1, 16000)) { Serial.println("Failed to start PDM Microphone"); while(1); }
}

void loop() {
  if (samplesRead > 0) {  
    long sum = 0;
    for (int i = 0; i < samplesRead; i++) {
      sum += abs(sampleBuffer[i]);
    }
    micValue = sum / samplesRead;
    samplesRead = 0;
  }

  if (APDS.colorAvailable()) {
    int r, g, b;
    APDS.readColor(r, g, b, clearValue);
  }

  if (APDS.proximityAvailable()) {
    proxValue = APDS.readProximity();
  }

  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(ax, ay, az);
    motionValue = sqrt(ax*ax + ay*ay + az*az);  // magnitude
  }

  sound = (micValue > SOUND_THRESHOLD);
  dark = (clearValue < LIGHT_THRESHOLD);
  moving = (abs(motionValue - 1.0) > MOTION_THRESHOLD); // deviation from steady
  near = (proxValue < PROX_THRESHOLD); // small = near, large = far

  String label = "UNKNOWN";
  if (!sound && !dark && !moving && !near)
    label = "QUIET_BRIGHT_STEADY_FAR";
  else if (sound && !dark && !moving && !near)
    label = "NOISY_BRIGHT_STEADY_FAR";
  else if (!sound && dark && !moving && near)
    label = "QUIET_DARK_STEADY_NEAR";
  else if (sound && !dark && moving && near)
    label = "NOISY_BRIGHT_MOVING_NEAR";


  Serial.print("raw,mic=");
  Serial.print(micValue);
  Serial.print(", clear=");
  Serial.print(clearValue);
  Serial.print(", motion=");
  Serial.print(motionValue,2);
  Serial.print(", prox=");
  Serial.print(proxValue);

  Serial.print(" flags, sound=");
  Serial.print(sound);
  Serial.print(", dark=");
  Serial.print(dark);
  Serial.print(", moving=");
  Serial.print(moving);
  Serial.print(", near=");
  Serial.print(near);

  Serial.print(" state, ");
  Serial.println(label);

  delay(200);
}
