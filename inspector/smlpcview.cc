/*
 * Copyright (C) 2010-2011 Stefan Westerfeld
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

#include "smlpcview.hh"
#include "smlpc.hh"

using namespace SpectMorph;

using std::vector;
using std::complex;
using std::max;

#if 0
LPCView::LPCView()
{
  time_freq_view_ptr = NULL;
  hzoom = 1;
  vzoom = 1;
}

bool
LPCView::on_expose_event (GdkEventExpose* ev)
{
  const int width =  600 * hzoom;
  const int height = 600 * vzoom;
  set_size_request (width, height);

  Glib::RefPtr<Gdk::Window> window = get_window();
  if (window)
    {
      Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();

      cr->save();
      cr->set_source_rgb (1.0, 1.0, 1.0);   // white
      cr->paint();
      cr->restore();

      // clip to the area indicated by the expose event so that we only redraw
      // the portion of the window that needs to be redrawn
      cr->rectangle (ev->area.x, ev->area.y, ev->area.width, ev->area.height);
      cr->clip();

      // draw lpc zeros
      if (audio_block.lpc_lsf_p.size() == 26 && audio_block.lpc_lsf_q.size() == 26)
        {
          vector<double> lpc;
          LPC::lsf2lpc (audio_block.lpc_lsf_p, audio_block.lpc_lsf_q, lpc);

          vector< complex<double> > roots;
          LPC::find_roots (lpc, roots);

          cr->set_source_rgb (0.8, 0.0, 0.0);
          cr->set_line_width (1.0);
          for (size_t i = 0; i < roots.size(); i++)
            {
              double root_x = (roots[i].real() + 1) / 2 * width;
              double root_y = (roots[i].imag() + 1) / 2 * width;

              cr->move_to (root_x - 5, root_y - 5);
              cr->line_to (root_x + 5, root_y + 5);
              cr->move_to (root_x + 5, root_y - 5);
              cr->line_to (root_x - 5, root_y + 5);
            }
          cr->stroke();
          cr->set_source_rgb (0.0, 0.0, 0.8);
          for (double t = 0; t < 2 * M_PI + 0.2; t += 0.1)
            {
              double x = (sin (t) + 1) / 2 * width;
              double y = (cos (t) + 1) / 2 * width;
              cr->line_to (x, y);
            }
          cr->stroke();

#if 0
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
#endif
        }
    }
  return true;
}

void
LPCView::set_lpc_model (TimeFreqView& tfview)
{
  tfview.signal_spectrum_changed.connect (sigc::mem_fun (*this, &LPCView::on_lpc_changed));
  time_freq_view_ptr = &tfview;
}

void
LPCView::on_lpc_changed()
{
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
LPCView::set_zoom (double new_hzoom, double new_vzoom)
{
  hzoom = new_hzoom;
  vzoom = new_vzoom;

  force_redraw();
}
  
void
LPCView::force_redraw()
{
  Glib::RefPtr<Gdk::Window> win = get_window();
  if (win)
    {
      Gdk::Rectangle r (0, 0, get_allocation().get_width(), get_allocation().get_height());
      win->invalidate_rect (r, false);
    }
}
#endif
