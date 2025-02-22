// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smconfig.hh"
#include "smmicroconf.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

string
Config::get_config_filename()
{
  return sm_get_user_dir (USER_DIR_DATA) + "/config";
}

Config::Config()
{
  MicroConf cfg_parser (get_config_filename());

  if (!cfg_parser.open_ok())
    return;

  while (cfg_parser.next())
    {
      int i;
      std::string s;

      if (cfg_parser.command ("zoom", i))
        {
          m_zoom = i;
        }
      else if (cfg_parser.command ("debug", s))
        {
          m_debug.push_back (s);
        }
      else
        {
          //cfg.die_if_unknown();
        }
    }
}

int
Config::zoom() const
{
  return m_zoom;
}

void
Config::set_zoom (int z)
{
  m_zoom = z;
}

vector<string>
Config::debug()
{
  return m_debug;
}

void
Config::store()
{
  FILE *file = fopen (get_config_filename().c_str(), "w");

  if (!file)
    return;

  fprintf (file, "# this file is automatically updated by SpectMorph\n");
  fprintf (file, "# it can be manually edited, however, if you do that, be careful\n");
  fprintf (file, "zoom %d\n", m_zoom);

  for (auto area : m_debug)
    fprintf (file, "debug %s\n", area.c_str());

  fclose (file);
}
