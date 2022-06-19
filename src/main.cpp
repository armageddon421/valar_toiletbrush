#include <Arduino.h>
#include <ESPUI.h>
#include "FastAccelStepper.h"
#include <HardwareSerial.h>
#include <TMCStepper.h>
#include <SPI.h>
#include <Preferences.h>
#include "Memory.h"
#include "ResetButton.h"
#include "MotorControl.h"
#include "API.h"
#include "ui.h"

//forward decl
void MotorTask(void *pvParameters);

void setup() {
  
  Serial.begin(115200);
  delay(1000);
  preferences.begin("local", false);
  load_preferences();
  setup_motors();
  API();

  move_to_percent = startSpeed;

  ESPUIsetup();
  
  // Now set up tasks to run independently.
  xTaskCreatePinnedToCore(
    MotorTask //Motor Task
    ,  "MotorTask"   // A name just for humans
    ,  1024*4  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  3  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL 
    ,  1);


}

void loop()
{
  // Empty. Things are done in Tasks.
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void MotorTask(void *pvParameters)  // Motor Task
{
  (void) pvParameters;

  move_motor();
  move_motor();
  move_motor();
  move_motor();

  if(startSpeed != 0)
  {
    move_to_percent = startSpeed;
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
  else{
    stepper->stopMove();
  }

  for (;;)
  {
    if(run_motor == true)
    {
      Serial.println("Run Motor Function");
      move_motor();
      run_motor = false;
      //ESPUI.updateLabel(positionLabel, String(int(((float)current_position/(float)max_steps)*100)));
      Serial.println("Motor Complete");
    }
    else if(set_zero == 1)
    {
      setZero();
      set_zero = 0;
    }
    else if(wifi_button == true)
    {
      button_change();
      wifi_button = false;
    }
    else
    {
      vTaskDelay(1);
    }
  }
}
