#include <Arduino.h>

class PrintString : public Print, public String
{
public:
  size_t write(uint8_t data) override
  {
    return concat(char(data));
  };
};