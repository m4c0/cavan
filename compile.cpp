#pragma leco tool
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIM_IMPLEMENTATION
#include "../leco/sim.hpp"

import cavan;
import jute;
import yoyo;

using namespace jute::literals;

static void run_java(cavan::deps &deps) {
  for (auto &d : deps) {
    if (d.scp != "compile"_s)
      continue;

    sim_sbt grp_path{};
    sim_sb_copy(&grp_path, d.grp.begin());
    while (auto *c = strchr(grp_path.buffer, '.')) {
      *c = '/';
    }

    const char *ver = d.ver.begin() ? d.ver.begin() : "1.0-SNAPSHOT";

    sim_sbt path{};
    sim_sb_copy(&path, getenv("HOME"));
    sim_sb_path_append(&path, grp_path.buffer);
    sim_sb_path_append(&path, d.art.begin());
    sim_sb_path_append(&path, ver);
    sim_sb_path_append(&path, d.art.begin());
    sim_sb_printf(&path, "-%s.jar", ver);

    puts(path.buffer);
  }
}

int main() try {
  yoyo::file_reader::open("pom.xml")
      .fmap(cavan::read_tokens)
      .fpeek(cavan::lint_xml)
      .fmap(cavan::list_deps)
      .map(run_java)
      .log_error([] { throw 1; });
} catch (...) {
  return 1;
}
