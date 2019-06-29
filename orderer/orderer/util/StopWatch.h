#include <iostream>
#include <chrono>
#include "StringFormat.h"


namespace MED {

    class StopWatch {
    public:
        StopWatch() :
            m_beg(clock_::now()) {
            total = 0.0;
        }
        void reset() {
            m_beg = clock_::now();
        }

        std::string  check() {
            double t = std::chrono::duration_cast<std::chrono::milliseconds>(clock_::now() - m_beg).count() / 1000.0;
            total += t;
            reset();
            return MED::STRING::string_format(" %f sec", t);
        }

        std::string  stop() {
            double t = std::chrono::duration_cast<std::chrono::milliseconds>(clock_::now() - m_beg).count() / 1000.0;
            total += t;
            return MED::STRING::string_format(" %f sec", total);
        }

     
    private:
        typedef std::chrono::high_resolution_clock clock_;
        typedef std::chrono::duration<double, std::ratio<1> > second_;
        std::chrono::time_point<clock_> m_beg;
        double total = 0.0;
    };

}
