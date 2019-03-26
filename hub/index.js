const Influx = require('influx');
const RadioHeadSerial = require('radiohead-serial').RadioHeadSerial;
// Constants
const DATABASE_HOST = 'localhost';
const DATABASE_NAME = 'smart-home';
const DATABASE_SCHEMA = {
    measurement: 'climate',
    fields: {
        temperature: Influx.FieldType.FLOAT,
        humidity: Influx.FieldType.FLOAT,
        humiture: Influx.FieldType.FLOAT,
        voltage: Influx.FieldType.FLOAT,
    },
    tags: ['room']
};

// Create instance of InfluxDB class
const influx = new Influx.InfluxDB({
    host: DATABASE_HOST,
    database: DATABASE_NAME,
    schema: [DATABASE_SCHEMA]
});

// Check is DB exists, and create it if it doesn't
influx.getDatabaseNames().then(names => {
    return !names.includes(DATABASE_NAME)
        ? influx.createDatabase(DATABASE_NAME)
        : Promise.resolve();
}).then(() => {
    console.log(`[✓] Influx database ${DATABASE_NAME} didn't exist and has been created`);
}).catch(err => {
    console.error(`[x] Error creating Influx database ${DATABASE_NAME}: ${err}`);
});

// Create an instance of the RadioHeadSerial class
// Uncomment this line when using locally
//const rhs = new RadioHeadSerial('/dev/tty.wchusbserial1410', 9600, 0x01, false);
const rhs = new RadioHeadSerial('/dev/ttyUSB0', 9600, 0x01, false);

rhs.setPromiscuous(true);

// Listen to the 'data' event for received messages
rhs.on('data', function (message) {
    if (message.length > 0) {
        let temperature = message.data.readFloatLE(0);
        let humidity = message.data.readFloatLE(4);
        let humiture = message.data.readFloatLE(8);
        let voltage = message.data.readFloatLE(12);
        if (!isNaN(temperature) && !isNaN(humidity) && !isNaN(humiture) && !isNaN(voltage)) {
            console.log(`-> New message of ${message.length} bytes: ${temperature}ºC | ${humidity}% | Feels like ${humiture}ºC | ${voltage} Volts`);

            influx.writePoints([{
                measurement: DATABASE_SCHEMA.measurement,
                fields: {
                    temperature: temperature,
                    humidity: humidity,
                    humiture: humiture,
                    voltage: voltage,
                },
                tags: {
                    room: 1
                },
            }]).then(() => {
                console.log('  [✓] Stored in InfluxDB');
            }).catch(error => {
                console.error(`  [x] Failed to store data in InfluxDB: ${error}`);
            });
        }
    }
});

rhs.on('error', function (err) {
    console.log(`[x] Failed to get message from 433MHz RX: ${err}`);
});

// Print some info
console.log('Starting hub...');
console.log('Listening for messages on 433MHz frequency...');

