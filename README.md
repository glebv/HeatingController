# HeatingController

* Open the project configuration menu (`idf.py menuconfig`)
* Configure MQTT App Configuration
    * wifi settings: ssid, password, Maximum retry
    * mqtt settings: mqtt broker url, mqtt username, mqtt password
* Flash by `idf.py -p /dev/tty.SLAB_USBtoUART -b 921600 --ccache flash monitor`

## Configuration

* idf.py menuconfig
    * HeatingController configuration
    * Component -> TFT Display -> ESP-WROVER-KIT for ILI9341

### Used Libs
* TFT Lib (mkspiffs, spidriver, spiffs_image, tft) https://github.com/jeremyjh/ESP32_TFT_library/tree/master/components

