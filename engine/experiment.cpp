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
 *      Types for organizing data aquired from Device
 *        Qpx::Hit        single energy event with coincidence flags
 *
 ******************************************************************************/

#include "experiment.h"
#include "custom_logger.h"
#include "UncertainDouble.h"

namespace Qpx {

void ExperimentProject::to_xml(pugi::xml_node &root) const
{
  pugi::xml_node node = root.append_child(this->xml_element_name().c_str());

//  if (base_prototypes.empty())
//    return;

  base_prototypes.to_xml(node);
  if (root_trajectory)
    root_trajectory->to_xml(node);

  if (!data.empty())
  {
    pugi::xml_node datanode = node.append_child("Data");

    for (auto &a : data)
    {
      pugi::xml_node projnode  = datanode.append_child(a.second->xml_element_name().c_str());
      projnode.append_attribute("idx").set_value((long long)a.first);
      a.second->to_xml(projnode);
    }
  }
}


void ExperimentProject::from_xml(const pugi::xml_node &node)
{
  base_prototypes.clear();
  if (node.child(base_prototypes.xml_element_name().c_str()))
    base_prototypes.from_xml(node.child(base_prototypes.xml_element_name().c_str()));

  if (node.child(TrajectoryNode().xml_element_name().c_str()))
  {
    root_trajectory = std::shared_ptr<Qpx::TrajectoryNode>(new Qpx::TrajectoryNode());
    root_trajectory->from_xml(node.child(TrajectoryNode().xml_element_name().c_str()));
  }

  next_idx = 1;

  if (node.child("Data"))
  {
    for (auto n : node.child("Data").children())
    {
      int64_t idx = -1;
      if (n.attribute("idx"))
        idx = n.attribute("idx").as_llong();
      if (idx >= 0)
      {
        ProjectPtr proj = ProjectPtr(new Project());
        proj->from_xml(n, true, true);
        data[idx] = proj;
        next_idx = idx + 1;
      }
    }
  }

  std::list<TrajectoryPtr> leafs;
  find_leafs(leafs, root_trajectory);
  for (auto &l : leafs)
  {
    if (!data.count(l->data_idx))
      continue;

    DataPoint dp;
    dp.node = l;
    dp.idx_proj = l->data_idx;
    gather_vars_recursive(dp, l);
    for (auto &s : data[l->data_idx]->get_sinks())
      if (s.second) {
        dp.idx_sink = s.first;
        dp.spectrum_info = s.second->metadata();
        results_.push_back(dp);
      }
  }

  gather_results();
}

void ExperimentProject::find_leafs(std::list<TrajectoryPtr> &list, TrajectoryPtr node)
{
  if (!node)
    return;
  if ((node->data_idx >= 0) && !node->childCount())
    list.push_back(node);
  else
    for (int i = 0; i < node->childCount(); i++)
      find_leafs(list, node->getChild(i));
}


std::pair<DomainType, TrajectoryPtr> ExperimentProject::next_setting()
{
  std::pair<DomainType, TrajectoryPtr> ret(none, nullptr);
  if (root_trajectory)
    ret = root_trajectory->next_setting();
  if (ret.second && !ret.second->childCount() && (ret.second->domain.type == none))
  {
//    DBG << "Experiment generating new data point with " << base_prototypes.size() << " sinks";

    XMLableDB<Qpx::Metadata> prototypes = base_prototypes;
    set_sink_vars_recursive(prototypes, ret.second);

    data[next_idx] = ProjectPtr(new Project());
    data[next_idx]->set_prototypes(prototypes);
    ret.second->data_idx = next_idx;

    DataPoint dp;
    dp.node = ret.second;
    dp.idx_proj = next_idx;
    gather_vars_recursive(dp, ret.second);
    for (auto &s : data[next_idx]->get_sinks())
      if (s.second) {
        dp.idx_sink = s.first;
        dp.spectrum_info = s.second->metadata();
        results_.push_back(dp);
      }

    next_idx++;

  }
  return ret;
}

void ExperimentProject::set_sink_vars_recursive(XMLableDB<Qpx::Metadata>& prototypes,
                                                TrajectoryPtr node)
{
  if (!node)
    return;
  TrajectoryPtr parent = node->getParent();
  if (!parent)
    return;
  set_sink_vars_recursive(prototypes, parent);

  if (parent->domain.type == Qpx::DomainType::sink)
  {
    for (auto &p : prototypes.my_data_)
    {
      if (p.attributes.has(node->domain_value, Qpx::Match::id | Qpx::Match::indices))
      {
//        DBG << "Sink prototype " << p.name << " " << p.type() << " " << " has "
//            << node->domain_value.id_;
        p.attributes.set_setting_r(node->domain_value, Qpx::Match::id | Qpx::Match::indices);
      }
    }
  }
}

void ExperimentProject::gather_vars_recursive(DataPoint& dp, TrajectoryPtr node)
{
  if (!node)
    return;
  TrajectoryPtr parent = node->getParent();
  if (!parent)
    return;
  gather_vars_recursive(dp, parent);

//  DBG << "checking domain " << parent->domain.verbose << " " << parent->to_string()
//      << " child " << node->to_string();

  if (parent->domain.type == Qpx::DomainType::sink) {
//    DBG << "a";
    if (dp.spectrum_info.attributes.has(node->domain_value, Qpx::Match::id | Qpx::Match::indices))
    {
//      DBG << "b";

      dp.domains[parent->domain.verbose] = node->domain_value;
    }
  }
  else if (parent->domain.type != Qpx::DomainType::none) {
//    DBG << "c";
    dp.domains[parent->domain.verbose] = node->domain_value;
  }
//  else
//    DBG << "d";

}


ProjectPtr ExperimentProject::get_data(int64_t i) const
{
  if (data.count(i))
    return data.at(i);
  else
    return nullptr;
}

void ExperimentProject::delete_data(int64_t i)
{
  if (data.count(i))
  {
    data.erase(i);
    DBG << "deleted data " << i;
  }
}

void ExperimentProject::gather_results()
{
  for (auto &r : results_)
  {
    r.domains.clear();
    if (r.node)
      gather_vars_recursive(r, r.node);
//    DBG << "domains found " << r.domains.size();
    if (data.count(r.idx_proj) && data.at(r.idx_proj))
    {
      if (data.at(r.idx_proj)->has_fitter(r.idx_sink))
      {
        Fitter f = data.at(r.idx_proj)->get_fitter(r.idx_sink);
        r.spectrum_info = f.metadata_;
        std::set<double> sel = f.get_selected_peaks();
        if (sel.size() && f.contains_peak(*sel.begin()))
          r.selected_peak = f.peak(*sel.begin());
      }
    }
  }
}



}