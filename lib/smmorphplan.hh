// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_PLAN_HH
#define SPECTMORPH_MORPH_PLAN_HH

#include "smindex.hh"
#include "smmorphoperator.hh"
#include "smobject.hh"
#include "smaudio.hh"

namespace SpectMorph
{

class MorphPlan : public Object
{
  Q_OBJECT
  Index                        m_index;
  std::vector<MorphOperator *> m_operators;

  std::string                  index_filename;
  bool                         in_restore;

  void clear();

public:
  MorphPlan();
  ~MorphPlan();

  class ExtraParameters
  {
  public:
    virtual std::string   section() = 0;
    virtual void          save (OutFile& out_file) = 0;
    virtual void          handle_event (InFile& ifile) = 0;
  };

  bool         load_index (const std::string& filename);
  const Index *index();

  enum AddPos {
    ADD_POS_AUTO,
    ADD_POS_END
  };

  void add_operator (MorphOperator *op, AddPos = ADD_POS_END, const std::string& name = "", const std::string& id = "", bool load_folded = false);
  const std::vector<MorphOperator *>& operators();
  void remove (MorphOperator *op);
  void move (MorphOperator *op, MorphOperator *op_next);

  void set_plan_str (const std::string& plan_str);
  void emit_plan_changed();
  void emit_index_changed();

  Bse::Error save (GenericOut *file, ExtraParameters *params = nullptr) const;
  Bse::Error load (GenericIn *in, ExtraParameters *params = nullptr);

  MorphPlan *clone() const; // create a deep copy

  static std::string id_chars();
  static std::string generate_id();

signals:
  void plan_changed();
  void index_changed();
  void operator_removed (MorphOperator *op);
  void need_view_rebuild();
};

typedef RefPtr<MorphPlan> MorphPlanPtr;

}

#endif
