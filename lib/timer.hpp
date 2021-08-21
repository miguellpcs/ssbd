#if !defined(timer_h)
#define timer_h

#include <chrono>
#include <string>

class Timer
{
public:
    Timer(std::string name) : m_Name(name)
    {
        m_StartTimepoint = std::chrono::high_resolution_clock::now();
    }

    ~Timer()
    {
        Stop();
    }

    void Stop()
    {
        auto endTimepoint = std::chrono::high_resolution_clock::now();

        auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
        auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

        auto duration = end - start;
        double ms = duration * 0.001;

        printf("Timer %s => %ldus (%lfms)\n", m_Name.c_str(), duration, ms);
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimepoint;
    std::string m_Name;
};

#endif // timer_h