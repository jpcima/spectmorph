// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_SYNTH_INTERFACE_HH
#define SPECTMORPH_SYNTH_INTERFACE_HH

#include "smmidisynth.hh"
#include "smproject.hh"
namespace SpectMorph
{

class SynthInterface
{
  Project *m_project = nullptr;
public:
  SynthInterface (Project *project)
  {
    m_project = project;
  }
  Project *
  get_project()
  {
    return m_project;
  }
  template<class DATA>
  void
  send_control_event (const std::function<void(Project *)>& func, DATA *data = nullptr)
  {
    m_project->synth_take_control_event (new InstFunc (func, [data]() { delete data;}));
  }
  void
  send_control_event (const std::function<void(Project *)>& func)
  {
    m_project->synth_take_control_event (new InstFunc (func, []() {}));
  }
  void
  synth_inst_edit_update (bool active, WavSet *take_wav_set, WavSet *take_ref_wav_set)
  {
    /* ownership of take_wav_set is transferred to the event */
    struct EventData
    {
      std::unique_ptr<WavSet> wav_set;
      std::unique_ptr<WavSet> ref_wav_set;
    } *event_data = new EventData;

    event_data->wav_set.reset (take_wav_set);
    event_data->ref_wav_set.reset (take_ref_wav_set);

    send_control_event (
      [=] (Project *project)
        {
          project->midi_synth()->set_inst_edit (active);

          if (active)
            project->midi_synth()->inst_edit_synth()->take_wav_sets (event_data->wav_set.release(), event_data->ref_wav_set.release());
        },
      event_data);
  }
  void
  synth_inst_edit_note (int note, bool on, unsigned int layer)
  {
    send_control_event (
      [=] (Project *project)
        {
          unsigned char event[3];

          event[0] = on ? 0x90 : 0x80;
          event[1] = note;
          event[2] = on ? 100 : 0;

          project->midi_synth()->inst_edit_synth()->handle_midi_event (event, layer);
        });
  }
  void
  emit_update_plan (MorphPlanPtr new_plan)
  {
    /* ownership of plan is transferred to the event */
    struct EventData
    {
      MorphPlanPtr plan;
    } *event_data = new EventData;

    // FIXME: refptr is locking (which is not too good)
    event_data->plan = new_plan;

    send_control_event (
      [=] (Project *project)
        {
          project->midi_synth()->update_plan (event_data->plan);
        },
      event_data);
  }
  void
  emit_update_gain (double gain)
  {
    send_control_event (
      [=] (Project *project)
        {
          project->midi_synth()->set_gain (gain);
        });
  }
  void
  emit_add_rebuild_result (int object_id, WavSet *take_wav_set)
  {
    /* ownership of take_wav_set is transferred to the event */
    struct EventData
    {
      std::unique_ptr<WavSet> wav_set;
    } *event_data = new EventData;

    event_data->wav_set.reset (take_wav_set);

    send_control_event (
      [=] (Project *project)
        {
          project->add_rebuild_result (object_id, event_data->wav_set.release());
        },
        event_data);
  }
  void
  emit_clear_wav_sets()
  {
    send_control_event (
      [=] (Project *project)
        {
          project->clear_wav_sets();
        });
  }
  Signal<SynthNotifyEvent *> signal_notify_event;
};

}

#endif
