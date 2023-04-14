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
 * By creating a queue to start which contains a randomly shuffled list of
 * every guest's gift. This minimizes the time taken to fetch the current
 * guest's present and add it to the linked list (represented as linked_guests).
 * Then, we use two separate locks, one for the queue and one for the lists to lock
 * others out while they are using data.
 */

#define NUM_GUESTS 100000
#define NUM_SERVANTS 4

std::queue<int> shuffled_guests;
std::vector<int> linked_guests;

bool shuffleInUse = false;
bool linkedInUse = false;

void run_servant_thread() {
  while (!shuffled_guests.empty() || !linked_guests.empty()) {
    
    // Take Present
    if (!shuffled_guests.empty()) {
      while (shuffleInUse) {
        std::this_thread::sleep_for(std::chrono::microseconds(1));
      }
      
      shuffleInUse = true;
      
      int guest_id = shuffled_guests.front();
      shuffled_guests.pop();
      
      shuffleInUse = false;
      
      while (linkedInUse) {
        std::this_thread::sleep_for(std::chrono::microseconds(1));
      }
      
      linkedInUse = true;
      
      if (linked_guests.empty()) {
        linked_guests.push_back(guest_id);
      } else {
        for (int i = 0; i < linked_guests.size(); i++) {
          if (linked_guests[i] > guest_id) {
            linked_guests.insert(linked_guests.begin() + i, guest_id);
            break;
          } else if (i == linked_guests.size() - 1) {
            linked_guests.push_back(guest_id);
            break;
          }
        }
      }
      
      linkedInUse = false;
    }
    
    // Write "Thank You"
    if (!linked_guests.empty()) {
      while (linkedInUse) {
        std::this_thread::sleep_for(std::chrono::microseconds(1));
      }

      linkedInUse = true;
      
      int rand_guest = rand() % linked_guests.size();
      
      linked_guests.erase(linked_guests.begin() + rand_guest);

      linkedInUse = false;
    }
  }
}

int main() {
  srand(time(0));
  std::vector<int> temp_guests(NUM_GUESTS);
  std::iota(temp_guests.begin(), temp_guests.end(), 0);
  std::shuffle(temp_guests.begin(), temp_guests.end(), std::default_random_engine(0));
  
  for (int shuffled_guest : temp_guests) { shuffled_guests.push(shuffled_guest); }
  
  std::thread servantThreads[NUM_SERVANTS];
  auto begin_execution = std::chrono::high_resolution_clock::now();
  
  for (int i = 0; i < NUM_SERVANTS; i++) {
    servantThreads[i] = std::thread(run_servant_thread);
  }

  for (auto& thread : servantThreads) {
    thread.join();
  }

  auto stop_execution = std::chrono::high_resolution_clock::now();
  auto execution_time = std::chrono::duration_cast<std::chrono::microseconds>(
      stop_execution - begin_execution);

  std::cout << "Time Took: " << execution_time.count() << std::endl;

  return 0;
}