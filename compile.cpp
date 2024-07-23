#pragma leco tool
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

static auto build_javac(cavan::deps &deps) {
  sim_sbt classpath{102400};
  sim_sb_copy(&classpath, "target/classes");

  for (auto &d : deps) {
    if (d.scp != "compile"_s)
      continue;

    sim_sbt path{};
    find_dep_path(&path, d);

    sim_sb_concat(&classpath, ":");
    sim_sb_concat(&classpath, path.buffer);
  }

  hai::varray<hai::cstr> args{10240};
  args.push_back("javac"_s.cstr());
  args.push_back("-d"_s.cstr());
  args.push_back("target/classes"_s.cstr());
  args.push_back("-cp"_s.cstr());
  args.push_back(jute::view::unsafe(classpath.buffer).cstr());
  return args;
}

static void run(hai::varray<hai::cstr> &args) {
  hai::array<char *> argv{args.size() + 1};
  for (auto i = 0; i < args.size(); i++) {
    argv[i] = args[i].begin();
  }
  execvp(argv[0], argv.begin());
}

int main(int argc, char **argv) try {
  yoyo::file_reader::open("pom.xml")
      .fmap(cavan::read_tokens)
      .fpeek(cavan::lint_xml)
      .fmap(cavan::list_deps)
      .map(build_javac)
      .peek([&](auto &args) {
        for (auto i = 1; i < argc; i++) {
          args.push_back(jute::view::unsafe(argv[i]).cstr());
        }
      })
      .map(run)
      .log_error([] { throw 1; });
} catch (...) {
  return 1;
}
