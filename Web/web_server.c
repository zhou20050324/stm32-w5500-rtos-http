#include "web_server.h"
#include "httpServer.h"
#include <string.h>

static const char index_html[] =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Cache-Control: no-store, no-cache, must-revalidate, max-age=0\r\n"
    "Pragma: no-cache\r\n"
    "Expires: 0\r\n"
    "Connection: close\r\n\r\n"
    "<!DOCTYPE html><html><head><meta charset=UTF-8><meta name=viewport content='width=device-width,initial-scale=1.0'>"
    "<style>"
    "body{background:#222;color:#ddd;font-family:Arial;margin:20px}"
    ".card{background:#333;border-radius:8px;padding:15px;margin:10px 0}"
    ".val{font-size:28px;color:#0f0}"
    "#clock{font-size:16px;color:#aaa;margin-top:20px}"
    "</style></head><body>"
    "<h1>STM32 Sensor Monitor</h1>"
    "<div class=card>MCU Internal Temp: <span id=t1 class=val>--</span> &deg;C</div>"
    "<div class=card>Light: <span id=l1 class=val>--</span> V</div>"
    "<div class=card>VREF: <span id=v1 class=val>--</span> V</div>"
    "<div class=card>DHT11 Temp: <span id=dt1 class=val>--</span> &deg;C</div>"
    "<div class=card>DHT11 Humidity: <span id=dh1 class=val>--</span> %</div>"
    "<div id=clock>Time: --</div>"
    "<script>"
    "function updateData(){"
    "var x=new XMLHttpRequest();x.open('GET','/data.json');x.onload=function(){"
    "if(x.status==200){"
    "var d=JSON.parse(x.responseText);"
    "document.getElementById('t1').textContent=d.mcu_temp.toFixed(2);"
    "document.getElementById('l1').textContent=d.light_v.toFixed(3);"
    "document.getElementById('v1').textContent=d.vref.toFixed(3);"
    "document.getElementById('dt1').textContent=d.dht_temp.toFixed(1);"
    "document.getElementById('dh1').textContent=d.dht_humi.toFixed(1);"
    "}};x.send();}"
    "function updateClock(){"
    "document.getElementById('clock').textContent='Time: '+new Date().toLocaleTimeString();}"
    "setInterval(updateData,1000);"
    "setInterval(updateClock,1000);"
    "updateData();updateClock();"
    "</script></body></html>";

void web_server_init(void)
{
    reg_httpServer_webContent((uint8_t *)"index.html", (uint8_t *)index_html);
}