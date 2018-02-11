// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_PLAN_VIEW_HH
#define SPECTMORPH_MORPH_PLAN_VIEW_HH

#include "smwindow.hh"
#include "smlabel.hh"
#include "smfixedgrid.hh"
#include "smmorphoperatorview.hh"
#include <functional>

namespace SpectMorph
{

struct MorphPlanView : public Widget
{
  MorphPlan *morph_plan;
  bool       need_view_rebuild;

  std::vector<MorphOperatorView *> m_op_views;
public:
  MorphPlanView (Widget *parent, MorphPlan *morph_plan);

  void on_plan_changed();
  void on_need_view_rebuild();
};

}

#endif
