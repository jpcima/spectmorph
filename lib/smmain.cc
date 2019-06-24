// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmain.hh"
#include "smfft.hh"
#include "smmath.hh"
#include "smutils.hh"
#include "smconfig.hh"
#include "sminstenccache.hh"
#include "smwavsetrepo.hh"
#include "config.h"
#include <stdio.h>
#include <assert.h>
#include <locale.h>

#if SPECTMORPH_HAVE_BSE
#include <bse/bsemain.hh>
#endif

using std::string;

namespace SpectMorph
{

float *int_sincos_table;
InstEncCache *singleton_inst_enc_cache = nullptr;
WavSetRepo   *singleton_wav_set_repo = nullptr;

static bool use_sse = true;

void
sm_enable_sse (bool sse)
{
  use_sse = sse;
}

bool
sm_sse()
{
  return use_sse;
}

static bool sm_init_done_flag = false;

bool
sm_init_done()
{
  return sm_init_done_flag;
}

void
sm_init_plugin()
{
  assert (sm_init_done_flag == false);

  /* ensure that user data dir exists */
  string user_data_dir = sm_get_user_dir (USER_DIR_DATA);
  g_mkdir_with_parents (user_data_dir.c_str(), 0775);

  /* ensure that cache dir exists */
  string cache_dir = sm_get_user_dir (USER_DIR_CACHE);
  g_mkdir_with_parents (cache_dir.c_str(), 0775);

  Config cfg;

  for (auto area : cfg.debug())
    Debug::enable (area);

  FFT::load_wisdom();
  int_sincos_init();
  sm_math_init();

  singleton_inst_enc_cache = new InstEncCache();
  singleton_wav_set_repo = new WavSetRepo();
  sm_init_done_flag = true;
}

void
sm_cleanup_plugin()
{
  assert (sm_init_done_flag == true);

  delete singleton_inst_enc_cache;
  singleton_inst_enc_cache = nullptr;

  delete singleton_wav_set_repo;
  singleton_wav_set_repo = nullptr;

  sm_init_done_flag = false;
}

Main::Main (int *argc_p, char ***argv_p)
{
  /* internationalized string printf */
  setlocale (LC_ALL, "");
#if SPECTMORPH_HAVE_BSE
  bse_init_inprocess (argc_p, *argv_p, NULL);
#endif
  sm_init_plugin();
}

Main::~Main()
{
  sm_cleanup_plugin();
}

InstEncCache *
Global::inst_enc_cache()
{
  return singleton_inst_enc_cache;
}

WavSetRepo *
Global::wav_set_repo()
{
  return singleton_wav_set_repo;
}

}
