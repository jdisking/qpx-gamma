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
 *      Qpx::Spectrum::SpectrumTime one-dimensional spectrum
 *
 ******************************************************************************/

#ifndef SPECTRUM_TIME_H
#define SPECTRUM_TIME_H

#include "spectrum.h"

namespace Qpx {
namespace Spectrum {

class SpectrumTime : public Spectrum
{
public:
  SpectrumTime() {}

  static Template get_template() {
    Template new_temp;
    new_temp.type = "Time";
    new_temp.description = "Time-domain log of activity";
    return new_temp;
  }
  

protected:
  std::string my_type() const override {return "Time";}
  XMLableDB<Gamma::Setting> default_settings() const override {return this->get_template().generic_attributes; }

  //1D is ok with all patterns
  bool initialize() override {
    metadata_.type = my_type();
    metadata_.dimensions = 1;

    return true;
  }

  PreciseFloat _get_count(std::initializer_list<uint16_t> list) const;
  std::unique_ptr<std::list<Entry>> _get_spectrum(std::initializer_list<Pair> list);
  void _add_bulk(const Entry&) override;
  void _set_detectors(const std::vector<Gamma::Detector>& dets) override;

  //event processing
  void addEvent(const Event&) override;
  virtual void addHit(const Hit&);
  void addStats(const StatsUpdate&) override;

  std::string _channels_to_xml() const override;
  uint16_t _channels_from_xml(const std::string&) override;

  bool channels_from_string(std::istream &data_stream, bool compression);

  std::vector<PreciseFloat> spectrum_;
  std::vector<double> seconds_;
  std::vector<StatsUpdate>  updates_;
};

}}

#endif // SpectrumTime_H
