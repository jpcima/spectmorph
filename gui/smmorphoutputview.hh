// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_OUTPUT_VIEW_HH
#define SPECTMORPH_MORPH_OUTPUT_VIEW_HH

#include "smmorphoperatorview.hh"
#include "smmorphoutput.hh"
#include "smcomboboxoperator.hh"

#include <QComboBox>
#include <QCheckBox>

namespace SpectMorph
{

class MorphOutputView : public MorphOperatorView
{
  Q_OBJECT

  struct ChannelView {
    QLabel           *label;
    ComboBoxOperator *combobox;
  };
  std::vector<ChannelView *>  channels;
  MorphOutput                *morph_output;
public:
  MorphOutputView (MorphOutput *morph_morph_output, MorphPlanWindow *morph_plan_window);

public slots:
  void on_sines_changed (bool new_value);
  void on_noise_changed (bool new_value);
  void on_operator_changed();
};

}

#endif
