  const char HOME_SCRIPT[] =
  "<SCRIPT>"
  "var xmlHttp=createXmlHttpObject();"

  "function createXmlHttpObject(){"
  " if(window.XMLHttpRequest) {"
      //code for modern browsers
  "   xmlHttp=new XMLHttpRequest();"
  " } else {"
      //code for IE6, IE5
  "    xmlHttp = new ActiveXObject('Microsoft.XMLHTTP');"
  " }"
  " return xmlHttp;"
  "}"

  "function process(){"
  " if(xmlHttp.readyState==0 || xmlHttp.readyState==4){"
  "   xmlHttp.open('PUT','lamboData',true);"
  "   xmlHttp.onreadystatechange=updateDisplayValues;" 
  "   xmlHttp.send(null);"
  " }"
  " setTimeout('process()',500);"
  "}"

  "function updateDisplayValues(){"
  " if(xmlHttp.readyState==4 && xmlHttp.status==200){"
  "   xmlResponse=xmlHttp.responseXML;"
  "   xmldoc = xmlResponse.getElementsByTagName('lamboData');"
  
  "   var HomeHtml='<p>MPH Left: ' +"
  "   xmldoc[0].getElementsByTagName('MPHL')[0].childNodes[0].nodeValue + "

  "   '<br>Duty Cycle:' + "
  "   xmldoc[0].getElementsByTagName('DC')[0].childNodes[0].nodeValue + "

  "   '<br>Battery Voltage:' + "
  "   xmldoc[0].getElementsByTagName('BV')[0].childNodes[0].nodeValue + "

  "   '<br>Gear:' + "
  "   xmldoc[0].getElementsByTagName('GEAR')[0].childNodes[0].nodeValue + '</p>';"

  "   document.getElementById('HomeDisplay').innerHTML=HomeHtml;"
  " }"
  "}"
  "</SCRIPT>";
