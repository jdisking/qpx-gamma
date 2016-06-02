/*******************************************************************************
 *
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 *
 * This software can be redistributed and/or modified freely provided that
 * any derivative works bear some notice that they are derived from it, and
 * any modified versions bear some notice that they have been modified.
 *
 * Author(s):
 *      Martin Shetty (NIST)
 *
 * Description:
 *      Types for organizing data aquired from device
 *        Qpx::Hit        single energy event with coincidence flags
 *
 ******************************************************************************/

#ifndef QPX_TIME_STAMP
#define QPX_TIME_STAMP

#include "xmlable.h"
#include <string>
#include <cmath>
#include <fstream>
#include "qpx_util.h"

namespace Qpx {

class TimeStamp
{

public:
  inline TimeStamp()
    : time_native(0)
    , timebase_multiplier(1)
    , timebase_divider(1)
  {}

  inline TimeStamp(uint32_t multiplier, uint32_t divider)
    : time_native(0)
    , timebase_multiplier(multiplier)
    , timebase_divider(divider)
  {
    if (!timebase_multiplier)
      timebase_multiplier = 1;
    if (!timebase_divider)
      timebase_divider = 1;
  }

  inline TimeStamp(const TimeStamp &model, uint64_t native)
    : TimeStamp(model)
  {
    time_native = native;
  }

  static inline TimeStamp common_timebase(const TimeStamp& a, const TimeStamp& b)
  {
    if (a.timebase_divider == b.timebase_divider)
    {
      if (a.timebase_multiplier < b.timebase_multiplier)
        return a;
      else
        return b;
    }
    else
      return TimeStamp(1, lcm(a.timebase_divider, b.timebase_divider));
  }

  inline double operator-(const TimeStamp other) const
  {
    return (to_nanosec() - other.to_nanosec());
  }

  inline bool same_base(const TimeStamp other) const
  {
    return ((timebase_divider == other.timebase_divider) && (timebase_multiplier == other.timebase_multiplier));
  }

  inline double to_nanosec(uint64_t native) const
  {
    return native * double(timebase_multiplier) / double(timebase_divider);
  }

  inline int64_t to_native(double ns)
  {
    return std::ceil(ns * double(timebase_divider) / double(timebase_multiplier));
  }

  inline double to_nanosec() const
  {
    return time_native * double(timebase_multiplier) / double(timebase_divider);
  }

  inline void delay(double ns)
  {
    if (ns > 0)
      time_native += std::ceil(ns * double(timebase_divider) / double(timebase_multiplier));
  }

  inline bool operator<(const TimeStamp other) const
  {
    if (same_base((other)))
      return (time_native < other.time_native);
    else
      return (to_nanosec() < other.to_nanosec());
  }

  inline bool operator>(const TimeStamp other) const
  {
    if (same_base((other)))
      return (time_native > other.time_native);
    else
      return (to_nanosec() > other.to_nanosec());
  }

  inline bool operator<=(const TimeStamp other) const
  {
    if (same_base((other)))
      return (time_native <= other.time_native);
    else
      return (to_nanosec() <= other.to_nanosec());
  }

  inline bool operator>=(const TimeStamp other) const
  {
    if (same_base((other)))
      return (time_native >= other.time_native);
    else
      return (to_nanosec() >= other.to_nanosec());
  }

  inline bool operator==(const TimeStamp other) const
  {
    if (same_base((other)))
      return (time_native == other.time_native);
    else
      return (to_nanosec() == other.to_nanosec());
  }

  inline bool operator!=(const TimeStamp other) const
  {
    if (same_base((other)))
      return (time_native != other.time_native);
    else
      return (to_nanosec() != other.to_nanosec());
  }

  inline void write_bin(std::ofstream &outfile) const
  {
    outfile.write((char*)&time_native, sizeof(time_native));
//    uint16_t time_hy = (time_native >> 48) & 0x000000000000FFFF;
//    uint16_t time_hi = (time_native >> 32) & 0x000000000000FFFF;
//    uint16_t time_mi = (time_native >> 16) & 0x000000000000FFFF;
//    uint16_t time_lo =  time_native        & 0x000000000000FFFF;
//    outfile.write((char*)&time_hy, sizeof(time_hy));
//    outfile.write((char*)&time_hi, sizeof(time_hi));
//    outfile.write((char*)&time_mi, sizeof(time_mi));
//    outfile.write((char*)&time_lo, sizeof(time_lo));
  }

  inline void read_bin(std::ifstream &infile)
  {
    infile.read(reinterpret_cast<char*>(&time_native), sizeof(time_native));
//    uint16_t entry[4];
//    infile.read(reinterpret_cast<char*>(entry), sizeof(uint16_t)*4);
//    uint64_t time_hy = entry[0];
//    uint64_t time_hi = entry[1];
//    uint64_t time_mi = entry[2];
//    uint64_t time_lo = entry[3];
//    time_native = (time_hy << 48) + (time_hi << 32) + (time_mi << 16) + time_lo;
  }

  void from_xml(const pugi::xml_node &);
  void to_xml(pugi::xml_node &) const;
  std::string xml_element_name() const {return "TimeStamp";}
  std::string to_string() const;

private:
  uint64_t time_native;
  uint32_t timebase_multiplier;
  uint32_t timebase_divider;
};

}

#endif
