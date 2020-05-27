void HomePage()
{
 webSite += "<p id='HomeDisplay'></p>";

 webSite += " <br><br>";
 webSite += "<P>Settings:</P>";
 webSite += "<form action=\"/\">";
 webSite += " Max Duty Cycle:<br>";
 webSite += " <select name=\"MAXDC\">";
 webSite += "   <option value=\"100\">100</option>";
 webSite += "   <option value=\"90\">90</option>";
 webSite += "   <option value=\"80\">80</option>";
 webSite += "   <option value=\"70\">70</option>";
 webSite += "   <option value=\"60\">60</option>";
 webSite += " </select>";
 webSite += " <br><br>";
 webSite += " Min Duty Cycle:<br>";
 webSite += " <select name=\"MINDC\">";
 webSite += "   <option value=\"0\">0</option>";
 webSite += "   <option value=\"10\">10</option>";
 webSite += "   <option value=\"20\">20</option>";
 webSite += "   <option value=\"30\">30</option>";
 webSite += "   <option value=\"40\">40</option>";
 webSite += "   <option value=\"50\">50</option>";
 webSite += "   <option value=\"60\">60</option>";
 webSite += " </select>";
 webSite += " <br><br>";
 webSite += " <input type=\"submit\">";
 webSite += "</form>";
}

void handleSettings() 
{
  buildHomePage();
  
  if (server.hasArg("MAXDC"))
  {
    MaxDutyCycle = server.arg("MAXDC").toInt();
  }

  if (server.hasArg("MINDC"))
  {
    MinDutyCycle = server.arg("MINDC").toInt();
  }

  //save settings
  EEPROM.write(0, eepromKey);
  EEPROM.write(1, MinDutyCycle);
  EEPROM.write(2, MaxDutyCycle);
  EEPROM.commit();
    
  server.send(200, "text/html", webSite);
}

void handleLamboData() {
  XML = "<?xml version='1.0'?>";
  XML += "<lamboData>";

  XML += "  <BV>";
  XML +=      String(BatteryVoltage, 2);
  XML += "  </BV>";

  XML += "  <MinDC>";
  XML +=      String(MinDutyCycle);
  XML += "  </MinDC>";

  int temp = 0;
  temp = EEPROM.read(1);
  XML += "  <EepromMinDC>";
  XML +=      String(temp);
  XML += "  </EepromMinDC>";

  XML += "  <MaxDC>";
  XML +=      String(MaxDutyCycle);
  XML += "  </MaxDC>";

  temp = EEPROM.read(2);
  XML += "  <EepromMaxDC>";
  XML +=      String(temp);
  XML += "  </EepromMaxDC>";
  
  XML += "  <FWD>";
  XML +=      String(digitalRead(PEDAL_FWD_IN));
  XML += "  </FWD>";

  XML += "  <REV>";
  XML +=      String(digitalRead(PEDAL_REV_IN));
  XML += "  </REV>";
  
  XML += "  <GEAR>";
  if(Gear == GNEUTRAL)  XML +=     "NEUTRAL";
  if(Gear == GFORWARD)  XML +=     "FORWARD";
  if(Gear == GREVERSE)  XML +=     "REVERSE";
  XML += "  </GEAR>";

  XML += "</lamboData>";

  server.send(200, "text/xml", XML);
}
