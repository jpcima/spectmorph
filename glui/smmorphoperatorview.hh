// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_OPERATOR_VIEW_HH
#define SPECTMORPH_MORPH_OPERATOR_VIEW_HH

#include "smwindow.hh"
#include "smlabel.hh"
#include "smfixedgrid.hh"
#include "smframe.hh"
#include "smslider.hh"
#include "smmorphplan.hh"
#include "smmorphplanwindow.hh"
#include <functional>

namespace SpectMorph
{

class MorphPlanWindow;

struct MorphOperatorView : public Frame
{
public:
  MorphOperatorView (Widget *parent, MorphOperator *op, MorphPlanWindow *window) :
    Frame (parent)
  {
    FixedGrid grid;

    // FIXME: need update (signal) on_operators_changed
    std::string title = op->type_name() + ": " + op->name();

    Label *label = new Label (this, title);
    label->align = TextAlign::CENTER;
    label->bold  = true;
    grid.add_widget (label, 0, 0, 43, 4);
  }
  virtual double
  view_height()
  {
    return 4;
  }
};

}

#endif
