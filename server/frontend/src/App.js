import React from 'react';
import TemperatureGraph from './TemperatureGraph';

const App = () => {
  return (
    <div>
      <h1>Temperature Data</h1>
      <TemperatureGraph deviceId="esp32_1" />
    </div>
  );
};

export default App;
