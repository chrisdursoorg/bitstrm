// TimeFixture.hpp
//


#ifndef BITSTREAM_TIMEFIXTURE_HPP
#define BITSTREAM_TIMEFIXTURE_HPP

#include <ctime>
#include <iostream>
#include <chrono>
#include <string>

namespace bitstrm{
  
  struct TimeFixture{
    
    TimeFixture(std::string const& name, size_t unit = 0) 
      : m_start(std::chrono::system_clock::now())
      , m_name(name)
      , m_number(s_timer_number++)
      , m_unit(unit) {
      std::cout << "TIMER START [" << m_number << "] - " << m_name
                << std::endl;
    }
 
    ~TimeFixture(){
      
      std::chrono::time_point<std::chrono::system_clock> end
        = std::chrono::system_clock::now();
      std::chrono::duration<double> elapsed_seconds = end - m_start;
      std::time_t end_time = std::chrono::system_clock::to_time_t(end);
      char mbstr[100];
      std::strftime(mbstr, sizeof(mbstr), "%X", std::localtime(&end_time));
      std::cout << "TIMER END [" << m_number << "] " << m_name
                << " -  completed @ " << mbstr << " elapsed time: "
                << elapsed_seconds.count() << "s";
      if(m_unit) {
        double per_unit = elapsed_seconds.count() / m_unit;
        std::cout << " @" << per_unit << " s/cycle freq: "
                  << 1./per_unit << "/s";
      }
      std::cout << std::endl;
    }
    
    static unsigned s_timer_number;
    std::chrono::time_point<std::chrono::system_clock> m_start;
    const std::string m_name;
    const unsigned m_number;
    size_t m_unit;
  };

}

#endif // def'd BITSTREAM_TIMEFIXTURE_HPP
