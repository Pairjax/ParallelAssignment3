#include <iostream>
#include <fstream>
#include <cmath>
#include <numeric>
#include <vector>
#include <chrono>
#include <algorithm>
#include <thread>
#include <queue>
#include <random>

/*
 * I'm cutting the time to 1 second per temperature reading & 10 seconds per report to save time.
 * Each temperature is stored in a vector which is held in an array whose index is tied to the
 * thread it belongs to. This ensures that each thread doesn't touch other's data. When the report
 * is due, it locks all other threads and collects the data to analyze before clearing all of the
 * data in each vector, starting the process over again.
 *
 * This ensures the report & data collection is timely with no hiccups as threads wait for each other
 * to run.
 */

#define MIN_TEMP -100
#define MAX_TEMP 70

#define NUM_THREADS 8

#define TIME_INTERVAL_TEMP std::chrono::seconds(1)
#define TIME_INTERVAL_REPORT std::chrono::seconds(10)

std::vector<int> temperature_memory[NUM_THREADS];

bool creating_report = false;

void run_sensor_thread(int sensor_index) {
  std::srand(sensor_index); // Refresh seed else generate the same number
  
  while (true) {
    while (creating_report) {
      std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
    
    std::this_thread::sleep_for(TIME_INTERVAL_TEMP);
    
    int recorded_temperature = MIN_TEMP + (rand() % (MAX_TEMP - MIN_TEMP));
    temperature_memory[sensor_index].push_back(recorded_temperature);
  }
}

int main() {
  std::thread roverThreads[NUM_THREADS];
  
  for (int i = 0; i < NUM_THREADS; i++) {
    roverThreads[i] = std::thread(run_sensor_thread, i);
  }
  
  while (true) {
    std::this_thread::sleep_for(TIME_INTERVAL_REPORT);
    
    creating_report = true;
    
    std::vector<int> all_measurements;
    all_measurements.reserve(temperature_memory[0].size() * NUM_THREADS);
    
    for (auto& sensor : temperature_memory) {
      all_measurements.insert(all_measurements.end(), sensor.begin(), sensor.end());
    }
    
    std::sort(all_measurements.begin(), all_measurements.end());
    
    std::cout << "TEMPERATURE REPORT" << std::endl << "=================" << std::endl;
    
    std::cout << "Highest Temperatures" << std::endl << "=---------------=" << std::endl;
    for (int i = all_measurements.size() - 1; i > all_measurements.size() - 6; i--) {
      std::cout << all_measurements[i] << " ";
    }
    std::cout << std::endl;
    
    std::cout << "Lowewst Temperatures" << std::endl << "=---------------=" << std::endl;
    for (int i = 0; i < 5; i++) {
      std::cout << all_measurements[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "Highest Temperature Difference" << std::endl << "=---------------=" << std::endl;
    int largest = 0;
    for (int i = 0; i < temperature_memory[0].size(); i++) {
      int max = INT_MIN;
      int min = INT_MAX;
      
      for (int j = 0; j < NUM_THREADS; j++) {
        int temp = temperature_memory[j][i];
        
        if (temp > max) { max = temp; }
        if (temp < min) { min = temp; }
      }
      
      if ((max - min) > largest) {
        largest = abs(max - min);
      }
    }
    
    std::cout << largest << std::endl;
    
    for (auto& sensor : temperature_memory) {
      sensor.clear();
    }
    
    creating_report = false;
  }

  for (auto& thread : roverThreads) {
    thread.join();
  }

  return 0;
}