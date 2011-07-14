/*
 * Copyright (C) 2011 Stefan Westerfeld
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

#include "sminfile.hh"
#include "smoutfile.hh"
#include "smmemout.hh"
#include "smmain.hh"

#include <map>

#include <birnet/birnet.hh>
#include <glib.h>
#include <stdlib.h>

#define PROG_NAME "smfileedit"

using namespace SpectMorph;

using std::vector;
using std::string;
using std::map;

struct Options
{
  string         mode;
  string         in_file;
  string         out_file;
  vector<string> commands;
} options;

bool
match_lhs (const string& command, const string& property, string& rhs)
{
  size_t pos = command.find ('=');
  if (pos != command.npos)
    {
      if (command.substr (0, pos) == property)
        {
          rhs = command.substr (pos + 1);
          return true;
        }
    }
  return false;
}

void
process_file (const string& property_prefix, InFile& ifile, OutFile& ofile)
{
  map<string, vector<unsigned char> > output_blob_map;
  map<string, int> section_counter;
  map<string, int> name_counter;

  string section_prefix;

  while (ifile.event() != InFile::END_OF_FILE)
    {
      string property = ifile.event_name();
      property = property_prefix + section_prefix + property;
      property = Birnet::string_printf ("%s[%d]", property.c_str(), name_counter[property]++);

      if (ifile.event() == InFile::BEGIN_SECTION)
        {
          const string& s = ifile.event_name();

          ofile.begin_section (ifile.event_name());
          section_prefix = Birnet::string_printf ("%s[%d]", s.c_str(), section_counter[s]++) + ".";
        }
      else if (ifile.event() == InFile::END_SECTION)
        {
          section_prefix = "";
          ofile.end_section();
        }
      else if (ifile.event() == InFile::STRING)
        {
          string value = ifile.event_data();

          for (vector<string>::iterator ci = options.commands.begin(); ci != options.commands.end(); ci++)
            {
              string new_value;
              if (match_lhs (*ci, property, new_value))
                value = new_value;
            }

          if (options.mode == "list")
            printf ("%s=%s\n", property.c_str(), value.c_str());

          ofile.write_string (ifile.event_name(), value);
        }
      else if (ifile.event() == InFile::FLOAT)
        {
          ofile.write_float (ifile.event_name(), ifile.event_float());
        }
      else if (ifile.event() == InFile::FLOAT_BLOCK)
        {
          ofile.write_float_block (ifile.event_name(), ifile.event_float_block());
        }
      else if (ifile.event() == InFile::INT)
        {
          ofile.write_int (ifile.event_name().c_str(), ifile.event_int());
        }
      else if (ifile.event() == InFile::BOOL)
        {
          ofile.write_bool (ifile.event_name(), ifile.event_bool());
        }
      else if (ifile.event() == InFile::BLOB)
        {
          GenericIn *blob_in = ifile.open_blob();
          if (!blob_in)
            {
              fprintf (stderr, PROG_NAME ": error opening input\n");
              exit (1);
            }
          InFile blob_ifile (blob_in);

          vector<unsigned char> blob_data;
          MemOut                blob_mo (&blob_data);

          {
            OutFile blob_ofile (&blob_mo, blob_ifile.file_type(), blob_ifile.file_version());
            process_file (property + ".", blob_ifile, blob_ofile);
            // blob_ofile destructor run here
          }

          ofile.write_blob (ifile.event_name(), &blob_data[0], blob_data.size());
          delete blob_in;

          output_blob_map[ifile.event_blob_sum()] = blob_data;
        }
      else if (ifile.event() == InFile::BLOB_REF)
        {
          vector<unsigned char>& blob_data = output_blob_map[ifile.event_blob_sum()];

          ofile.write_blob (ifile.event_name(), &blob_data[0], blob_data.size());
        }
      else if (ifile.event() == InFile::READ_ERROR)
        {
          g_printerr (PROG_NAME ": read error\n");
          break;
        }
      else
        {
          g_printerr (PROG_NAME ": unhandled event %d\n", ifile.event());
        }
      ifile.next_event();
    }
}

void
print_usage ()
{
  printf ("usage: %s <command> [ <options> ] [ <command specific args...> ]\n", PROG_NAME);
  printf ("\n");
  printf ("command specific args:\n");
  printf ("\n");
  printf (" %s list [ <options> ] <infile>\n", PROG_NAME);
  printf (" %s edit [ <options> ] <infile> <outfile> [ <property1>=<value1> ... ]\n", PROG_NAME);
  printf ("\n");
}


int
main (int argc, char **argv)
{
  vector<string> commands;

  sm_init (&argc, &argv);

  if (argc < 3)
    {
      print_usage();
      return 1;
    }

  options.mode    = argv[1];
  options.in_file = argv[2];

  if (options.mode == "list")
    {
      options.out_file = "/dev/null";
    }
  else if (options.mode == "edit")
    {
      if (argc < 4)
        {
          print_usage();
          return 1;
        }

      options.out_file = argv[3];
      for (int i = 4; i < argc; i++)
        options.commands.push_back (argv[i]);
    }
  else
    {
      printf ("command must be list or edit\n\n");
      print_usage();
      return 1;
    }

  GenericIn *in = StdioIn::open (options.in_file);
  if (!in)
    {
      fprintf (stderr, PROG_NAME ": error opening input\n");
      return 1;
    }
  InFile ifile (in);
  OutFile ofile (options.out_file, ifile.file_type(), ifile.file_version());

  process_file ("", ifile, ofile);
  delete in;
}
