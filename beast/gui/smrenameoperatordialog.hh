/*
 * Copyright (C) 2011 Stefan Westerfeld
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SPECTMORPH_RENAME_OPERATOR_DIALOG_HH
#define SPECTMORPH_RENAME_OPERATOR_DIALOG_HH

#include <gtkmm.h>
#include "smmorphoperator.hh"

namespace SpectMorph
{

class RenameOperatorDialog : public Gtk::Dialog
{
protected:
  Gtk::Label old_label;
  Gtk::Label old_name_label;
  Gtk::Label new_label;
  Gtk::Entry new_name_entry;
  Gtk::Table table;

  Gtk::Button *ok_button;

  MorphOperator *op;

  void on_name_changed();

public:
  RenameOperatorDialog (MorphOperator *op);

  std::string new_name();
};

}

#endif
