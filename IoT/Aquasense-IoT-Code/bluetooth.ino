#include <Wire.h>
#include <VL53L0X.h>

VL53L0X sensor;

// Container dimensions
const float containerHeight = 265.0; // mm
const float containerWidth = 120.0;   // mm

// Water level measurements
const int waterLevelMeasurements[] = {270, 260, 250, 235, 210, 190, 170, 149, 127, 110, 95, 55, 40, 33, 20}; // mm
const int waterVolume[] = {0, 118, 236, 354, 475, 590, 708, 826, 944, 1062, 1180, 1298, 1416, 1534, 1652};     // ml

void setup()
{
    Serial.begin(9600);
    // Serial1.begin(9600);

    Wire.begin();
    sensor.init();
    sensor.setTimeout(500);

    // Start continuous back-to-back mode (take readings as
    // fast as possible).  To use continuous timed mode
    // instead, provide a desired inter-measurement period in
    // ms (e.g. sensor.startContinuous(100)).
    sensor.startContinuous();
}

void loop()
{
    unsigned long startTime = millis(); // Store the start time of the 10-second interval
    int distance;
    int waterVolumeResults[60]; // Store volume results for 10 seconds (60 readings at 6 readings per second)
    int counter = 0; // Counter for readings

    while (millis() - startTime < 2000) // Run for 10 seconds
    {
        distance = sensor.readRangeContinuousMillimeters();

        // Calculate water level, water distance, and water volume
        
        float waterLevel = calculateWaterLevel(distance);
        float waterDistance = calculateWaterDistance(distance);
        float waterVolumeLevel = calculateWaterVolumeLevel(waterLevel);

        // Store water volume result in the array
  // Serial.print("Distance: ");
  // Serial.print(distance);
  // Serial.println(" mm");

  // Serial.print("Water Level: ");
  // Serial.print(waterLevel);
  // Serial.println("%");

  // Serial.print("Water Distance: ");
  // Serial.print(waterDistance);
  // Serial.println(" mm");

  // Serial.print("Water Volume Level: ");
  // Serial.print(waterVolumeLevel);
  
  // Serial.println(" ml");
        waterVolumeResults[counter] = waterVolumeLevel;

     if (sensor.timeoutOccurred())
    {
        // Restart the loop by jumping back to the beginning
        Serial.println("TIMEOUT");
 // Jump back to the beginning of the loop
    }

        counter++;
        delay(100); // Delay for approximately 166 milliseconds for a total of 6 readings per second
    }

    // Calculate mode of water volume results
    int mode = calculateMode(waterVolumeResults, counter);


    Serial.print(mode);


    Serial.println();
    
}


float calculateWaterLevel(int distance)
{
    // Interpolate water volume for given distance
    int index = 0;
    for (int i = 0; i < sizeof(waterLevelMeasurements) / sizeof(waterLevelMeasurements[0]); i++)
    {
        if (distance >= waterLevelMeasurements[i])
        {
            index = i;
            break;
        }
    }

    // Calculate water level percentage
    float volumeAbove = 0.0;
    float volumeBelow = 0.0;

    if (index == 0)
    {
        volumeAbove = waterVolume[0];
        volumeBelow = 0;
    }
    else
    {
        volumeAbove = waterVolume[index];
        volumeBelow = waterVolume[index - 1];
    }

    float interpolatedVolume = volumeBelow + (volumeAbove - volumeBelow) * (float(distance - waterLevelMeasurements[index - 1]) / float(waterLevelMeasurements[index] - waterLevelMeasurements[index - 1]));

    float waterLevel = (interpolatedVolume / 1652.0) * 100.0; // 1652 ml is the total volume at the lowest water level
    return waterLevel;
}

float calculateWaterDistance(int distance)
{
    // Calculate water distance from the bottom of the container
    float waterDistance = containerHeight - distance;
    // Ensure water distance does not exceed the limits
    if (waterDistance > containerHeight)
    {
        waterDistance = containerHeight;
    }
    else if (waterDistance < 0)
    {
        waterDistance = 0;
    }
    return waterDistance;
}

float calculateWaterVolumeLevel(float waterLevel)
{
    // Find the corresponding water volume for the given water level
    float waterVolumeLevel = (waterLevel / 100.0) * 1652.0; // 1652 ml is the total volume at the lowest water level
    return waterVolumeLevel;
}

int calculateMode(int arr[], int n)
{
    // Initialize variables to store mode and frequency
    int mode = arr[0];
    int maxCount = 1;

    for (int i = 0; i < n; i++)
    {
        int count = 0; // Counter for current element
        for (int j = i + 1; j < n; j++)
        {
            if (arr[i] == arr[j]) // If element appears again
                count++;
        }
        if (count > maxCount) // If count is higher than maxCount, update mode and maxCount
        {
            mode = arr[i];
            maxCount = count;
        }
        
    }
    return mode;
}

