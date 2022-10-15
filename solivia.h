// *****************************************************************
// *          ESPHome Custom Component Modbus sniffer for          *
// *              Delta Solvia Inverter 3.0 EU G4 TR               *
// *             installed with Solivia Gateway M1 D2.             *
// *****************************************************************

#include "esphome.h"

class solivia : public PollingComponent, public Sensor, public UARTDevice {
  public:
    solivia(UARTComponent *parent) : PollingComponent(400), UARTDevice(parent) {}
    Sensor *d_yield = new Sensor();
    Sensor *t_yield = new Sensor();
    Sensor *ac_power = new Sensor();
    Sensor *dc_power = new Sensor();
    Sensor *ac_v = new Sensor();
    Sensor *dc_v = new Sensor();
    Sensor *ac_a = new Sensor();
    Sensor *dc_a = new Sensor();
    Sensor *freq = new Sensor();
    Sensor *temp_amb = new Sensor();
    Sensor *temp_hs = new Sensor();
    Sensor *iso_plus = new Sensor();
    Sensor *iso_minus = new Sensor();
    Sensor *ac_react = new Sensor();
    Sensor *status_1 = new Sensor();
    Sensor *status_2 = new Sensor();
  
  void setup() override {

  }

  std::vector<int> bytes;
  int count = 15;

  //void loop() override {

  void update() {
    while(available() > 0) {
      bytes.push_back(read());      
      //make sure at least 8 header bytes are available for check
      if(bytes.size() < 8)       
      {
        continue;  
      }
      //ESP_LOGD("custom", "Checking for inverter package");
      // Check for Delta Solivia Gateway package response.
      if(bytes[0] != 0x02 || bytes[1] != 0x06 || bytes[2] != 0x01 || bytes[3] != 0xFF || bytes[4] != 0x60 || bytes[5] != 0x01) {
        bytes.erase(bytes.begin()); //remove first byte from buffer
        //buffer will never get above 8 until the response is a match
        continue;
      }      
      
	    if (bytes.size() == 262) {
        
        // Some 15 packages are recieved appx. every 10 seconds
        // With this counter we will only update ESPHome sensor appx. every 10 seconds
        // NB. Remove this counter if you're not using a Solivia Gateway. 
        count--;
        if (count != 0) {
          ESP_LOGI("custom", "Update ESPHome sensors in: %i seconds", count);
          bytes.clear();
          continue;
        }

        count = 15;

        TwoByte dc_power_data;
        dc_power_data.Byte[0] = bytes[0x4B +6]; // DC Power lsb
        dc_power_data.Byte[1] = bytes[0x4A +6]; // DC Power msb
        TwoByte dc_v_data;
        dc_v_data.Byte[0] = bytes[0x4D +6]; // DC voltage lsb
        dc_v_data.Byte[1] = bytes[0x4C +6]; // DC voltage msb
        TwoByte dc_a_data;
        dc_a_data.Byte[0] = bytes[0x4F +6]; // DC current lsb
        dc_a_data.Byte[1] = bytes[0x4E +6]; // DC current msb

        TwoByte ac_a_data;
        ac_a_data.Byte[0] = bytes[0x5D + 6]; // AC current lsb
        ac_a_data.Byte[1] = bytes[0x5C + 6]; // AC current msb
        TwoByte ac_v_data;
        ac_v_data.Byte[0] = bytes[0x5F + 6]; // AC voltage lsb
        ac_v_data.Byte[1] = bytes[0x5E + 6]; // AC voltage lsb
        TwoByte freq_data;
        freq_data.Byte[0] = bytes[0x61 + 6]; // Frequency lsb
        freq_data.Byte[1] = bytes[0x60 + 6]; // Frequency msb
        TwoByte ac_power_data;
        ac_power_data.Byte[0] = bytes[0x63 +6]; // AC Power lsb
        ac_power_data.Byte[1] = bytes[0x62 +6]; // AC Power msb
        TwoByte ac_react_data;
        ac_react_data.Byte[0] = bytes[0x65 +6]; // AC reactive power lsb
        ac_react_data.Byte[1] = bytes[0x64 +6]; // AC reactive power msb

        TwoByte iso_plus_data;
        iso_plus_data.Byte[0] = bytes[0x7B +6]; // ISO+ lsb
        iso_plus_data.Byte[1] = bytes[0x7A +6]; // ISO+ msb
        TwoByte iso_minus_data;
        iso_minus_data.Byte[0] = bytes[0x7D +6]; // ISO- lsb
        iso_minus_data.Byte[1] = bytes[0x7C +6]; // ISO- msb

        TwoByte temp_amb_data;
        temp_amb_data.Byte[0] = bytes[0x7F +6]; // Temperature ambient lsb
        temp_amb_data.Byte[1] = bytes[0x7E +6]; // Temperature ambient msb
        TwoByte temp_hs_data;
        temp_hs_data.Byte[0] = bytes[0x81 +6]; // Temperature heatsink lsb
        temp_hs_data.Byte[1] = bytes[0x80 +6]; // Temperature heatsink msb

        TwoByte d_yield_data;
        d_yield_data.Byte[0] = bytes[0xB5 +6]; // Daily yield lsb
        d_yield_data.Byte[1] = bytes[0xB4 +6]; // Daily yield msb
        uint32_t t_yield_data = int(
            (unsigned char)(bytes[0x86 +6]) << 24 |
            (unsigned char)(bytes[0x87 +6]) << 16 |
            (unsigned char)(bytes[0x88 +6]) << 8 |
            (unsigned char)(bytes[0x89 +6]));  // Total yield (4 bytes float)

        TwoByte status_1_data;
        status_1_data.Byte[0] = bytes[0x91 +6]; // Status 1 (bit 0-7)
        status_1_data.Byte[1] = 0;
        TwoByte status_2_data;
        status_2_data.Byte[0] = 0;
        status_2_data.Byte[1] = bytes[0x94 +6]; // Status 2 (bit 8-15)

        char etx;
        etx = bytes[261]; // ETX byte (last byte)

        // Quick and dirty check for package integrity is done, in order to avoid irratic sensor value updates 
        // This effectively blocks out any erroneous sensor updates due to rx package corruption
        // Check if ETX = 3. If not (invalid package), ditch whole package, clear buffer and continue
        if (etx != 0x03) {
          ESP_LOGI("custom", "ETX check failure - NO sensor update! ETX: %i", etx);
          bytes.clear();
          continue;
        }

          d_yield->publish_state(d_yield_data.UInt16);
          t_yield->publish_state(t_yield_data);
          dc_v->publish_state(dc_v_data.UInt16);
          dc_a->publish_state(dc_a_data.UInt16);
          dc_power->publish_state(dc_power_data.UInt16);
          ac_v->publish_state(ac_v_data.UInt16);
          ac_a->publish_state(ac_a_data.UInt16);
          ac_power->publish_state(ac_power_data.UInt16);
          ac_react->publish_state(ac_react_data.Int16);
          freq->publish_state(freq_data.UInt16);
          temp_amb->publish_state(temp_amb_data.Int16);
          temp_hs->publish_state(temp_hs_data.Int16);
          iso_plus->publish_state(iso_plus_data.UInt16);
          iso_minus->publish_state(iso_minus_data.UInt16);
          status_1->publish_state(status_1_data.UInt16);
          status_2->publish_state(status_2_data.UInt16);

	        ESP_LOGI("custom", "ETX check OK: %i - ESPHome sensors updated", etx);
          ESP_LOGI("custom", "Current production: %i W / Daily yield: %i Wh", ac_power_data.UInt16, d_yield_data.UInt16);
        
          bytes.clear();
      }
      else {
      }    
    }    
  }

  typedef union
  {
    unsigned char Byte[2];
    int16_t Int16;
    uint16_t UInt16;
    unsigned char UChar;
    char Char;
  }TwoByte;};
