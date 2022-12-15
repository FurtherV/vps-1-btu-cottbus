#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <chrono>
#include <fstream>
#include <iostream>
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
     * @brief Records measured time in milliseconds.
     * @return measured time in milliseconds.
     */
    long stop() {
        auto end_time_point = std::chrono::high_resolution_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(end_time_point - start_time_point).count();
        timings.push_back(duration);
        return duration;
    };

    /**
     * @brief Writes all measured timings to a csv file, each prefixed with their stop index, each on their own line.
     * @param path Path to the file
     */
    void to_file(std::string path) {
        std::ofstream csv;
        csv.open(path, ios::out | ios::trunc);
        if (csv.is_open()) {
            for (size_t index = 0; index < timings.size(); index++) {
                csv << index << "," << timings[index] << "\n";
            }
            csv.close();
        }
    }

    std::vector<long> timings;

  private:
    std::chrono::high_resolution_clock::time_point start_time_point;
};

#endif // STOPWATCH_H
