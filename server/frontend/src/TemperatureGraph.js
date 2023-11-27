import React, { useState, useEffect } from 'react';
import axios from 'axios';
import { Line } from 'react-chartjs-2';
import 'chart.js/auto';

const TemperatureGraph = ({ deviceId }) => {
  const [data, setData] = useState({
    labels: [],
    datasets: [
      {
        label: 'Temperature',
        data: [],
        borderColor: 'rgb(75, 192, 192)',
        backgroundColor: 'rgba(75, 192, 192, 0.5)',
      },
      // Add more datasets for humidity, heat index, etc., if needed
    ],
  });

  useEffect(() => {
    const fetchData = async () => {
      try {
        const response = await axios.get(`http://localhost:3000/temperature/${deviceId}/all`);

        let response_data = response.data;
        let x_label = null;
        let temperature_data = null;

        if (true) {
          let a = response_data[0];
          let b = response_data[response_data.length - 1];

          // console.log(a);
          // console.log(b);

          let first_ts = new Date(a.ts);
          let last_ts = new Date(b.ts);
          // console.log("AA " + first_ts.toLocaleString() + " BB " + last_ts.toLocaleString());
          const hours_s = [];
          const hours_e = [];
          // const hours_b = [];
          const vals = [];

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
            let start = hours_s[i];
            let end = hours_e[i];

            let temp_list = [];
            for (let j = 0; j < response_data.length; j++) {
              // console.log("AAA" + response_data[j].ts);
              let temp = new Date(response_data[j].ts);
              // console.log("temp : " + temp.toLocaleString());
              if (temp > start) {
                if (temp < end) {
                  // console.log("count");
                  temp_list.push(parseFloat(response_data[j].temperature.replace(",", ".")));
                }
              }
            }

            // console.log(temp_list);

            var sum = 0;
            for (var number of temp_list) {
              sum += number;
            }
            let average = sum / Math.max(temp_list.length, 1);
            // console.log(average);

            vals.push(average);
          }

          x_label = hours_s.splice(-48).map(item => new Date(item).toLocaleString());
          temperature_data = vals.splice(-48);

        } else {
          response_data = response_data.slice(-50);
          x_label = response_data.map(item => new Date(item.ts).toLocaleTimeString());
          temperature_data = response_data.map(item => item.temperature);
        }

        setData({
          ...data,
          labels: x_label,
          datasets: [{ ...data.datasets[0], data: temperature_data }],
        });
      } catch (error) {
        console.error('Error fetching data:', error);
      }
    };

    fetchData();
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

  return <Line data={data} options={options} />;
};

export default TemperatureGraph;
