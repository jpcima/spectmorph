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

#include "smspectrumview.hh"
#include "smnavigator.hh"
#include "smlpc.hh"

using namespace SpectMorph;

using std::vector;
using std::max;

SpectrumView::SpectrumView (Navigator *navigator) :
  navigator (navigator)
{
  time_freq_view_ptr = NULL;
  hzoom = 1;
  vzoom = 1;
}

static float
value_scale (float value)
{
  double db = value;
  if (db > -96)
    return db + 96;
  else
    return 0;
}

bool
SpectrumView::on_expose_event (GdkEventExpose* ev)
{
  const int width =  800 * hzoom;
  const int height = 600 * vzoom;
  set_size_request (width, height);

  Glib::RefPtr<Gdk::Window> window = get_window();
  if (window)
  {
    float max_value = 0;
    for (vector<float>::const_iterator mi = spectrum.mags.begin(); mi != spectrum.mags.end(); mi++)
      {
        max_value = max (max_value, value_scale (*mi));
      }


    Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();

    cr->save();
    cr->set_source_rgb (1.0, 1.0, 1.0);   // white
    cr->paint();
    cr->restore();

    cr->set_line_width (2.0);

    // clip to the area indicated by the expose event so that we only redraw
    // the portion of the window that needs to be redrawn
    cr->rectangle (ev->area.x, ev->area.y, ev->area.width, ev->area.height);
    cr->clip();

    cr->set_source_rgb (0.0, 0.0, 0.8);
    if (time_freq_view_ptr->show_frequency_grid())
      {
        double fundamental_freq = time_freq_view_ptr->fundamental_freq();
        double mix_freq = time_freq_view_ptr->mix_freq();

        double pos;
        int partial = 1;
        do
          {
            pos = partial * fundamental_freq / (mix_freq / 2);
            partial++;

            cr->move_to (pos * width, 0);
            cr->line_to (pos * width, height);
          }
        while (pos < 1);
      }
    cr->stroke();

    // draw red lines out from the center of the window
    cr->set_source_rgb (0.8, 0.0, 0.0);
    for (size_t i = 0; i < spectrum.mags.size(); i++)
      {
        cr->line_to (double (i) / spectrum.mags.size() * width, height - value_scale (spectrum.mags[i]) / max_value * height);
      }
    cr->stroke();

    // draw lpc envelope
    if (audio_block.lpc_lsf_p.size() > 10 && navigator->display_param_window()->show_lpc())
      {
        LPC::LSFEnvelope env;
        env.init (audio_block.lpc_lsf_p, audio_block.lpc_lsf_q);

        cr->set_source_rgb (0.0, 0.8, 0.0);
        double max_lpc_value = 0;
        for (float freq = 0; freq < M_PI; freq += 0.001)
          {
            double value = env.eval (freq);
            double value_db = bse_db_from_factor (value, -200);
            max_lpc_value = max (max_lpc_value, value_db);
          }
        for (float freq = 0; freq < M_PI; freq += 0.001)
          {
            double value = env.eval (freq);
            double value_db = bse_db_from_factor (value, -200) - max_lpc_value + max_value;
            cr->line_to (freq / M_PI * width, height - value_db / max_value * height);
          }
        cr->stroke();
      }
    // draw lsf parameters
    if (audio_block.lpc_lsf_p.size() > 10 && navigator->display_param_window()->show_lsf())
      {
        cr->set_source_rgb (0.0, 0.0, 0.3);
        for (size_t i = 0; i < audio_block.lpc_lsf_p.size(); i++)
          {
            cr->move_to (audio_block.lpc_lsf_p[i] / M_PI * width, 0);
            cr->line_to (audio_block.lpc_lsf_p[i] / M_PI * width, height);
          }
        cr->stroke();
        cr->set_source_rgb (0.0, 0.0, 0.8);
        for (size_t i = 0; i < audio_block.lpc_lsf_q.size(); i++)
          {
            cr->move_to (audio_block.lpc_lsf_q[i] / M_PI * width, 0);
            cr->line_to (audio_block.lpc_lsf_q[i] / M_PI * width, height);
          }
        cr->stroke();
      }
  }
  return true;
}

void
SpectrumView::set_spectrum_model (TimeFreqView& tfview)
{
  tfview.signal_spectrum_changed.connect (sigc::mem_fun (*this, &SpectrumView::on_spectrum_changed));
  time_freq_view_ptr = &tfview;
}

void
SpectrumView::on_spectrum_changed()
{
  spectrum = time_freq_view_ptr->get_spectrum();
  audio_block = AudioBlock(); // reset

  Audio *audio = time_freq_view_ptr->audio();
  if (audio)
    {
      int frame = time_freq_view_ptr->position_frac() * audio->contents.size();
      int frame_count = audio->contents.size();

      if (frame >= 0 && frame < frame_count)
        {
          audio_block = audio->contents[frame];
        }
    }

  force_redraw();
}

void
SpectrumView::set_zoom (double new_hzoom, double new_vzoom)
{
  hzoom = new_hzoom;
  vzoom = new_vzoom;

  force_redraw();
}
  
void
SpectrumView::force_redraw()
{
  Glib::RefPtr<Gdk::Window> win = get_window();
  if (win)
    {
      Gdk::Rectangle r (0, 0, get_allocation().get_width(), get_allocation().get_height());
      win->invalidate_rect (r, false);
    }
}

void
SpectrumView::on_display_params_changed()
{
  force_redraw();
}
