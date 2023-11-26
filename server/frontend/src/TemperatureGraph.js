import React, { useState, useEffect } from 'react';
import axios from 'axios';
import { Line } from 'react-chartjs-2';
import 'chart.js/auto';

const TemperatureGraph = ({ deviceId }) => {
  const [data, setData] = useState({
    labels: [],
    datasets: [
      {
        label: 'Average Temperature',
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
        const response = await axios.get(`http://localhost:3000/temperature/${deviceId}/quarter-hourly`);
        const labels = response.data.map(item => new Date(item.interval_start).toLocaleTimeString());
        const tempData = response.data.map(item => item.temperature);

        setData({
          ...data,
          labels,
          datasets: [{ ...data.datasets[0], data: tempData }],
        });
      } catch (error) {
        console.error('Error fetching data:', error);
      }
    };

    fetchData();
  }, [deviceId]);

  return <Line data={data} />;
};

export default TemperatureGraph;
