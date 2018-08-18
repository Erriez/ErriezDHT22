rem Install Python27 platformio
rem C:\Python27\Scripts\pip.exe install -U platformio

rem Set Python path
set PATH=%PATH%;C:\Python27
set PATH=%PATH%;C:\Python27\Scripts

rem Update library
rem git fetch
rem git pull

rem Build example(s)
platformio ci --lib=".." --project-conf=platformio.ini ../examples/DHT22/DHT22.ino
platformio ci --lib=".." --project-conf=platformio.ini ../examples/DHT22Average/DHT22Average.ino
platformio ci --lib=".." --project-conf=platformio.ini ../examples/DHT22DurationTest/DHT22DurationTest.ino

@pause