const express = require('express');
const { Pool } = require('pg');
const cors = require('cors');
const app = express();
const port = 3000;

app.use(cors());


// PostgreSQL pool
const pool = new Pool({
  user: 'postgres',
  host: 'filipi.local',
  database: 'production',
  password: 'KPqMh8Xg8KCnjqvSDnt3',
  port: 5432,
});

app.get('/device/number', async (req, res) => {
  try {
    const result = await pool.query('SELECT COUNT(*) FROM temperature_readings');
    res.json({ numberOfDevices: result.rows[0].count });
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

app.get('/temperature/:device/all', async (req, res) => {
  const deviceId = req.params.device;
  try {
    const result = await pool.query('SELECT * FROM temperature_readings WHERE device_id = $1 ORDER BY ts', [deviceId]);
    res.json(result.rows);
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

app.get('/temperature/:device/hourly', async (req, res) => {
  const deviceId = req.params.device;
  try {
    const result = await pool.query(
      'SELECT date_trunc(\'hour\', ts) as hour, AVG(temperature) as temperature, AVG(humidity) as humidity, AVG(heat_index) as heat_index ' +
      'FROM temperature_readings ' +
      'WHERE device_id = $1 ' +
      'GROUP BY date_trunc(\'hour\', ts) ' +
      'ORDER BY date_trunc(\'hour\', ts)',
      [deviceId]
    );
    res.json(result.rows);
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

app.get('/temperature/:device/quarter-hourly', async (req, res) => {
  const deviceId = req.params.device;
  try {
    const result = await pool.query(
      'SELECT date_trunc(\'minute\', ts) - INTERVAL \'15 minutes\' * ((EXTRACT(MINUTE FROM ts)::integer / 15)::integer) as interval_start, ' +
      'AVG(temperature) as temperature, AVG(humidity) as humidity, AVG(heat_index) as heat_index ' +
      'FROM temperature_readings ' +
      'WHERE device_id = $1 ' +
      'GROUP BY interval_start ' +
      'ORDER BY interval_start',
      [deviceId]
    );
    res.json(result.rows);
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

app.listen(port, () => {
  console.log(`Server running on http://localhost:${port}`);
});
