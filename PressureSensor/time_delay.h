/**
 * @breif Thepurpose of this class is to provide a mechanism to execute any function
 * repeatedly in in geven intervals
 */
class CTimeDelay
{
private:
  static const int threshould = 20;
  bool isInDelay;
public:
  CTimeDelay()
  {
    isInDelay = false;
  }

  /**
   * @breief 
   * @param delayFunc is a fucntion pointer to the fucntion whch needs executed periodically
   * @delay the interfal in milli seconds
   */
  void TimeDelay(void (*DelayFunc)(), uint16_t delay)
  {
    long mills = millis();
    if(((mills%delay) < threshould ) && !isInDelay)
    {
      DelayFunc();
      isInDelay=true;
    }
    else
    {
      if(((mills%delay) > threshould) && isInDelay)
      {
        isInDelay=false;
      }
    }
  }
};
