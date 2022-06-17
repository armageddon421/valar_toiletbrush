
#include <ESPUI.h>

uint16_t labelWifi;
uint16_t wifi_ssid_text, wifi_pass_text;
uint16_t speedMax;
uint16_t accelMax;
String display_wifi;

void numberSpeedCall(Control* sender, int type)
{
    max_speed = sender->value.toInt();
    preferences.putInt("max_speed", max_speed);
    
    Serial.print("max_speed: ");
    Serial.println(max_speed);
}

void numberAccelCall(Control* sender, int type)
{
    accel = sender->value.toInt();
    preferences.putInt("accel", accel);
    stepper->setAcceleration(accel);
    Serial.print("accel: ");
    Serial.println(accel);
}

void numberCurrentCall(Control* sender, int type)
{
    current = sender->value.toInt();
    preferences.putInt ("current", current);
    Serial.print("current: ");
    Serial.println(current);
}

void numberStallCall(Control* sender, int type)
{
    Serial.println(sender->value);
    stall = sender->value.toInt();
    driver.SGTHRS(stall);
    preferences.putInt ("stall", stall);
    Serial.print("stall: ");
    Serial.println(stall);
}


void sliderSpeed(Control* sender, int type)
{
    Serial.println(sender->value);
    move_to_percent = sender->value.toInt();
    if(move_to_percent < 0)
    {
        stepper->setSpeedInHz(move_to_percent * -max_speed);
        stepper->runBackward();
    }
    else if (move_to_percent > 0)
    {
        
        stepper->setSpeedInHz(move_to_percent * max_speed);
        stepper->runForward();
    }else
    {
        stepper->stopMove();
    }

    //stepper->moveTo(move_to_step);
    run_motor=true;
      
}

void buttonStopCall(Control* sender, int type)
{
    switch (type)
    {
    case B_DOWN:
        Serial.println("Button Pressed");
        set_zero = 1;
        stepper->stopMove();
        Serial.print("set_zero: ");
        Serial.println(set_zero);  
        break;
    }
}



void textNetworkCall(Control* sender, int type)
{
//    ssid = sender->value;
//    Serial.print(ssid);

}

void textPasswordCall(Control* sender, int type)
{
//    Serial.print(sender->value);
//    pass = sender->value;
//    Serial.print(pass);
}

void buttonSaveNetworkCall(Control* sender, int type)
{
if(type == B_UP) {
        Serial.println("Button Pressed");
        ssid = ESPUI.getControl(wifi_ssid_text)->value;
        pass = ESPUI.getControl(wifi_pass_text)->value;
        preferences.putString ("ssid", ssid);
        preferences.putString ("pass", pass);
        wifi_set = 1;
        display_wifi = "SET";
        ESPUI.updateLabel(labelWifi, display_wifi);
        preferences.putInt ("wifi_set", 1);
        connectWifi();
    }
}

void buttonClearNetworkCall(Control* sender, int type)
{
if(type == B_UP) {
        preferences.putInt ("wifi_set", 0);
        preferences.putString ("ssid", "NOT_SET");
        preferences.putString ("pass", "NOT_SET");
        preferences.end();
        ESP.restart();
    }
}

void ESPUIsetup(){

    uint16_t tab1 = ESPUI.addControl(ControlType::Tab, "Positioning", "Positioning");
    uint16_t tab2 = ESPUI.addControl(ControlType::Tab, "Settings", "Settings");
    uint16_t tab4 = ESPUI.addControl(ControlType::Tab, "WiFi", "WiFi");
    uint16_t tab5 = ESPUI.addControl(ControlType::Tab, "API", "API");

    // shown above all tabs. Not part of any tab
    //status = ESPUI.addControl(ControlType::Label, "Status:", "Stop", ControlColor::Turquoise);

//Tab1: Positioning
   //Slider: Move to position
   uint16_t positionMax = ESPUI.addControl(ControlType::Slider, "Speed %", "0", ControlColor::Alizarin, tab1, &sliderSpeed);
   ESPUI.addControl(ControlType::Min, "", "-100", ControlColor::None, positionMax);
   ESPUI.addControl(ControlType::Max, "", "100", ControlColor::None, positionMax);

//Tab2: Settings
   
   ESPUI.addControl(ControlType::Separator, "Motor Setting", "", ControlColor::Peterriver, tab2);
   //Number: Current
   speedMax = ESPUI.addControl(ControlType::Number, "Speed", String(max_speed), ControlColor::Alizarin, tab2, &numberSpeedCall);
   accelMax = ESPUI.addControl(ControlType::Number, "Acceleration", String(accel), ControlColor::Alizarin, tab2, &numberAccelCall);
   uint16_t currentMax = ESPUI.addControl(ControlType::Number, "Current(mA)", String(current), ControlColor::Alizarin, tab2, &numberCurrentCall);
   ESPUI.addControl(ControlType::Min, "", "400", ControlColor::None, currentMax);
   ESPUI.addControl(ControlType::Max, "", "2000", ControlColor::None, currentMax);
   //Number: Stall
   uint16_t stallMax = ESPUI.addControl(ControlType::Number, "Stall", String(stall), ControlColor::Carrot, tab2, &numberStallCall);
   ESPUI.addControl(ControlType::Min, "", "0", ControlColor::None, stallMax);
   ESPUI.addControl(ControlType::Max, "", "255", ControlColor::None, stallMax);


//Tab4: WiFi 
   ESPUI.addControl(ControlType::Separator, "Wifi Status", "", ControlColor::None, tab4);
   //Label: Is wifi set?
   
   
   if(wifi_set)
   {
   display_wifi = "SET";  
   }else
   {
   display_wifi = "ACCESS POINT";  
   }
   
   labelWifi = ESPUI.addControl(ControlType::Label, "Wifi Status", display_wifi, ControlColor::Emerald, tab4);
     //Button: Clear Network Settings
   ESPUI.addControl(ControlType::Button, "Clear Settings", "CLEAR", ControlColor::Emerald, tab4, &buttonClearNetworkCall);
   //Text: Name


   ESPUI.addControl(ControlType::Separator, "Set Wifi", "", ControlColor::None, tab4);
   wifi_ssid_text = ESPUI.addControl(ControlType::Text, "Network", ssid, ControlColor::Emerald, tab4, &textNetworkCall);
   
   //Text: Password
   wifi_pass_text = ESPUI.addControl(ControlType::Text, "Password", "******", ControlColor::Emerald, tab4, &textPasswordCall);

   //Button: Save
   ESPUI.addControl(ControlType::Button, "Save Settings", "SAVE", ControlColor::Emerald, tab4, &buttonSaveNetworkCall);
   
//Tab5: API 
   char apiPosition[50];
   snprintf(apiPosition, sizeof(apiPosition), "http://%s:8080/position?move_to_percent=%i", ip_address.c_str(), move_to_percent);
   ESPUI.addControl(ControlType::Label, "Move to Position", apiPosition, ControlColor::Turquoise, tab5);
   
   char apisettings[50];
   snprintf(apisettings, sizeof(apisettings), "http://%s:8080/settings", ip_address.c_str());
   ESPUI.addControl(ControlType::Label, "Check Settings", apisettings, ControlColor::Turquoise, tab5);

   ESPUI.sliderContinuous = true;
   ESPUI.begin("Valar Systems");
    
}
