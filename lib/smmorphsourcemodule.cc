// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphsourcemodule.hh"
#include "smmorphsource.hh"
#include "smmorphplan.hh"
#include "smwavsetrepo.hh"
#include "smleakdebugger.hh"
#include <glib.h>

using namespace SpectMorph;

using std::string;
using std::vector;
using std::max;

static LeakDebugger leak_debugger ("SpectMorph::MorphSourceModule");

static float
freq_to_note (float freq)
{
  return 69 + 12 * log (freq / 440) / log (2);
}

MorphSourceModule::MorphSourceModule (MorphPlanVoice *voice) :
  MorphOperatorModule (voice, 0)
{
  my_source.wav_set      = NULL;
  my_source.active_audio = NULL;

  leak_debugger.add (this);
}

MorphSourceModule::~MorphSourceModule()
{
  leak_debugger.del (this);
}

LiveDecoderSource *
MorphSourceModule::source()
{
  return &my_source;
}

void
MorphSourceModule::set_config (MorphOperator *op)
{
  MorphSource *source = dynamic_cast<MorphSource *> (op);
  string smset = source->smset();
  string smset_dir = source->morph_plan()->index()->smset_dir();
  string path = smset_dir + "/" + smset;

  my_source.wav_set = WavSetRepo::the()->get (path);
}

float
MorphSourceModule::latency_ms()
{
  float max_start_ms = 0;
  if (my_source.wav_set)
    {
      for (vector<WavSetWave>::iterator wi = my_source.wav_set->waves.begin(); wi != my_source.wav_set->waves.end(); wi++)
        {
          max_start_ms = max (wi->audio->start_ms, max_start_ms);
        }
    }
  return max_start_ms;
}

void
MorphSourceModule::MySource::retrigger (int channel, float freq, int midi_velocity, float mix_freq)
{
  Audio *best_audio = NULL;
  float  best_diff  = 1e10;

  if (wav_set)
    {
      float note = freq_to_note (freq);
      for (vector<WavSetWave>::iterator wi = wav_set->waves.begin(); wi != wav_set->waves.end(); wi++)
        {
          Audio *audio = wi->audio;
          if (audio && wi->channel == channel &&
                       wi->velocity_range_min <= midi_velocity &&
                       wi->velocity_range_max >= midi_velocity)
            {
              float audio_note = freq_to_note (audio->fundamental_freq);
              if (fabs (audio_note - note) < best_diff)
                {
                  best_diff = fabs (audio_note - note);
                  best_audio = audio;
                }
            }
        }
    }
  active_audio = best_audio;
}

Audio*
MorphSourceModule::MySource::audio()
{
  return active_audio;
}

AudioBlock *
MorphSourceModule::MySource::audio_block (size_t index)
{
  if (active_audio && index < active_audio->contents.size())
    return &active_audio->contents[index];
  else
    return NULL;
}
