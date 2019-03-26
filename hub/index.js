// Require the radiohead-serial module
const http = require('http');
const RadioHeadSerial = require('radiohead-serial').RadioHeadSerial;

// Create an instance of the RadioHeadSerial class
// Uncomment this line when using locally
//const rhs = new RadioHeadSerial('/dev/tty.wchusbserial1410', 9600, 0x01, false);
var rhs = new RadioHeadSerial('/dev/ttyUSB0', 9600, 0x01, false);

rhs.setPromiscuous(true);

// Listen to the 'data' event for received messages
rhs.on('data', function (message) {
    // Convert the decimal from address to hex
    let sender = ('0' + message.headerFrom.toString(16)).slice(-2).toUpperCase();

    // Print a readable form of the data
    if (message.length > 0) {
        let temperature = message.data.readFloatLE(0);
        let humidity = message.data.readFloatLE(4);
        let humiture = message.data.readFloatLE(8);
        let voltage = message.data.readFloatLE(12);
        if (!isNaN(temperature) && !isNaN(humidity) && !isNaN(humiture) && !isNaN(voltage)) {
            console.log(`-> New message of ${message.length} bytes from 0x${sender}: ${temperature}ºC | ${humidity}% | Feels like ${humiture}ºC | ${voltage} Volts`);

            const data = JSON.stringify({
                roomid: 1,
                temperature: temperature,
                humidity: humidity,
                humiture: humiture,
                voltage: voltage,
                timestamp: new Date().getTime()
            });

            const req = http.request({
                host: 'localhost',
                port: 9200,
                path: '/smart-home/climate',
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                    'Content-Length': data.length
                }
            }, (res) => {
                res.on('data', () => {
                    console.log('  [✓] Data stored in ElasticSearch');
                });
            });

            req.on('error', (e) => {
                console.error(`  [x] Failed to save data to ElasticSearch: ${e.message}`);
            });

            req.write(data);
            req.end();
        }
    }
});

rhs.on('error', function (err) {
    console.log(`[x] Failed to get message from 433MHz RX: ${err}`);
});

// Print some info
console.log('Starting hub...');
console.log('Listening form messages on 433MHz frequency...');

