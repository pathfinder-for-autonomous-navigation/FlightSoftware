import './App.css';

function App() {
  return (
    <div className="App">
      <img src="/PANMissionControl.png" alt="Pan Mission Control Log" className='ResponsiveImage'/>
      <div>
        <ul className='Module'>
          <h1 className='ModuleName'>Telemetry</h1>
          <h2>
            <a href="https://mct.panmc.dev" target="_blank">OpenMCT Station</a>
          </h2>
        </ul>

        <ul className='Module'>
          <h1 className='ModuleName'>Uplinks</h1>
          <div>
            <h2 className='Commander'>Leader:</h2>
            <h2 className='Commander'>
              <a href="https://leader.panmc.dev/swagger" target="_blank">Commander</a>
            </h2>
            <h2 className='Log'>
              <a href="https://github.com/pathfinder-for-autonomous-navigation/FlightSoftware/issues/852" target="_blank">Log</a>
            </h2>
          </div>
          
          <div>
            <h2 className='Commander'>Follower:</h2>
            <h2 className='Commander'>
              <a href="https://follower.panmc.dev/swagger" target="_blank">Commander</a>
            </h2>
            <h2 className='Log'>
              <a href="https://github.com/pathfinder-for-autonomous-navigation/FlightSoftware/issues/853" target="_blank">Log</a>
            </h2>
          </div> 
          <h3 className='Notice'>Reminder: Fill out a log for each uplink </h3>
        </ul>
      </div>
    </div>
  );
}

export default App;
