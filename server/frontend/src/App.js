import React from 'react';
import 'bulma/css/bulma.min.css';


import TemperatureGraph from './TemperatureGraph';


const App = () => {
  return (
    <div style={{ 'padding': '2vh' }}>
      <div className='columns'>
        <div className='column is-6'>
          <div className='box'>
            <h1 className='title'>Temperature ESP32 #1</h1>
            <TemperatureGraph deviceId="esp32_1" />
          </div>
        </div>
        <div className='column is-6'>
          <div className='box'>
            <h1 className='title'>Temperature ESP32 #1</h1>
            <TemperatureGraph deviceId="esp32_1" />
          </div>
        </div>
      </div>
    </div>
  );
};

export default App;
