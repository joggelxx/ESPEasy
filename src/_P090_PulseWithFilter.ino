#ifdef USES_P090
//#######################################################################################################
//#################################### Plugin 090: Pulse with filter  ###############################################
//#######################################################################################################

#define PLUGIN_090
#define PLUGIN_ID_090         90
#define PLUGIN_NAME_090       "Generic - Pulse counter with Filter"
#define PLUGIN_VALUENAME1_090 "Count"
#define PLUGIN_VALUENAME2_090 "Error"
#define PLUGIN_VALUENAME3_090 "Time"


void Plugin_090_pulse_interrupt1() ICACHE_RAM_ATTR;
void Plugin_090_pulse_interrupt2() ICACHE_RAM_ATTR;
void Plugin_090_pulse_interrupt3() ICACHE_RAM_ATTR;
void Plugin_090_pulse_interrupt4() ICACHE_RAM_ATTR;
//this takes 20 bytes of IRAM per handler
// void Plugin_090_pulse_interrupt5() ICACHE_RAM_ATTR;
// void Plugin_090_pulse_interrupt6() ICACHE_RAM_ATTR;
// void Plugin_090_pulse_interrupt7() ICACHE_RAM_ATTR;
// void Plugin_090_pulse_interrupt8() ICACHE_RAM_ATTR;

unsigned long Plugin_090_pulseCounter[TASKS_MAX];
unsigned long Plugin_090_pulseFiltered[TASKS_MAX];
unsigned long Plugin_090_pulseTime[TASKS_MAX];
unsigned long Plugin_090_pulseTimePrevious[TASKS_MAX];
byte Plugin_090_pulseState[TASKS_MAX];

boolean Plugin_090(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_090;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 3;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_090);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_090));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_090));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_090));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
      	addFormNumericBox(F("Filter Time (mSec)"), F("plugin_090")
      			, Settings.TaskDevicePluginConfig[event->TaskIndex][0]);

        /*byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
        byte choice2 = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
        String options[4] = { F("Delta"), F("Delta/Total/Time"), F("Total"), F("Delta/Total") };
        addFormSelector(F("Counter Type"), F("plugin_090_countertype"), 4, options, NULL, choice );

        if (choice !=0)
          addHtml(F("<span style=\"color:red\">Total count is not persistent!</span>"));

        String modeRaise[4];
        modeRaise[0] = F("LOW");
        modeRaise[1] = F("CHANGE");
        modeRaise[2] = F("RISING");
        modeRaise[3] = F("FALLING");
        int modeValues[4];
        modeValues[0] = LOW;
        modeValues[1] = CHANGE;
        modeValues[2] = RISING;
        modeValues[3] = FALLING;

        addFormSelector(F("Mode Type"), F("plugin_090_raisetype"), 4, modeRaise, modeValues, choice2 );*/

        byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][1];

        String pulseStartName[2];
        pulseStartName[0] = F("RISING");
        pulseStartName[1] = F("FALLING");
        int pulseStartValue[2];
        pulseStartValue[0] = RISING;
        pulseStartValue[1] = FALLING;

        addFormSelector(F("Pulse starts on"), F("plugin_090_pulsestarttype"), 2, pulseStartName, pulseStartValue, choice );

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = getFormItemInt(F("plugin_090"));
        //Settings.TaskDevicePluginConfig[event->TaskIndex][1] = getFormItemInt(F("plugin_090_countertype"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = getFormItemInt(F("plugin_090_pulsestarttype"));
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SHOW_VALUES:
      {
        string += F("<div class=\"div_l\">");
        string += ExtraTaskSettings.TaskDeviceValueNames[0];
        string += F(":</div><div class=\"div_r\">");
        string += Plugin_090_pulseCounter[event->TaskIndex];
        string += F("</div><div class=\"div_br\"></div><div class=\"div_l\">");
        string += ExtraTaskSettings.TaskDeviceValueNames[1];
        string += F(":</div><div class=\"div_r\">");
        string += Plugin_090_pulseFiltered[event->TaskIndex];
        string += F("</div><div class=\"div_br\"></div><div class=\"div_l\">");
        string += ExtraTaskSettings.TaskDeviceValueNames[2];
        string += F(":</div><div class=\"div_r\">");
        string += Plugin_090_pulseTime[event->TaskIndex];
        string += F("</div>");
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        String log = F("INIT : Pulse ");
        log += Settings.TaskDevicePin1[event->TaskIndex];
        addLog(LOG_LEVEL_INFO,log);
        pinMode(Settings.TaskDevicePin1[event->TaskIndex], INPUT_PULLUP);
        Plugin_090_pulseState[event->TaskIndex] = false;
        //success = Plugin_090_pulseinit(Settings.TaskDevicePin1[event->TaskIndex], event->TaskIndex,Settings.TaskDevicePluginConfig[event->TaskIndex][2]);
        success = Plugin_090_pulseinit(Settings.TaskDevicePin1[event->TaskIndex], event->TaskIndex,CHANGE);
        break;
      }

    case PLUGIN_READ:
      {
        UserVar[event->BaseVarIndex] = Plugin_090_pulseCounter[event->TaskIndex];
        UserVar[event->BaseVarIndex+1] = Plugin_090_pulseFiltered[event->TaskIndex];
        UserVar[event->BaseVarIndex+2] = Plugin_090_pulseTime[event->TaskIndex];

        /*switch (Settings.TaskDevicePluginConfig[event->TaskIndex][1])
        {
          case 0:
          {
            event->sensorType = SENSOR_TYPE_SINGLE;
            UserVar[event->BaseVarIndex] = Plugin_090_pulseCounter[event->TaskIndex];
            break;
          }
          case 1:
          {
            event->sensorType = SENSOR_TYPE_TRIPLE;
            UserVar[event->BaseVarIndex] = Plugin_090_pulseCounter[event->TaskIndex];
            UserVar[event->BaseVarIndex+1] = Plugin_090_pulseFiltered[event->TaskIndex];
            UserVar[event->BaseVarIndex+2] = Plugin_090_pulseTime[event->TaskIndex];
            break;
          }
          case 2:
          {
            event->sensorType = SENSOR_TYPE_SINGLE;
            UserVar[event->BaseVarIndex] = Plugin_090_pulseFiltered[event->TaskIndex];
            break;
          }
          case 3:
          {
            event->sensorType = SENSOR_TYPE_DUAL;
            UserVar[event->BaseVarIndex] = Plugin_090_pulseCounter[event->TaskIndex];
            UserVar[event->BaseVarIndex+1] = Plugin_090_pulseFiltered[event->TaskIndex];
            break;
          }
        }*/
        event->sensorType = SENSOR_TYPE_TRIPLE;
        Plugin_090_pulseCounter[event->TaskIndex] = 0;
        Plugin_090_pulseFiltered[event->TaskIndex] = 0;
        success = true;
        break;
      }
  }
  return success;
}


/*********************************************************************************************\
 * Check Pulse Counters (called from irq handler)
\*********************************************************************************************/
void Plugin_090_pulsecheck(byte Index)
{

  /*bool pulseStart = bool(digitalRead(Settings.TaskDevicePin1[Index]))
    != bool(Settings.TaskDevicePluginConfig[Index][1] == FALLING);*/

  const unsigned long PulseTime=timePassedSince(Plugin_090_pulseTimePrevious[Index]);

  if (!Plugin_090_pulseState[Index)
    {
      //if(PulseTime > (unsigned long)Settings.TaskDevicePluginConfig[Index][0])
      //  {
          Plugin_090_pulseTimePrevious[Index]=millis();
      //  }
    }
  else
    {
      if(PulseTime > (unsigned long)Settings.TaskDevicePluginConfig[Index][0])
        {
          Plugin_090_pulseCounter[Index]++;
          Plugin_090_pulseTime[Index] = PulseTime;
        }
      else
        {
          Plugin_090_pulseFiltered[Index]++;
        }
    }
  Plugin_090_pulseState[Index] = !Plugin_090_pulseState[Index];
}


/*********************************************************************************************\
 * Pulse Counter IRQ handlers
\*********************************************************************************************/
void Plugin_090_pulse_interrupt1()
{
  Plugin_090_pulsecheck(0);
}
void Plugin_090_pulse_interrupt2()
{
  Plugin_090_pulsecheck(1);
}
void Plugin_090_pulse_interrupt3()
{
  Plugin_090_pulsecheck(2);
}
void Plugin_090_pulse_interrupt4()
{
  Plugin_090_pulsecheck(3);
}
void Plugin_090_pulse_interrupt5()
{
  Plugin_090_pulsecheck(4);
}
void Plugin_090_pulse_interrupt6()
{
  Plugin_090_pulsecheck(5);
}
void Plugin_090_pulse_interrupt7()
{
  Plugin_090_pulsecheck(6);
}
void Plugin_090_pulse_interrupt8()
{
  Plugin_090_pulsecheck(7);
}


/*********************************************************************************************\
 * Init Pulse Counters
\*********************************************************************************************/
bool Plugin_090_pulseinit(byte Par1, byte Index, byte Mode)
{

  switch (Index)
  {
    case 0:
      attachInterrupt(Par1, Plugin_090_pulse_interrupt1, Mode);
      break;
    case 1:
      attachInterrupt(Par1, Plugin_090_pulse_interrupt2, Mode);
      break;
    case 2:
      attachInterrupt(Par1, Plugin_090_pulse_interrupt3, Mode);
      break;
    case 3:
      attachInterrupt(Par1, Plugin_090_pulse_interrupt4, Mode);
      break;
    // case 4:
    //   attachInterrupt(Par1, Plugin_090_pulse_interrupt5, Mode);
    //   break;
    // case 5:
    //   attachInterrupt(Par1, Plugin_090_pulse_interrupt6, Mode);
    //   break;
    // case 6:
    //   attachInterrupt(Par1, Plugin_090_pulse_interrupt7, Mode);
    //   break;
    // case 7:
    //   attachInterrupt(Par1, Plugin_090_pulse_interrupt8, Mode);
    //   break;
    default:
      addLog(LOG_LEVEL_ERROR,F("PULSE: Error, only the first 4 tasks can be pulse counters."));
      return(false);
  }

  return(true);
}
#endif // USES_P090
