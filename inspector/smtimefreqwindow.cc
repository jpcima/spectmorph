/*
 * Copyright (C) 2010 Stefan Westerfeld
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

#include "smtimefreqwindow.hh"
#include "smmath.hh"
#include "smnavigator.hh"

using std::vector;
using std::string;

using namespace SpectMorph;

TimeFreqWindow::TimeFreqWindow (Navigator *navigator) :
  zoom_controller (5000, 10000),
  position_adjustment (0.0, 0.0, 1.0, 0.01, 1.0, 0.0),
  position_scale (position_adjustment),
  min_db_scale (-192, -3, 0.01),
  boost_scale (0, 100, 0.01),
  navigator (navigator)
{
  set_border_width (10);
  set_default_size (800, 600);
  set_title ("Time/Frequency View");
  vbox.pack_start (scrolled_win);

  vbox.pack_start (zoom_controller, Gtk::PACK_SHRINK);
  zoom_controller.signal_zoom_changed.connect (sigc::mem_fun (*this, &TimeFreqWindow::on_zoom_changed));

  vbox.pack_start (position_hbox, Gtk::PACK_SHRINK);
  position_hbox.pack_start (position_scale);
  position_hbox.pack_start (position_label, Gtk::PACK_SHRINK);
  position_scale.set_draw_value (false);
  position_label.set_text ("frame 0");
  position_hbox.set_border_width (10);
  position_scale.signal_value_changed().connect (sigc::mem_fun (*this, &TimeFreqWindow::on_position_changed));

  vbox.pack_start (min_db_hbox, Gtk::PACK_SHRINK);
  min_db_hbox.pack_start (min_db_scale);
  min_db_hbox.pack_start (min_db_label, Gtk::PACK_SHRINK);
  min_db_scale.set_draw_value (false);
  min_db_hbox.set_border_width (10);
  min_db_scale.signal_value_changed().connect (sigc::mem_fun (*this, &TimeFreqWindow::on_display_params_changed));
  min_db_scale.set_value (-96);

  vbox.pack_start (boost_hbox, Gtk::PACK_SHRINK);
  boost_hbox.pack_start (boost_scale);
  boost_hbox.pack_start (boost_label, Gtk::PACK_SHRINK);
  boost_scale.set_draw_value (false);
  boost_hbox.set_border_width (10);
  boost_scale.signal_value_changed().connect (sigc::mem_fun (*this, &TimeFreqWindow::on_display_params_changed));
  boost_scale.set_value (0);

  add (vbox);
  scrolled_win.add (m_time_freq_view);
  show_all_children();

  navigator->fft_param_window()->signal_params_changed.connect (sigc::mem_fun (*this, &TimeFreqWindow::on_dhandle_changed));
  m_time_freq_view.signal_progress_changed.connect (sigc::mem_fun (*this, &TimeFreqWindow::on_progress_changed));
  m_time_freq_view.signal_resized.connect (sigc::mem_fun (*this, &TimeFreqWindow::on_resized));
}

void
TimeFreqWindow::on_zoom_changed()
{
  m_time_freq_view.set_zoom (zoom_controller.get_hzoom(), zoom_controller.get_vzoom());
}

void
TimeFreqWindow::on_position_changed()
{
  int frames = m_time_freq_view.get_frames();
  int position = CLAMP (sm_round_positive (position_adjustment.get_value() * frames), 0, frames - 1);
  char buffer[1024];
  sprintf (buffer, "frame %d", position);
  position_label.set_text (buffer);
  if (navigator->get_show_position())
    m_time_freq_view.set_position (position);
  else
    m_time_freq_view.set_position (-1);
}

void
TimeFreqWindow::on_analysis_changed()
{
  m_time_freq_view.set_show_analysis (navigator->get_show_analysis());
}

void
TimeFreqWindow::on_frequency_grid_changed()
{
  m_time_freq_view.set_show_frequency_grid (navigator->get_show_frequency_grid());
}

void
TimeFreqWindow::on_dhandle_changed()
{
  m_time_freq_view.load (navigator->get_dhandle(), "fn", navigator->get_audio(),
                       navigator->fft_param_window()->get_analysis_params());
}

void
TimeFreqWindow::on_progress_changed()
{
  navigator->fft_param_window()->set_progress (m_time_freq_view.get_progress());
}

void
TimeFreqWindow::on_display_params_changed()
{
  min_db_label.set_text (Birnet::string_printf ("min_db %.2f", min_db_scale.get_value()));
  boost_label.set_text (Birnet::string_printf ("boost %.2f", boost_scale.get_value()));
  m_time_freq_view.set_display_params (min_db_scale.get_value(), boost_scale.get_value());
}

void
TimeFreqWindow::on_resized (int old_width, int old_height, int new_width, int new_height)
{
  if (old_width > 0 && old_height > 0 && new_width > 0 && new_height > 0)
    {
      Gtk::Viewport *view_port = dynamic_cast<Gtk::Viewport*> (scrolled_win.get_child());
      const int h_2 = view_port->get_height() / 2;
      const int w_2 = view_port->get_width() / 2;
      Gtk::Adjustment *vadj = scrolled_win.get_vadjustment();
      Gtk::Adjustment *hadj = scrolled_win.get_hadjustment();

      vadj->set_value ((vadj->get_value() + h_2) / old_height * new_height - h_2);
      hadj->set_value ((hadj->get_value() + w_2) / old_width * new_width - w_2);
    }
}

TimeFreqView*
TimeFreqWindow::time_freq_view()
{
  return &m_time_freq_view;
}
