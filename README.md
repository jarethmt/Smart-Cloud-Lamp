# Smart Cloud Lamp #

You know those [super cool cloud lamps](https://www.google.com/search?q=diy+cloud+lamp&client=ubuntu&hs=dPl&channel=fs&sxsrf=APq-WBv57XFWb5iN7OBHuYRzZNvmq4Crvg:1645583905466&source=lnms&tbm=isch&sa=X&ved=2ahUKEwjG_ofh5ZT2AhVzjokEHfgDBKMQ_AUoAnoECAEQBA&biw=1600&bih=783&dpr=1) that people make using polyfil and some LEDs? Well, take yours to the next level with this code! Rather than a basic LED strip, you can wire your lamp up with some Neopixel LEDs and an ESP8266 board, and turn it into a smart weather station! This code hooks up to the Open Weather API to get the outside weather from your location, and then it displays a custom light pattern based on the outside conditions. So far, there are patterns coded for the following conditions:
 - Clear
 - Cloudy
 - Mist
 - Drizzle
 - Rain
 - Thunderstorm
 - Snow

## Installation ##

Simply clone this repo or open the Cloud_Lamp.ino filer in the Arduino IDE. Then, hook up your ESP8266 board and flash this to it. Before flashing, make sure to set the ssid, password, openWeatherMapApiKey, city, and countryCode variables at the top of the file. This will allow the board to connect up to your local network. in order to get an API key to use with Open Weather, you can follow the instructions in [this blog post](https://randomnerdtutorials.com/esp32-http-get-open-weather-map-thingspeak-arduino/).

## Future Improvements ##

 - Possibly a generic color / party mode
 - Button input to cycle through modes and turn light on / off
 - Web server to cycle modes and turn the lights on / off
