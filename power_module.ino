#include <Arduino.h>
#include <Arduino_CAN.h>
#include "RTClib.h"
RTC_DS3231 rtc;
void printDateTime();
void pf_get (void);
void pf_io  (void);
void pf_put (void);
void pf_test (void);
 
#define PIN_BATTERY_RELAY         9
#define PIN_CHARGER_RELAY         10
#define PIN_MOT_DRV_RELAY         11
#define PIN_CONTROL_RELAY         12
#define PIN_AUX_DEV_RELAY         13

#define PIN_SW_REQ_UNLOCK         31
#define PIN_SERIAL_DR             33
#define PIN_LED_BUTTON1           35
#define PIN_LED_BUTTON2           37
#define PIN_SW_OFF                41
#define PIN_EMERGENCY_IN          45
#define PIN_EMERGENCY_OUT         47
#define PIN_UNLOCK_MOTOR          49
#define PIN_POWER_ON              51
 
#define READ_SW_POWER_OFF         digitalRead(PIN_SW_OFF)
#define READ_SW_UNLOCK            digitalRead(PIN_SW_REQ_UNLOCK)
#define READ_EMERGENCY_STOP_IN    digitalRead(PIN_EMERGENCY_IN)
 
#define MOTOR_UNLOCK_ON           digitalWrite(PIN_UNLOCK_MOTOR,HIGH)
#define MOTOR_UNLOCK_OFF          digitalWrite(PIN_UNLOCK_MOTOR,LOW )
#define EMERGENCY_STOP_ON         digitalWrite(PIN_EMERGENCY_OUT,LOW)
#define EMERGENCY_STOP_OFF        digitalWrite(PIN_EMERGENCY_OUT, HIGH )
#define LED_BUTTON1_ON            digitalWrite(PIN_LED_BUTTON2,   HIGH)
#define LED_BUTTON1_OFF           digitalWrite(PIN_LED_BUTTON2,   LOW )
#define LED_BUTTON2_ON            digitalWrite(PIN_LED_BUTTON1,   HIGH)
#define LED_BUTTON2_OFF           digitalWrite(PIN_LED_BUTTON1,   LOW )
#define POWER_SELF_ON             digitalWrite(PIN_POWER_ON,   HIGH)
#define POWER_SELF_OFF            digitalWrite(PIN_POWER_ON,   LOW )
#define RS485_DR_H                digitalWrite(PIN_SERIAL_DR,   HIGH)
#define RS485_DR_L                digitalWrite(PIN_SERIAL_DR,   LOW )
 
#define POWER_BATTERY_CONNECT     digitalWrite(PIN_BATTERY_RELAY,   HIGH)
#define POWER_BATTERY_DISCONNECT  digitalWrite(PIN_BATTERY_RELAY,   LOW )
#define POWER_CHARGER_CONNECT     digitalWrite(PIN_CHARGER_RELAY,   HIGH)
#define POWER_CHARGER_DISCONNECT  digitalWrite(PIN_CHARGER_RELAY,   LOW )
#define POWER_MOT_DRV_CONNECT     digitalWrite(PIN_MOT_DRV_RELAY,   HIGH)
#define POWER_MOT_DRV_DISCONNECT  digitalWrite(PIN_MOT_DRV_RELAY,   LOW )
#define POWER_CONTROL_CONNECT     digitalWrite(PIN_CONTROL_RELAY,   HIGH)
#define POWER_CONTROL_DISCONNECT  digitalWrite(PIN_CONTROL_RELAY,   LOW )
#define POWER_AUX_DEV_CONNECT     digitalWrite(PIN_AUX_DEV_RELAY,   HIGH)
#define POWER_AUX_DEV_DISCONNECT  digitalWrite(PIN_AUX_DEV_RELAY,   LOW )
 


#define MES_I_AUX 0
#define MES_V_AUX 1
#define MES_I_CON 2
#define MES_V_CON 3
#define MES_I_MOT 4
#define MES_V_MOT 5
#define MES_I_CHR 6
#define MES_V_CHR 7
#define MES_I_BAT 8
#define MES_V_BUS 9
#define MES_V_BAT 10
#define MES_V_GNDC 11
#define MES_V_GNDD 12


const float ADC_GAIN[16] = {0.02260, 0.01605,0.02260, 0.01605, 0.02260, 0.01605,0.02260, 0.01605,0.02260, 0.01605,0.01605,0.01605,0.01605};
const int ADC_OFFSET[16] = {292,0,292,0,1460,0,1460,0,1460,0,0,0,0};
const char ADC_UNIT[16] = {'I','V','I','V','I','V','I','V','I','V','V','V','V'};
const char ADC_CH_NAME[16][20] = {
"MES_I_AUX ",
"MES_V_AUX ",
"MES_I_CON ",
"MES_V_CON ",
"MES_I_MOT ",
"MES_V_MOT ",
"MES_I_CHR ",
"MES_V_CHR ",
"MES_I_BAT ",
"MES_V_BUS ",
"MES_V_BAT ",
"MES_V_GNDC",
"MES_V_GNDD"
};



/* Relay contrl status */
int power_sw_battery , power_sw_charger , power_sw_mot_drv   , power_sw_control , power_sw_aux_drv;
 
int power_on_relay, unlock_motor     , emergency_stop_out , led_button1 , led_button2 , sw_req_unlock;

int DO_motor_unlock, DO_emergency_stop, DO_led_button1, DO_led_button2, DO_power_self, DO_rs485_dr;
 



// ใช้ชื่อ Port ตรงๆ แทนตัวเลข
const int PIN_LED_RED   = 86;  // PI_12
const int PIN_LED_GREEN = 87;  // PJ_13
const int PIN_LED_BLUE  = 88;  // PE_3
 
int analogValue[16];
float phisicalValue[16];
unsigned long lastLogTime;
unsigned int cnt_for1sec;
unsigned long cnt_sec;
const unsigned long logInterval = 20;
 
int system_state;
 
void setup() {
  lastLogTime = 0;
  cnt_for1sec = 0;
  cnt_sec = 0;
  analogReadResolution(12);
  pinMode(PIN_LED_RED,   OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_BLUE,  OUTPUT);



  power_sw_battery = 0;
  power_sw_charger = 0;
  power_sw_mot_drv = 0;
  power_sw_control = 0;
  power_sw_aux_drv = 0;

  power_on_relay = 0;

  pinMode(PIN_UNLOCK_MOTOR, OUTPUT);  MOTOR_UNLOCK_OFF;     DO_motor_unlock   =0 ;
  pinMode(PIN_EMERGENCY_OUT, OUTPUT); EMERGENCY_STOP_ON;    DO_emergency_stop =1 ;
  pinMode(PIN_LED_BUTTON2, OUTPUT);   LED_BUTTON2_OFF;      DO_led_button2    =0 ;
  pinMode(PIN_LED_BUTTON1, OUTPUT);   LED_BUTTON1_OFF;      DO_led_button1    =0 ;
  pinMode(PIN_POWER_ON, OUTPUT);      POWER_SELF_OFF;       DO_power_self     =0 ;
  //pinMode(PIN_SERIAL_DR, OUTPUT);     RS485_DR_L;






  pinMode(PIN_BATTERY_RELAY, OUTPUT); POWER_BATTERY_DISCONNECT;
  pinMode(PIN_CHARGER_RELAY, OUTPUT); POWER_CHARGER_DISCONNECT;
  pinMode(PIN_MOT_DRV_RELAY, OUTPUT); POWER_MOT_DRV_DISCONNECT;
  pinMode(PIN_CONTROL_RELAY, OUTPUT); POWER_CONTROL_DISCONNECT;
  pinMode(PIN_AUX_DEV_RELAY, OUTPUT); POWER_AUX_DEV_DISCONNECT;
 
 
  pinMode(PIN_SERIAL_DR    , OUTPUT); RS485_DR_H ;
  pinMode(PIN_LED_BUTTON1  , OUTPUT); LED_BUTTON1_OFF;
  pinMode(PIN_LED_BUTTON2  , OUTPUT); LED_BUTTON2_OFF;
  pinMode(PIN_EMERGENCY_OUT, OUTPUT); EMERGENCY_STOP_ON;
  pinMode(PIN_POWER_ON     , OUTPUT); POWER_SELF_OFF;
 
  
  pinMode(PIN_SW_REQ_UNLOCK ,  INPUT_PULLUP);
  pinMode(PIN_SW_OFF         , INPUT_PULLUP);
  pinMode(PIN_EMERGENCY_IN   , INPUT_PULLUP);
 
  digitalWrite(PIN_LED_RED,   HIGH); // HIGH = ดับ
  digitalWrite(PIN_LED_GREEN, HIGH);
  digitalWrite(PIN_LED_BLUE,  HIGH);
  Serial.begin(115200);
  Serial1.begin(115200);  // ตัวที่ 2: RX=D0,  TX=D1
  Serial2.begin(115200);  // ตัวที่ 3: RX=D19, TX=D18
  Serial3.begin(115200);  // ตัวที่ 4: RX=D17, TX=D16
  Serial4.begin(115200);  // ตัวที่ 5: RX=D15, TX=D14




  sw_req_unlock = READ_SW_UNLOCK;

  system_state = 0;
  digitalWrite(PIN_LED_BLUE,  LOW);
 
    
 
   // ===== RTC DS3231 =====
  if (!rtc.begin()) {
    Serial.println("ERROR: DS3231 not found!");
  } else {
    // ตั้งเวลาเริ่มต้น: 25 DEC 2026 17:30:00
    //rtc.adjust(DateTime(2026, 06, 23, 11, 10, 0));
    Serial.println("RTC initialized");
    printDateTime();          // อ่านครั้งแรกตอนเปิดเครื่อง
    //lastRtcRead = millis();
    
  }
 
  // ===== CAN bus =====
  // if (!CAN.begin(CanBitRate::BR_500k)) {
  //   Serial.println("ERROR: CAN.begin() failed!");
  // } else {
  //   Serial.println("CAN initialized @ 500kbps");
  // }
  delay(5000);
  POWER_SELF_ON;
  power_on_relay = 1 ; 
  digitalWrite(PIN_LED_BLUE,  HIGH);
  digitalWrite(PIN_LED_GREEN,  LOW);
}

 
void loop() {
  
  if (millis() - lastLogTime >= logInterval) {
    lastLogTime = millis();
    pf_get ();
    if (++cnt_for1sec >= 50) {
      cnt_for1sec = 0;
      cnt_sec++;
      //Serial.print("\nTest serial console");
      // เพิ่มตรงนี้
      //Serial.print("I_AUX:"); Serial.print(phisicalValue[MES_I_AUX], 3); Serial.print(",");
      //Serial.print("V_AUX:"); Serial.print(phisicalValue[MES_V_AUX], 3); Serial.print(",");
      //Serial.print("I_MOT:"); Serial.print(phisicalValue[MES_I_MOT], 3); Serial.print(",");
      //Serial.print("V_MOT:"); Serial.print(phisicalValue[MES_V_MOT], 3);
      //Serial.println();

      Serial1.print("\nTest serial IPC");
      Serial2.print("\nTest serial motor driver");
      Serial3.print("\nTest serial battery & charger");
      Serial4.print("\nTest serial display");
    }
  
    
 
    
  }
  pf_io  ();
  pf_test();
  pf_put ();
 
 
}
 
void pf_get (){
  int i,raw;
  float voltage ;
 
  analogValue[MES_I_AUX] = analogRead(A0);
  analogValue[MES_V_AUX] = analogRead(A1);
  analogValue[MES_I_CON] = analogRead(A2);
  analogValue[MES_V_CON] = analogRead(A3);
  analogValue[MES_I_MOT] = analogRead(A4);
  analogValue[MES_V_MOT] = analogRead(A5);
  analogValue[MES_I_CHR] = analogRead(A6);
  analogValue[MES_V_CHR] = analogRead(A7);
  analogValue[MES_I_BAT] = analogRead(A8);
  analogValue[MES_V_BUS] = analogRead(A9);
  analogValue[MES_V_BAT] = analogRead(A10);
  analogValue[MES_V_GNDC] = analogRead(A11);
  analogValue[MES_V_GNDD] = analogRead(A12);
  //analogValue[13] = analogRead(A13);
  



  for (i = 0; i < 14; i++) {
    phisicalValue [i] = (analogValue[i]  - ADC_OFFSET[i]) * ADC_GAIN[i];   // 12-bit, ref 3.3V
  }
    
    //Serial.print("A");
    //Serial.print(i);
    //Serial.print(": ");
    //Serial.print(voltage, 3);
    //Serial.println(" V");
  
}


void pf_io() {
  int t_sw_req_unlock;
  t_sw_req_unlock = READ_SW_UNLOCK;
  if (t_sw_req_unlock != sw_req_unlock){
    sw_req_unlock = t_sw_req_unlock;
    if (sw_req_unlock == 0) {
      MOTOR_UNLOCK_ON;
      DO_motor_unlock = 1;
    }
    else {
      MOTOR_UNLOCK_OFF;
      DO_motor_unlock = 0;
    }
  }
}

void pf_test (){
  int i ;
  //if(cnt_sec==5 ) POWER_BATTERY_CONNECT;
  //if(cnt_sec==10) POWER_BATTERY_DISCONNECT;
  //if(cnt_sec==30) POWER_SELF_OFF;

  if (Serial.available() > 0) {
    char c = Serial.read();
    Serial.print("Received via serial console : ");
    Serial.println(c);
    if(c=='1'){
      if(power_sw_battery==0){
        Serial.print("\n battery connect ");
        power_sw_battery = 1;
        POWER_BATTERY_CONNECT;
      }else{
        Serial.print("\n battery disconnect ");
        power_sw_battery = 0;
        POWER_BATTERY_DISCONNECT;
      }
    }
    if(c=='2'){
      if(power_sw_charger==0){
        Serial.print("\n charger connect ");
        power_sw_charger = 1;
        POWER_CHARGER_CONNECT;
      }else{
        Serial.print("\n charger disconnect ");
        power_sw_charger = 0;
        POWER_CHARGER_DISCONNECT;
      }
    }
    if(c=='3'){
      if(power_sw_mot_drv==0){
        Serial.print("\n motor driver connect ");
        power_sw_mot_drv = 1;
        POWER_MOT_DRV_CONNECT;
      }else{
        Serial.print("\n motor driver disconnect ");
        power_sw_mot_drv = 0;
        POWER_MOT_DRV_DISCONNECT;
      }
    }
    if(c=='4'){
      if(power_sw_control==0){
        Serial.print("\n controller connect ");
        power_sw_control = 1;
        POWER_CONTROL_CONNECT;
      }else{
        Serial.print("\n controller disconnect ");
        power_sw_control = 0;
        POWER_CONTROL_DISCONNECT;
      }
    }
    if(c=='5'){
      if(power_sw_aux_drv==0){
        Serial.print("\n AUX device connect ");
        power_sw_aux_drv = 1;
        POWER_AUX_DEV_CONNECT;
      }else{
        Serial.print("\n AUX device disconnect ");
        power_sw_aux_drv = 0;
        POWER_AUX_DEV_DISCONNECT;
      }
    }
  if(c=='p'){
      if(power_on_relay==0){
        Serial.print("\n self power on ");
        power_on_relay = 1;
        POWER_SELF_ON;
      }else{
        Serial.print("\n self power off ");
        power_on_relay = 0;
        POWER_SELF_OFF;
      }
    }



    if(c=='6'){
      if(DO_motor_unlock==0){
        Serial.print("\n Motor Unlock ");
        DO_motor_unlock = 1;
        MOTOR_UNLOCK_ON;
      }else{
        Serial.print("\n Motor lock ");
        DO_motor_unlock = 0;
        MOTOR_UNLOCK_OFF;
      }
    }
    if(c=='7'){
      if(DO_emergency_stop==0){
        Serial.print("\n Emergency stop active ");
        DO_emergency_stop = 1;
        EMERGENCY_STOP_ON;
      }else{
        Serial.print("\n Emergency stop inactive ");
        DO_emergency_stop = 0;
        EMERGENCY_STOP_OFF;
      }
    }
    if(c=='8'){
      if(DO_led_button1==0){
        Serial.print("\n LED Power button ON ");
        DO_led_button1 = 1;
        LED_BUTTON1_ON;
      }else{
        Serial.print("\n LED Power button OFF ");
        DO_led_button1 = 0;
        LED_BUTTON1_OFF;
      }
    }
    if(c=='9'){
      if(DO_led_button2==0){
        Serial.print("\n LED Master button ON ");
        DO_led_button2 = 1;
        LED_BUTTON2_ON;
      }else{
        Serial.print("\n LED Master button OFF ");
        DO_led_button2 = 0;
        LED_BUTTON2_OFF;
      }
    }
    if(c=='0'){
      if(power_sw_aux_drv==0){
        Serial.print("\n AUX device connect ");
        power_sw_aux_drv = 1;
        POWER_AUX_DEV_CONNECT;
      }else{
        Serial.print("\n AUX device disconnect ");
        power_sw_aux_drv = 0;
        POWER_AUX_DEV_DISCONNECT;
      }
    }
    if (c == 'i') {
      Serial.print("\n battery status      : "); Serial.print(power_sw_battery);
      Serial.print("\n charger status      : "); Serial.print(power_sw_charger);
      Serial.print("\n motor driver status : "); Serial.print(power_sw_mot_drv);
      Serial.print("\n controller status   : "); Serial.print(power_sw_control);
      Serial.print("\n AUX device status   : "); Serial.print(power_sw_aux_drv);
      Serial.print("\n ");
      for (i=0; i<13; i++) {
          Serial.print("\n");
          Serial.print(ADC_CH_NAME[i]);
          Serial.print(": ");
          Serial.print(phisicalValue[i], 3);
          Serial.print(" ");
          Serial.print(ADC_UNIT[i]);
      }
      Serial.print("\n READ_SW_POWER_OFF_STATUS           : "); Serial.print(READ_SW_POWER_OFF);
      Serial.print("\n READ_SW_UNLOCK_STATUS              : "); Serial.print(READ_SW_UNLOCK);
      Serial.print("\n READ_EMERGENCY_STOP_IN_STATUS      : "); Serial.print(READ_EMERGENCY_STOP_IN);
      Serial.print("\n ");
      printDateTime();




    }
 
  }
  if (Serial1.available() > 0) {
    char c = Serial1.read();
    Serial.print("Received via serial IPC : ");
    Serial.println(c);
  }
  if (Serial2.available() > 0) {
    char c = Serial2.read();
    //Serial.print("Received via serial motor driver : ");
    //Serial.println(c);
  }
  if (Serial3.available() > 0) {
    char c = Serial3.read();
    Serial.print("Received via serial battery & charger : ");
    Serial.println(c);
  }
  if (Serial4.available() > 0) {
    char c = Serial4.read();
    Serial.print("Received via serial display : ");
    Serial.println(c);
  }
#if 0
  if(cnt_for1sec<16){
    digitalWrite(PIN_LED_RED,   LOW );
    digitalWrite(PIN_LED_GREEN, HIGH);
    digitalWrite(PIN_LED_BLUE,  HIGH);
  }else if(cnt_for1sec<32){
    digitalWrite(PIN_LED_RED,   HIGH);
    digitalWrite(PIN_LED_GREEN, LOW );
    digitalWrite(PIN_LED_BLUE,  HIGH);    
  }else{
    digitalWrite(PIN_LED_RED,   HIGH);
    digitalWrite(PIN_LED_GREEN, HIGH);
    digitalWrite(PIN_LED_BLUE,  LOW );
  }
  #endif
}
void pf_put (){
 
}
void printDateTime() {
  DateTime now = rtc.now();
  char buf[32];
  sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
          now.year(), now.month(), now.day(),
          now.hour(), now.minute(), now.second());
  Serial.print("RTC: ");
  Serial.println(buf);
}
 
