// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smnativefiledialog.hh"
#include "smwindow.hh"

#include <glib.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/wait.h>

using namespace SpectMorph;

using std::string;
using std::vector;

namespace SpectMorph {

class ExtFileDialog : public NativeFileDialog
{
  int    child_pid = -1;
  int    child_stdout = -1;
  string selected_filename;
  bool   selected_filename_ok = false;

  static string last_start_directory;

public:
  ExtFileDialog (PuglNativeWindow win_id, bool open, const string& title, const FileDialogFormats& formats);

  void process_events();
};

string ExtFileDialog::last_start_directory;

}

NativeFileDialog *
NativeFileDialog::create (PuglNativeWindow win_id, bool open, const string& title, const FileDialogFormats& formats)
{
  return new ExtFileDialog (win_id, open, title, formats);
}

ExtFileDialog::ExtFileDialog (PuglNativeWindow win_id, bool open, const string& title, const FileDialogFormats& formats)
{
  GError *err;

  if (last_start_directory == "")
    last_start_directory = g_get_home_dir();

#if 0
  string filter_spec = filter + "|" + filter_title;
  string attach = string_printf ("%ld", win_id);

  vector<const char *> argv = { "kdialog", open ? "--getopenfilename" : "--getsavefilename", g_get_home_dir(), filter_spec.c_str(), "--title", title.c_str(),
                                           "--attach", attach.c_str(), nullptr };
#endif
  string attach = string_printf ("%" PRIuPTR, win_id);
  string smfiledialog = sm_get_install_dir (INSTALL_DIR_BIN) + "/smfiledialog.sh";

  string filter_spec;
  for (auto format : formats.formats)
    {
      string filter;
      for (auto ext : format.exts)
        {
          if (!filter.empty())
            filter += " ";

          if (ext == "*") /* filter for all should be '*', not '*.*' */
            filter += "*";
          else
            filter += "*." + ext;
        }

      if (!filter_spec.empty())
        filter_spec += "|";
      filter_spec += format.title + "(" + filter + ")";
    }
  /* example filter spec: Supported Audio Files(*.wav *.flac *.ogg *.aiff)|Wav Files(*.wav)|FLAC Files(*.flac)|All Files(*) */
  vector<const char *> argv = { smfiledialog.c_str(), open ? "open" : "save",
                                last_start_directory.c_str(), filter_spec.c_str(), title.c_str(), attach.c_str(), nullptr };

  if (!g_spawn_async_with_pipes (NULL, /* working directory = current dir */
                                 (char **) &argv[0],
                                 NULL, /* inherit environment */
                                 G_SPAWN_SEARCH_PATH,
                                 NULL, NULL, /* no child setup */
                                 &child_pid,
                                 NULL, /* inherit stdin */
                                 &child_stdout,
                                 NULL, /* inherit stderr */
                                 &err))
    {
      printf ("error spawning child %s\n", err->message);
    }
}

void
ExtFileDialog::process_events()
{
  if (child_stdout >= 0)
    {
      fd_set fds;

      FD_ZERO (&fds);
      FD_SET (child_stdout, &fds);

      timeval tv;
      tv.tv_sec = 0;
      tv.tv_usec = 0;

      int select_ret = select (child_stdout + 1, &fds, NULL, NULL, &tv);

      if (select_ret > 0 && FD_ISSET (child_stdout, &fds))
        {
          unsigned char buffer[1024]; // we expect utf8 encoded filenames

          int bytes = read (child_stdout, buffer, 1024);
          for (int i = 0; i < bytes; i++)
            {
              if (buffer[i] >= 32)
                selected_filename += buffer[i];
              if (buffer[i] == '\n')
                selected_filename_ok = true;
            }
          if (bytes == 0)
            {
              // we ignore waitpid result here, as child may no longer exist,
              // and somebody else used waitpid() already (host)
              int status;
              waitpid (child_pid, &status, WNOHANG);

              if (selected_filename_ok)
                {
                  char *dir_name = g_path_get_dirname (selected_filename.c_str());
                  last_start_directory = dir_name;
                  g_free (dir_name);

                  signal_file_selected (selected_filename);
                }
              else
                signal_file_selected ("");

              child_pid = -1;

              close (child_stdout);
              child_stdout = -1;
            }
        }
    }
}
