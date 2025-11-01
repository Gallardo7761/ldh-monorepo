#define PRO_MINI // PRO_MINI o NANO

#define MY_RADIO_RF24
#define MY_DEBUG

#ifdef NANO
  #define MY_RF24_CE_PIN 10
  #define MY_RF24_CS_PIN 9
#endif

#ifdef PRO_MINI
  #define MY_RF24_CE_PIN 9
  #define MY_RF24_CS_PIN 10
#endif

#define MY_RF24_CHANNEL 3

#ifdef PRO_MINI
    #define MY_NODE_ID 5
#endif

#ifdef NANO
    #define MY_NODE_ID 4
#endif

#include <MySensors.h>

#ifdef NANO 
    #include <Bounce2.h>
#endif

#ifdef PRO_MINI
    #include <Vcc.h>
    #include <DHT.h>
#endif

#ifdef NANO
    #define RLY_ID 1

    #define RLY_PIN 4
    #define SW1_PIN 5
    #define SW2_PIN 6

    MyMessage ch_comm_relay(RLY_ID, V_STATUS);
    Bounce sw1Debouncer = Bounce();
#endif

#ifdef PRO_MINI
  #define DHT_PIN 4
  DHT dht22(DHT_PIN, DHT22);

  #define DHT_ID_TEM 1
  #define DHT_ID_HUM 2
  MyMessage ch_comm_dht_temperature(DHT_ID_TEM, V_TEMP);
  MyMessage ch_comm_dht_humidity(DHT_ID_HUM, V_HUM);

  const float VccMin = 3.7;
  const float VccMax = 4.2;
  Vcc vcc;

  #define VCC_ID 3
  MyMessage ch_comm_vcc(VCC_ID, V_VOLTAGE);
#endif

void presentation()
{
  sendSketchInfo("HH-1", "1.0");
  #ifdef NANO
    present(RLY_ID, S_BINARY);
  #endif

  #ifdef PRO_MINI
    present(DHT_ID_TEM, S_TEMP);
    present(DHT_ID_HUM, S_HUM);
    present(VCC_ID, S_MULTIMETER);
  #endif
}

void setup()
{
  Serial.begin(115200);
  
  #ifdef NANO
    pinMode(RLY_PIN, OUTPUT);
    pinMode(SW1_PIN, INPUT_PULLUP); // siempre a HIGH a menos que se pulse
    pinMode(SW2_PIN, OUTPUT);       // siempre a LOW
    
    digitalWrite(SW2_PIN, LOW);

    sw1Debouncer.attach(SW1_PIN);
    sw1Debouncer.interval(25);

    send(ch_comm_relay.set(LOW));
  #endif

  #ifdef PRO_MINI
    dht22.begin();
    pinMode(DHT_PIN, INPUT);
  #endif

  wait(5000);
}

void loop()
{
  #ifdef NANO
    sw1Debouncer.update();
    static uint8_t lastState = 255; 
    uint8_t newState = !(sw1Debouncer.read() ^ digitalRead(SW2_PIN)); // SW1 XNOR SW2

    if (newState != lastState)
    {
        lastState = newState;
        digitalWrite(RLY_PIN, newState);
        send(ch_comm_relay.set(newState));
    }
  #endif

  #ifdef PRO_MINI
    float t = dht22.readTemperature();
    float h = dht22.readHumidity();

    if (!isnan(t) && !isnan(h))
    {
      send(ch_comm_dht_temperature.set(t, 2));
      send(ch_comm_dht_humidity.set(h, 2));
    }
  
    int voltage = Vcc::measure(100, 1100);
    int batteryPercent = static_cast<int>(100.0 * (voltage - VccMin) / (VccMax - VccMin));
    if (batteryPercent > 100) batteryPercent = 100;
    if (batteryPercent < 0)   batteryPercent = 0;

    sendBatteryLevel(batteryPercent);  // función de MySensor que envía el procentaje

    wait(2000);
  #endif  
}

#ifdef NANO
void receive(const MyMessage &message)
{
  int dest_node = message.getSensor();
  int msg_type = message.getType();
  
  if (dest_node != RLY_ID)
  {
    return;
  }

  if (msg_type != V_STATUS)
  {
    return;
  }

  int new_value = message.getInt();
  digitalWrite(RLY_PIN, new_value);
  send(ch_comm_relay.set(new_value));
}
#endif
