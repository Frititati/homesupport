// app.js
const express = require('express');
const bodyParser = require('body-parser');
const cors = require('cors');
const mqtt = require('mqtt');

// Create an instance of express
const app = express();

// Use middleware
app.use(cors());
app.use(bodyParser.json());

// Set up a route for home control API
app.get('/', (req, res) => {
  res.send('Home Control Backend Running');
});

// MQTT Client Setup
const mqttClient = mqtt.connect('mqtt://localhost:1883');
mqttClient.on('connect', () => {
  console.log('Connected to MQTT Broker!');
});

// MQTT Client Error Handling
mqttClient.on('error', (error) => {
  console.error('MQTT Client Error:', error);
});

// Other routes and logic go here...

// Start the server
const PORT = process.env.PORT || 5000;
app.listen(PORT, () => {
  console.log(`Server running on port ${PORT}`);
});
