/* 
 * Copyright (C) 2009 Stefan Westerfeld
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "frame.hh"

using Stw::Codec::Frame;

Frame::Frame (size_t frame_size)
  : frame_size (frame_size)
{
  decoded_residue.resize (frame_size);
  decoded_sines.resize (frame_size);
  noise_envelope.resize (256);
}

Frame::Frame (Stw::Codec::AudioBlockHandle audio_block, size_t frame_size) :
    frame_size (frame_size),
    freqs (audio_block->freqs.begin(), audio_block->freqs.end()),
    phases (audio_block->phases.begin(), audio_block->phases.end()),
    noise_envelope (audio_block->meaning.begin(), audio_block->meaning.end())
{
  decoded_residue.resize (frame_size);
  decoded_sines.resize (frame_size);
}
