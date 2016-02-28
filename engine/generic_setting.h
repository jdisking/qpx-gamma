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
 *      Setting exactly what it says
 *
 ******************************************************************************/

#ifndef GENERIC_SETTING
#define GENERIC_SETTING

#include <vector>
#include <string>
#include <map>
#include <set>
#include <boost/date_time.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include "pattern.h"

#include "pugixml.hpp"
#include "xmlable.h"

#define QPX_FLOAT_PRECISION 16

typedef boost::multiprecision::number<boost::multiprecision::cpp_dec_float<QPX_FLOAT_PRECISION> > PreciseFloat;


namespace Qpx {

enum class SettingType : int {none,
                              stem,
                              boolean,
                              integer,
                              floating,
                              floating_precise,
                              text,
                              int_menu,
                              detector, // does not scale
                              time,
                              time_duration,
                              pattern,
                              file_path,
                              dir_path,
                              binary,
                              command,
                              indicator
                             };

enum Match {
  id      = 1 << 0,
  name    = 1 << 1,
  address = 1 << 2,
  indices = 1 << 3
};


inline Match operator|(Match a, Match b) {return static_cast<Match>(static_cast<int>(a) | static_cast<int>(b));}
inline Match operator&(Match a, Match b) {return static_cast<Match>(static_cast<int>(a) & static_cast<int>(b));}


SettingType to_type(const std::string &type);
std::string to_string(SettingType);

struct SettingMeta : public XMLable {

  std::string        id_;
  SettingType        setting_type;

  bool               writable;
  bool               visible;
  bool               saveworthy;
  int64_t            address;
  std::string        name, description;
  double             minimum, maximum, step;
  std::string        unit;                       //or extension if file
  std::map<int32_t, std::string> int_menu_items; //or intrinsic branches
  std::set<std::string> flags;
  int16_t            max_indices;


  SettingMeta(const pugi::xml_node &node) : SettingMeta() {this->from_xml(node);}
  SettingMeta() : setting_type(SettingType::none), writable(false), visible(true), saveworthy(true),
    address(-1), minimum(0), maximum(0), step(0), max_indices(0) {}

  std::string xml_element_name() const {return "SettingMeta";}

  bool shallow_equals(const SettingMeta& other) const {
    return (id_ == other.id_);
  }

  bool operator!= (const SettingMeta& other) const {return !operator==(other);}
  bool operator== (const SettingMeta& other) const {
    if (id_ != other.id_) return false;
    if (name != other.name) return false;
    if (unit != other.unit) return false;
    if (minimum != other.minimum) return false;
    if (maximum != other.maximum) return false;
    if (step != other.step) return false;
    if (writable != other.writable) return false;
    if (description != other.description) return false;
    if (address != other.address) return false;
    if (int_menu_items != other.int_menu_items) return false;
    if (flags != other.flags) return false;
    return true;
  }

  SettingMeta stripped() const {
    SettingMeta s;
    s.id_ == id_;
    s.setting_type = setting_type;
    return s;
  }

  bool meaningful() const {
    return (operator!=(this->stripped()));
  }

  void from_xml(const pugi::xml_node &node);
  void to_xml(pugi::xml_node &node) const;

  void populate_menu(const pugi::xml_node &node,
                     const std::string &key_name,
                     const std::string &value_name);
  void menu_to_node(pugi::xml_node &node, const std::string &element_name,
                    const std::string &key_name, const std::string &value_name) const;

};


struct Setting;

struct Setting : public XMLable {
  std::string       id_;

  std::set<int32_t> indices;

  int64_t                          value_int;
  std::string                      value_text;
  double                           value_dbl;
  boost::posix_time::ptime         value_time;
  boost::posix_time::time_duration value_duration;
  PreciseFloat                     value_precise;
  Pattern                          value_pattern;


  XMLableDB<Setting> branches;

  SettingMeta        metadata;

  
  Setting() : branches("branches"), value_dbl(0.0), value_int(0) {}
  
  Setting(const pugi::xml_node &node) : Setting() {this->from_xml(node);}
  
  Setting(std::string id) : Setting() {id_ = id;} //BAD
  Setting(SettingMeta meta) : Setting() {
    id_ = meta.id_;
    metadata = meta;
  }

  std::string xml_element_name() const {return "Setting";}

  bool shallow_equals(const Setting& other) const {
    return (id_ == other.id_);
  }
  bool compare(const Setting &other, Match flags) const;

  bool operator!= (const Setting& other) const {return !operator==(other);}
  bool operator== (const Setting& other) const {
    if (id_            != other.id_) return false;
    if (indices        != other.indices) return false;
    if (value_dbl      != other.value_dbl) return false;
    if (value_int      != other.value_int) return false;
    if (value_text     != other.value_text) return false;
    if (value_time     != other.value_time) return false;
    if (value_duration != other.value_duration) return false;
    if (value_precise  != other.value_precise) return false;
    if (value_pattern  != other.value_pattern) return false;
    if (branches       != other.branches) return false;
//    if (metadata != other.metadata) return false;
    return true;
  }

  std::string val_to_string() const;
  std::string val_to_pretty_string() const;
  void val_from_node(const pugi::xml_node &node);

  friend std::ostream & operator << (std::ostream &os, Setting s) {
    os << s.id_;
    if (s.indices.size()) {
      os << " { ";
      for (auto &q : s.indices)
        os << q << " ";
      os << "}";
    }
    os << " = " << s.val_to_string() << std::endl;
    for (auto &q : s.branches.my_data_)
      os << "_" << q;
    return os;
  }
  
  void from_xml(const pugi::xml_node &node) override;
  void to_xml(pugi::xml_node &node, bool with_metadata) const;
  void to_xml(pugi::xml_node &node) const override {this->to_xml(node, false);}

  Setting get_setting(Setting address, Match flags) const;
  bool retrieve_one_setting(Setting&, const Setting&, Match flags) const;
  bool push_one_setting(const Setting &setting, Setting& root, Match flags);

  void del_setting(Setting address, Match flags);
  void delete_one_setting(const Setting&, Setting&, Match flags);

  void condense();
  void cull_invisible();
  void cull_readonly();
  void strip_metadata();
  void enrich(const std::map<std::string, SettingMeta> &, bool impose_limits = false);

};

}

#endif
