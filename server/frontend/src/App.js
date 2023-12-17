import React from 'react';
import 'bulma/css/bulma.min.css';


import TemperatureGraph from './TemperatureGraph';


const App = () => {
  return (
    <div style={{ 'padding': '2vh' }}>
      <div className='columns'>
        <div className='column is-6'>
          <TemperatureGraph deviceId="esp32_1" title="ESP32 #1" />
        </div>
        <div className='column is-6'>
          <TemperatureGraph deviceId="esp32_2" title="ESP32 #2" />
        </div>
      </div>
      <div className='columns'>
        <div className='column is-6'>
          <TemperatureGraph deviceId="esp32_3" title="ESP32 #3" />
        </div>
      </div>
    </div>
  );
};

export default App;
