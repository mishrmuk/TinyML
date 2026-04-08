#include <Arduino_HS300x.h>
#include <Arduino_APDS9960.h>
#include <Arduino_BMI270_BMM150.h>


const float HUMIDITY_JUMP = 5.0;       
const float TEMP_RISE = 2.0;          
const float MAG_THRESHOLD = 50.0;      
const int LIGHT_CHANGE_THRESHOLD = 50; 

float temperature = 0;
float humidity = 0;
float magX, magY, magZ;
float magValue = 0;
int r, g, b, clearValue = 0;


float prevHumidity = 0;
float prevTemp = 0;
float prevClear = 0;
float prevMagValue = 0;


int humid_jump = 0;
int temp_rise = 0;
int mag_shift = 0;
int light_or_color_change = 0;


unsigned long lastEventTime = 0;
const unsigned long EVENT_COOLDOWN = 1000; 

void setup() {
  Serial.begin(115200);
  delay(1500);

  if (!HS300x.begin()) {
    Serial.println("Failed to initialize humidity/temperature sensor.");
    while (1);
  }

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU/magnetometer.");
    while (1);
  }

  if (!APDS.begin()) {
    Serial.println("Failed to initialize APDS9960 sensor.");
    while (1);
  }

  Serial.println("Environmental event detection started");
}

void loop() {
  unsigned long now = millis();
  

  humidity = HS300x.readHumidity();
  temperature = HS300x.readTemperature();

  if (IMU.magneticFieldAvailable()) {
    IMU.readMagneticField(magX, magY, magZ);
    magValue = sqrt(magX*magX + magY*magY + magZ*magZ);
  }

  if (APDS.colorAvailable()) {
    APDS.readColor(r, g, b, clearValue);
  }


  humid_jump = (abs(humidity - prevHumidity) > HUMIDITY_JUMP);
  temp_rise = (temperature - prevTemp > TEMP_RISE);
  mag_shift = (abs(magValue - prevMagValue) > MAG_THRESHOLD);
  light_or_color_change = (abs(clearValue - prevClear) > LIGHT_CHANGE_THRESHOLD);


  String label = "BASELINE_NORMAL";
  if (humid_jump || temp_rise) label = "BREATH_OR_WARM_AIR_EVENT";
  if (mag_shift) label = "MAGNETIC_DISTURBANCE_EVENT";
  if (light_or_color_change) label = "LIGHT_OR_COLOR_CHANGE_EVENT";


  Serial.print("raw,rh=");
  Serial.print(humidity, 2);
  Serial.print(",temp=");
  Serial.print(temperature, 2);
  Serial.print(",mag=");
  Serial.print(magValue, 2);
  Serial.print(",r=");
  Serial.print(r);
  Serial.print(",g=");
  Serial.print(g);
  Serial.print(",b=");
  Serial.print(b);
  Serial.print(",clear=");
  Serial.print(clearValue);

  Serial.print(" flags,humid_jump=");
  Serial.print(humid_jump);
  Serial.print(",temp_rise=");
  Serial.print(temp_rise);
  Serial.print(",mag_shift=");
  Serial.print(mag_shift);
  Serial.print(",light_or_color_change=");
  Serial.print(light_or_color_change);

  Serial.print(" event,");
  Serial.println(label);


  if (now - lastEventTime > EVENT_COOLDOWN) {
    prevHumidity = humidity;
    prevTemp = temperature;
    prevMagValue = magValue;
    prevClear = clearValue;
    lastEventTime = now;
  }

  delay(200);
}