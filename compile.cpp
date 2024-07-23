#pragma leco tool
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIM_IMPLEMENTATION
#include "../leco/sim.hpp"

import cavan;
import hai;
import jute;
import mtime;
import pprent;
import silog;
import yoyo;

using namespace jute::literals;

static void build_path(sim_sb *path, const char *grp, const char *art) {
  sim_sb_copy(path, getenv("HOME"));
  sim_sb_path_append(path, ".m2/repository");
  sim_sb_path_append(path, grp);
  sim_sb_path_append(path, art);
}

static auto build_path(sim_sb *path, const char *grp, const char *art,
                       const char *ver) {
  build_path(path, grp, art);
  sim_sb_path_append(path, ver);
  sim_sb_path_append(path, art);
  sim_sb_printf(path, "-%s.jar", ver);
  return mtime::of(path->buffer);
}
static void check_path(sim_sb *path, const char *grp, const char *art,
                       const char *ver) {
  if (build_path(path, grp, art, ver))
    return;

  silog::log(silog::error, "file not found: %s", path->buffer);
  throw 1;
}

static void find_dep_path(sim_sb *path, const cavan::dep &d) {
  sim_sbt grp_path{};
  sim_sb_copy(&grp_path, d.grp.begin());
  while (auto *c = strchr(grp_path.buffer, '.')) {
    *c = '/';
  }

  if (d.ver.begin()) {
    check_path(path, grp_path.buffer, d.art.begin(), d.ver.begin());
    return;
  }

  if (build_path(path, grp_path.buffer, d.art.begin(), "1.0-SNAPSHOT"))
    return;

  mtime::t max{};

  build_path(path, grp_path.buffer, d.art.begin());
  for (auto ver : pprent::list(path->buffer)) {
    sim_sbt tmp{};
    auto mtime = build_path(&tmp, grp_path.buffer, d.art.begin(), ver);

    if (mtime < max)
      continue;

    sim_sb_copy(path, tmp.buffer);
    max = mtime;
  }

  if (max == 0) {
    silog::log(silog::error, "dependency not found: %s:%s", d.grp.begin(),
               d.art.begin());
    throw 1;
  }
}

static void run_java(cavan::deps &deps) {
  for (auto &d : deps) {
    if (d.scp != "compile"_s)
      continue;

    sim_sbt path{};
    find_dep_path(&path, d);

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
