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

#include "smmorphoperatorview.hh"
#include "smmorphoperator.hh"

using namespace SpectMorph;

MorphOperatorView::MorphOperatorView (MorphOperator *op, MainWindow *main_window) :
  main_window (main_window),
  op (op)
{
  on_operators_changed();

  op->morph_plan()->signal_plan_changed.connect (sigc::mem_fun (*this, &MorphOperatorView::on_operators_changed));
  add (frame);
}

bool
MorphOperatorView::on_button_press_event (GdkEventButton *event)
{
  if ((event->type == GDK_BUTTON_PRESS) && (event->button == 3))
    {
      main_window->show_popup (event, op);
      return true; // it has been handled
    }
  else
    return false;
}

void
MorphOperatorView::on_operators_changed()
{
  frame.set_label (op->name());
}
