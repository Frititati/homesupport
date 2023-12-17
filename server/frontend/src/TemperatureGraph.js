import React, { useState, useEffect } from 'react';
import axios from 'axios';
import { Line } from 'react-chartjs-2';
import 'chart.js/auto';

const TemperatureGraph = ({ deviceId, title }) => {
  const [currentTemperature, setCurrentTemperature] = useState(0.0);
  const [lastUpdate, setLastUpdate] = useState("");

  const [graphData, setGraphData] = useState({
    labels: [],
    datasets: [
      {
        label: 'Temperature',
        data: [],
        borderColor: 'rgb(75, 192, 192)',
        backgroundColor: 'rgba(75, 192, 192, 0.5)',
      },
      {
        label: 'Humidity',
        data: [],
        borderColor: 'rgb(90, 192, 192)',
        backgroundColor: 'rgba(90, 192, 192, 0.5)',
      },
      // Add more datasets for humidity, heat index, etc., if needed
    ],
  });

  useEffect(() => {
    const fetchGraphData = async () => {
      try {
        const response = await axios.get(`http://localhost:3000/temperature/${deviceId}/twodays`);

        let response_data = response.data;
        let x_label = null;
        let temperature_data = null;
        let humidity_data = null;

        if (true) {
          let first_ts = new Date(response_data[0].ts);
          let last_ts = new Date(response_data[response_data.length - 1].ts);

          const hours_s = [];
          const hours_e = [];
          const temperature_vals = [];
          const humidity_vals = [];

          while (first_ts <= last_ts) {
            let temp_date = new Date(first_ts.getTime());
            let current_hour = temp_date.getHours();
            hours_s.push(temp_date.setUTCHours(current_hour, 0, 0, 0));
            hours_e.push(temp_date.setUTCHours(current_hour, 59, 59, 999));
            first_ts.setTime(first_ts.getTime() + 60 * 60 * 1000);
          }

          // console.log(hours_s.map(item => new Date(item).toLocaleString()));
          // console.log(hours_e.map(item => new Date(item).toLocaleString()));

          for (let i = 0; i < hours_s.length; i++) {
            let start = new Date(hours_s[i]);
            let end = new Date(hours_e[i]);

            let temperature_list = [];
            let humidity_list = [];
            for (let j = 0; j < response_data.length; j++) {
              // console.log("AAA" + response_data[j].ts);
              let item_date = new Date(response_data[j].ts);
              // console.log("temp : " + temp.toLocaleString());
              if (item_date.getTime() > start.getTime()) {
                if (item_date.getTime() < end.getTime()) {
                  // console.log("count");
                  temperature_list.push(parseFloat(response_data[j].temperature.replace(",", ".")));
                  humidity_list.push(parseFloat(response_data[j].humidity));
                }
              }
            }

            // console.log(temp_list);

            var temperature_sum = 0;
            for (var number of temperature_list) {
              temperature_sum += number;
            }
            let temperature_average = temperature_sum / Math.max(temperature_list.length, 1);
            
            temperature_vals.push(temperature_average);

            var humidity_sum = 0;
            for (var number of humidity_list) {
              humidity_sum += number;
            }
            let humidity_average = humidity_sum / Math.max(humidity_list.length, 1);
            // console.log(average);

            humidity_vals.push(humidity_average);
          }

          x_label = hours_s.splice(-48).map(item => new Date(item).toLocaleString());
          temperature_data = temperature_vals.splice(-48);
          humidity_data = humidity_vals.splice(-48);

        } else {
          response_data = response_data.slice(-50);
          x_label = response_data.map(item => new Date(item.ts).toLocaleTimeString());
          temperature_data = response_data.map(item => item.temperature);
        }

        setGraphData({
          ...graphData,
          labels: x_label,
          datasets: [{ ...graphData.datasets[0], data: temperature_data }],
          // { ...data.datasets[1], data: humidity_data }
        });
      } catch (error) {
        console.error('Error fetching data:', error);
      }
    };

    fetchGraphData();
  }, [deviceId]);

  useEffect(() => {
    const fetchTemperatureData = async () => {
      try {
        const response = await axios.get(`http://localhost:3000/temperature/${deviceId}/last`);

        let response_data = response.data;
        // console.log(response_data);

        setCurrentTemperature(response_data[0].temperature);
        var last_update_date = new Date(response_data[0].ts);
        setLastUpdate(last_update_date.toLocaleString());

      } catch (error) {
        console.error('Error fetching data:', error);
      }
    };

    fetchTemperatureData();
  }, [deviceId]);

  const options = {
    scales: {
      y: {
        min: 0,
        max: 40,
        title: {
          display: true,
          text: 'Temperature (°C)'
        }
      }
    }
  };

  return (
    <div className='box'>
      <h1 className='title'>{title} :: {currentTemperature}&deg; at {lastUpdate}</h1>
      <Line data={graphData} options={options} />
    </div>
  );
};

export default TemperatureGraph;
