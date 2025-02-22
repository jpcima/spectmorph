// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphoutputmodule.hh"
#include "smmorphoutput.hh"
#include "smmorphplan.hh"
#include "smleakdebugger.hh"
#include <glib.h>
#include <assert.h>

#define CHANNEL_OP_COUNT 4

using namespace SpectMorph;

using std::string;
using std::vector;

static LeakDebugger leak_debugger ("SpectMorph::MorphOutputModule");

MorphOutputModule::MorphOutputModule (MorphPlanVoice *voice) :
  MorphOperatorModule (voice)
{
  out_ops.resize (CHANNEL_OP_COUNT);
  out_decoders.resize (CHANNEL_OP_COUNT);
  m_portamento = false;

  leak_debugger.add (this);
}

MorphOutputModule::~MorphOutputModule()
{
  for (size_t ch = 0; ch < CHANNEL_OP_COUNT; ch++)
    {
      if (out_decoders[ch])
        {
          delete out_decoders[ch];
          out_decoders[ch] = NULL;
        }
    }
  leak_debugger.del (this);
}

void
MorphOutputModule::set_config (MorphOperator *op)
{
  MorphOutput *out_op = dynamic_cast <MorphOutput *> (op);
  g_return_if_fail (out_op != NULL);

  clear_dependencies();
  for (size_t ch = 0; ch < CHANNEL_OP_COUNT; ch++)
    {
      MorphOperatorModule *mod = NULL;
      EffectDecoder *dec = NULL;

      MorphOperator *op = out_op->channel_op (ch);
      if (op)
        mod = morph_plan_voice->module (op);

      if (mod == out_ops[ch]) // same source
        {
          dec = out_decoders[ch];
          // keep decoder as it is
        }
      else
        {
          if (out_decoders[ch])
            delete out_decoders[ch];
          if (mod)
            {
              dec = new EffectDecoder (mod->source());
            }
        }

      if (dec)
        dec->set_config (out_op, morph_plan_voice->mix_freq());

      m_portamento            = out_op->portamento();
      m_portamento_glide      = out_op->portamento_glide();
      m_velocity_sensitivity  = out_op->velocity_sensitivity();

      out_ops[ch] = mod;
      out_decoders[ch] = dec;

      add_dependency (mod);
    }
}

bool
MorphOutputModule::portamento() const
{
  return m_portamento;
}

float
MorphOutputModule::portamento_glide() const
{
  return m_portamento_glide;
}

float
MorphOutputModule::velocity_sensitivity() const
{
  return m_velocity_sensitivity;
}

static void
recursive_reset_tag (MorphOperatorModule *module)
{
  if (!module)
    return;

  const vector<MorphOperatorModule *>& deps = module->dependencies();
  for (size_t i = 0; i < deps.size(); i++)
    recursive_reset_tag (deps[i]);

  module->update_value_tag() = 0;
}

static void
recursive_update_value (MorphOperatorModule *module, double time_ms)
{
  if (!module)
    return;

  const vector<MorphOperatorModule *>& deps = module->dependencies();
  for (size_t i = 0; i < deps.size(); i++)
    recursive_update_value (deps[i], time_ms);

  if (!module->update_value_tag())
    {
      module->update_value (time_ms);
      module->update_value_tag()++;
    }
}

static void
recursive_reset_value (MorphOperatorModule *module)
{
  if (!module)
    return;

  const vector<MorphOperatorModule *>& deps = module->dependencies();
  for (size_t i = 0; i < deps.size(); i++)
    recursive_reset_value (deps[i]);

  if (!module->update_value_tag())
    {
      module->reset_value();
      module->update_value_tag()++;
    }
}

static bool
recursive_cycle_check (MorphOperatorModule *module, int depth = 0)
{
  /* check if processing would fail due to cycles
   *
   * this check should avoid crashes in this situation, although no audio will be produced
   */
  if (depth > 500)
    return true;

  for (auto mod : module->dependencies())
    if (recursive_cycle_check (mod, depth + 1))
      return true;

  return false;
}

void
MorphOutputModule::process (size_t n_samples, float **values, size_t n_ports, const float *freq_in)
{
  g_return_if_fail (n_ports <= out_decoders.size());

  const bool have_cycle = recursive_cycle_check (this);

  for (size_t port = 0; port < n_ports; port++)
    {
      if (values[port])
        {
          if (out_decoders[port] && !have_cycle)
            {
              out_decoders[port]->process (n_samples, freq_in, values[port]);
            }
          else
            {
              zero_float_block (n_samples, values[port]);
            }
        }
    }

  if (!have_cycle)
    {
      recursive_reset_tag (this);
      recursive_update_value (this, n_samples / morph_plan_voice->mix_freq() * 1000);
    }
}

void
MorphOutputModule::retrigger (int channel, float freq, int midi_velocity)
{
  if (recursive_cycle_check (this))
    return;

  for (size_t port = 0; port < CHANNEL_OP_COUNT; port++)
    {
      if (out_decoders[port])
        {
          out_decoders[port]->retrigger (channel, freq, midi_velocity, morph_plan_voice->mix_freq());
        }
    }
  recursive_reset_tag (this);
  recursive_reset_value (this);
}

void
MorphOutputModule::release()
{
  for (auto dec : out_decoders)
    {
      if (dec)
        dec->release();
    }
}

bool
MorphOutputModule::done()
{
  // done means: the signal will be only zeros from here
  bool done = true;

  for (auto dec : out_decoders)
    {
      // we're done if all decoders are done
      if (dec)
        done = done && dec->done();
    }
  return done;
}
