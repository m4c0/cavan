#pragma leco tool
#include <unistd.h>

#define SIM_IMPLEMENTATION
#include "../leco/sim.hpp"

import cavan;
import jojo;
import jute;
import mtime;

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

static void create_efpom(jute::view base) {
  // TODO: generate effective-pom using cavan

  chdir(base.cstr().begin());

  const auto cmd = "mvn -q -nsu -o -B help:effective-pom -Dverbose=true -Doutput=target/cavan-compile.pom";
  if (0 != system(cmd)) cavan::fail("maven failed");
}

static int compile(char * fname) {
  auto src = jute::view::unsafe(fname);
  auto base = infer_base_folder(src);
  auto tgt = base + "/target/classes";
  auto efpom = (base + "/target/cavan-compile.pom").cstr();
  auto tmpnam = (base + "/target/cavan-compile.args").cstr();

  if (mtime::of(efpom.begin()) == 0) create_efpom(base);

  auto pom = cavan::read_pom(jojo::read_cstr(efpom));

  jojo::write(tmpnam, "-d "_hs + tgt + "\n");
  jojo::append(tmpnam, "-cp "_hs + tgt);

  for (auto & d : pom.deps) {
    if (d.cls != "jar"_s && d.cls != ""_s) continue;
    if (d.scp != "compile"_s) continue;

    sim_sbt grp {};
    sim_sb_copy(&grp, d.grp.begin());
    while (auto p = strchr(grp.buffer, '.')) *p = '/';

    sim_sbt path {};
    build_path(&path, grp.buffer, d.art.begin(), d.ver.begin(), d.typ.begin());

    jojo::append(tmpnam, ":"_hs);
    jojo::append(tmpnam, jute::view { path.buffer, path.len });
  }

  jojo::append(tmpnam, "\n"_hs);
  jojo::append(tmpnam, src);

  auto cmd = ("javac @"_s + tmpnam).cstr();
  return system(cmd.begin());
}

int main(int argc, char ** argv) try {
  if (argc != 2) cavan::fail("usage: compile.exe <java-file>");

  return compile(argv[1]);
} catch (...) {
  return 1;
}
