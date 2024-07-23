#pragma leco tool
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SIM_IMPLEMENTATION
#include "../leco/sim.hpp"

import cavan;
import hai;
import hashley;
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
                       const char *ver, const char *type) {
  build_path(path, grp, art);
  sim_sb_path_append(path, ver);
  sim_sb_path_append(path, art);
  sim_sb_printf(path, "-%s.%s", ver, type);
  return mtime::of(path->buffer);
}

static void find_dep_path(sim_sb *path, const char *grp, const char *art,
                          const char *ver) {
  sim_sbt grp_path{};
  sim_sb_copy(&grp_path, grp);
  while (auto *c = strchr(grp_path.buffer, '.')) {
    *c = '/';
  }

  if (ver) {
    if (build_path(path, grp_path.buffer, art, ver, "jar"))
      return;
  }

  if (build_path(path, grp_path.buffer, art, "1.0-SNAPSHOT", "jar"))
    return;

  mtime::t max{};

  build_path(path, grp_path.buffer, art);
  for (auto ver : pprent::list(path->buffer)) {
    sim_sbt tmp{};
    auto mtime = build_path(&tmp, grp_path.buffer, art, ver, "jar");

    if (mtime < max)
      continue;

    sim_sb_copy(path, tmp.buffer);
    max = mtime;
  }

  if (max == 0) {
    silog::log(silog::error, "dependency not found: %s:%s", grp, art);
    throw 1;
  }
}

static void append_classpath(hashley::rowan &done, sim_sb *classpath,
                             const cavan::dep *parent,
                             const cavan::deps &deps) {
  for (auto &d : deps) {
    if (d.opt)
      continue;
    if (d.scp != "compile"_s)
      continue;

    sim_sbt key{};
    sim_sb_printf(&key, "%s:%s", d.grp.begin(), d.art.begin());
    auto &k = done[key.buffer];
    if (k > 0)
      continue;

    if (parent && parent->exc[key.buffer])
      continue;

    k = 1;

    auto grp =
        "${project.groupId}"_s == d.grp ? parent->grp.begin() : d.grp.begin();
    auto art = "${project.artifactId}"_s == d.art ? parent->art.begin()
                                                  : d.art.begin();
    auto ver = d.ver.begin();

    sim_sbt path{};
    find_dep_path(&path, grp, art, ver);

    sim_sb_concat(classpath, ":");
    sim_sb_concat(classpath, path.buffer);

    sim_sb_path_set_extension(&path, "pom");
    yoyo::file_reader::open(path.buffer)
        .fmap(cavan::read_tokens)
        .fmap(cavan::list_deps)
        .map([&](auto &deps) { append_classpath(done, classpath, &d, deps); })
        .trace("traversing "_hs + jute::view::unsafe(key.buffer))
        .log_error([] { throw 1; });
  }
}

static auto build_javac(const cavan::deps &deps) {
  hashley::rowan done{};

  sim_sbt classpath{102400};
  sim_sb_copy(&classpath, "target/classes");
  append_classpath(done, &classpath, nullptr, deps);

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

static void compile(const char *pom, jute::view src) {
  yoyo::file_reader::open(pom)
      .fmap(cavan::read_tokens)
      .fpeek(cavan::lint_xml)
      .fmap(cavan::list_deps)
      .map(build_javac)
      .peek([&](auto &args) { args.push_back(src.cstr()); })
      .map(run)
      .log_error([] { throw 1; });
}

int main(int argc, char **argv) try {
  if (argc != 2) {
    fprintf(stderr, "usage: compile.exe <java-file>");
    return 1;
  }

  sim_sbt file{};
  sim_sb_path_copy_real(&file, argv[1]);
  while (file.len > 1) {
    sim_sb_path_parent(&file);
    sim_sb_path_parent(&file);
    sim_sb_path_append(&file, "pom.xml");

    if (mtime::of(file.buffer) > 0) {
      compile(file.buffer, jute::view::unsafe(argv[1]));
    }
  }

  return 1;
} catch (...) {
  return 1;
}
