#pragma leco tool
#include <stdio.h>

#define SIM_IMPLEMENTATION
#include "../leco/sim.hpp"

import cavan;
import jojo;
import jute;

using namespace jute::literals;

static void build_path(sim_sb * path, const char * grp, const char * art) {
  sim_sb_copy(path, getenv("HOME"));
  sim_sb_path_append(path, ".m2/repository");
  sim_sb_path_append(path, grp);
  sim_sb_path_append(path, art);
}

static void build_path(sim_sb * path, const char * grp, const char * art, const char * ver, const char * type) {
  build_path(path, grp, art);
  sim_sb_path_append(path, ver);
  sim_sb_path_append(path, art);
  sim_sb_printf(path, "-%s.%s", ver, type);
}

static auto infer_base_folder(jute::view src) {
  while (src != "") {
    auto [l, r] = src.rsplit('/');
    if (r == "src") return l;
    src = l;
  }
  cavan::fail("file not in maven repo");
}

static int compile(char * fname, char * efpom) {
  auto pom = cavan::read_pom(jojo::read_cstr(jute::view::unsafe(efpom)));

  auto src = jute::view::unsafe(fname);
  auto tgt = infer_base_folder(src) + "/target/classes";

  jojo::write("tmp-args", "-d "_hs + tgt + "\n");
  jojo::append("tmp-args", "-cp "_hs + tgt);

  for (auto & d : pom.deps) {
    if (d.cls != "jar"_s && d.cls != ""_s) continue;
    if (d.scp != "compile"_s) continue;

    sim_sbt grp {};
    sim_sb_copy(&grp, d.grp.begin());
    while (auto p = strchr(grp.buffer, '.')) *p = '/';

    sim_sbt path {};
    build_path(&path, grp.buffer, d.art.begin(), d.ver.begin(), d.typ.begin());

    jojo::append("tmp-args", ":"_hs);
    jojo::append("tmp-args", jute::view { path.buffer, path.len });
  }

  jojo::append("tmp-args", "\n"_hs);
  jojo::append("tmp-args", src);

  auto res = system("javac @tmp-args");
  remove("tmp-args");
  return res;
}

int main(int argc, char ** argv) try {
  // TODO: generate effective-pom
  if (argc != 3) cavan::fail("usage: compile.exe <java-file> <effective-pom>");

  return compile(argv[1], argv[2]);
} catch (...) {
  return 1;
}
