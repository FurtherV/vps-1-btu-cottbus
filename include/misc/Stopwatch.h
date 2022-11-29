#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <chrono>
#include <misc/Log.h>
#include <string>
#include <vector>

class Stopwatch {
  public:
    Stopwatch() {}
    ~Stopwatch() {}

    /**
     * @brief Starts measuring time.
     */
    void start() { start_time_point = std::chrono::high_resolution_clock::now(); };

    /**
     * @brief Stops measuring time and records measured time as milliseconds in a vector.
     * @return measured time in milliseconds.
     */
    long stop() {
        auto end_time_point = std::chrono::high_resolution_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(end_time_point - start_time_point).count();
        timings.push_back(duration);
        return duration;
    };

    std::vector<long> timings;

  private:
    std::chrono::high_resolution_clock::time_point start_time_point;
};

#endif // STOPWATCH_H
