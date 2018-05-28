void HomePage()
{
 webSite += "<p id='HomeDisplay'></p>";

 
 webSite += " <br><br>";
 webSite += "<P>Settings:</P>";
 webSite += "<form action=\"/\">";
 webSite += " Top Speed:<br>";
 webSite += " <select name=\"SPD\">";
 webSite += "   <option value=\"3\">3 Mph</option>";
 webSite += "   <option value=\"6\">6 Mph </option>";
 webSite += "   <option value=\"9\">9 Mph</option>";
 webSite += "   <option value=\"12\">12 Mph</option>";
 webSite += " </select>";
 webSite += " <br><br>";
 webSite += " Traction Control:<br>";
 webSite += " <select name=\"TCS\">";
 webSite += "   <option value=\"2\">Comfort</option>";
 webSite += "   <option value=\"1\">Sport</option>";
 webSite += "   <option value=\"0\">Off</option>";
 webSite += " </select>";
 webSite += " <br><br>";
 webSite += " Max Duty Cycle:<br>";
 webSite += " <select name=\"MAXDC\">";
 webSite += "   <option value=\"100\">100</option>";
 webSite += "   <option value=\"80\">80</option>";
 webSite += "   <option value=\"60\">60</option>";
 webSite += "   <option value=\"40\">40</option>";
 webSite += "   <option value=\"20\">20</option>";
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

void GaugePage()
{
   
 webSite += "<div class='floating-littlebox'>MPH</div>";

 webSite += "<div class='floating-littlebox'>RPM</div>";

 webSite += "<div class='floating-littlebox'>Power %</div>";

 webSite += "<div class='after-box'></div>";

 webSite += "<div class='floating-box'>";
   webSite += "<canvas id='MphGauge' width='200' height='200'";
   webSite += "style='background-color:#fff'>";
   webSite += "</canvas>";
 webSite += "</div>";

 webSite += "<div class='floating-box'>";
   webSite += "<canvas id='RpmGauge' width='200' height='200'";
   webSite += "style='background-color:#fff'>";
   webSite += "</canvas>";
 webSite += "</div>";

 webSite += "<div class='floating-box'>";
   webSite += "<canvas id='PowerGauge' width='200' height='200'";
   webSite += "style='background-color:#fff'>";
   webSite += "</canvas>";
 webSite += "</div>";

 webSite += "<div class='after-box' id='GaugeDisplay'></div>";
 webSite += "<a href='/settings'>Settings</a>";
}

void handleLamboData() {
  XML = "<?xml version='1.0'?>";
  XML += "<lamboData>";

  XML += "  <MPHL>";
  XML +=      String(MPH[W_LEFT],2);
  XML += "  </MPHL>";

  XML += "  <MPHR>";
  XML +=      String(MPH[W_RIGHT],2);
  XML += "  </MPHR>";

  XML += "  <RPML>";
  XML +=      String(RPM[W_LEFT],2);
  XML += "  </RPML>";

  XML += "  <RPMR>";
  XML +=      String(RPM[W_RIGHT],2);
  XML += "  </RPMR>";

  XML += "  <TICKL>";
  XML +=      String(tickCount[W_LEFT]);
  XML += "  </TICKL>";

  XML += "  <TICKR>";
  XML +=      String(tickCount[W_RIGHT]);
  XML += "  </TICKR>";

  XML += "  <OD>";
  XML +=      String((tickCount[W_RIGHT] / WHEEL_TEETH) * WHEEL_DIAMETER_CM / 100);
  XML += "  </OD>";
  
  XML += "  <BV>";
  XML +=      String(BatteryVoltage, 2);
  XML += "  </BV>";

  XML += "  <DC>";
  XML +=      String(DutyCycle);
  XML += "  </DC>";

  XML += "  <MAXMPH>";
  XML +=      String(MaxMph);
  XML += "  </MAXMPH>";

  XML += "  <GEAR>";
  if(Gear == GNEUTRAL)  XML +=     "NEUTRAL";
  if(Gear == GFORWARD)  XML +=     "FORWARD";
  if(Gear == GREVERSE)  XML +=     "REVERSE";
  if(Gear == GERROR)    XML +=     "ERROR";
  XML += "  </GEAR>";

  XML += "  <TCS>";
  if(TCS == 0)          XML +=     "TCS OFF";
  else if(TCS_Comfort)  XML +=     "TCS Comfort";
  else                  XML +=     "TCS Sport";
  XML += "  </TCS>";

  XML += "</lamboData>";

  server.send(200, "text/xml", XML);
}
