# Octave / MATLAB parser
SensorReadout file format parser written in/for Octave & MATLAB.

## How to use
To use this parser, bring the function file `loadSensorReadoutParser.m` in your path and execute the function,
passing the path to the parser's `src` directory as parameter.

```matlab
loadSensorReadoutParser("src/");
```

For concrete examples of how to use it, have a look at the two files:
- `loadRadioAdvertisements.m`
- `viewSensorsInRecording.m`