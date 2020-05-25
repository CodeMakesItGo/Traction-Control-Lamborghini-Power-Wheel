void HomePage()
{
 webSite += "<p id='HomeDisplay'></p>";

 webSite += " <br><br>";
 webSite += "<P>Settings:</P>";
 webSite += "<form action=\"/\">";
 webSite += " Max Duty Cycle:<br>";
 webSite += " <select name=\"MAXDC\">";
 webSite += "   <option value=\"100\">100</option>";
 webSite += "   <option value=\"90\">80</option>";
 webSite += "   <option value=\"80\">60</option>";
 webSite += "   <option value=\"70\">40</option>";
 webSite += "   <option value=\"60\">20</option>";
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

  server.send(200, "text/html", webSite);
}

void handleLamboData() {
  XML = "<?xml version='1.0'?>";
  XML += "<lamboData>";

  XML += "  <BV>";
  XML +=      String(BatteryVoltage, 2);
  XML += "  </BV>";

  XML += "  <DC>";
  XML +=      String(DutyCycle);
  XML += "  </DC>";

  XML += "  <GEAR>";
  if(Gear == GNEUTRAL)  XML +=     "NEUTRAL";
  if(Gear == GFORWARD)  XML +=     "FORWARD";
  if(Gear == GREVERSE)  XML +=     "REVERSE";
  XML += "  </GEAR>";

  XML += "</lamboData>";

  server.send(200, "text/xml", XML);
}
