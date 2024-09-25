#pragma leco tool
#include <stdlib.h>
#include <string.h>

import cavan;
import hai;
import hashley;
import jojo;
import jute;

using namespace jute::literals;

static auto infer_base_folder(jute::view src) {
  while (src != "") {
    auto [l, r] = src.rsplit('/');
    if (r == "src") return l;
    src = l;
  }
  cavan::fail("file not in maven repo");
}

static void add_deps(const auto & tmpnam, cavan::pom & pom, bool test_scope, hai::fn<bool, cavan::dep &> excl) try {
  jute::heap m2repo { jute::view::unsafe(getenv("HOME")) + "/.m2/repository" };

  for (auto & d : pom.deps) {
    if (excl(d)) continue;
    if (d.cls != "jar"_s && d.cls != ""_s) continue;
    if (test_scope && d.scp != "test"_s && d.scp != "compile"_s) continue;
    if (!test_scope && d.scp != "compile"_s) continue;

    auto jar = m2repo;
    auto grp = d.grp;
    while (grp.size() > 0) {
      auto [l, r] = grp.split('.');
      jar = jar + "/" + l;
      grp = r;
    }
    jar = jar + "/" + d.art + "/" + d.ver + "/" + d.art + "-" + d.ver + "." + d.typ;

    jojo::append(tmpnam, ":"_hs + jar);

    auto dpom = cavan::read_pom(d.grp, d.art, *d.ver);
    cavan::eff_pom(&dpom);

    // TODO: dependency recursion
    // TODO: "reactor"?

    /*
    hashley::rowan exc {};
    for (auto [g, a] : *d.exc) exc[(g + ":" + a).cstr()] = 1;
    add_deps(tmpnam, dpom, test_scope, [&exc, &excl](auto & d) {
      if (exc[(d.grp + ":" + d.art).cstr()]) return true;
      return excl(d);
    });
    */
  }
} catch (...) {
  cavan::whilst("processing dependencies of " + pom.grp + ":" + pom.art + ":" + pom.ver);
}

static int compile(char * fname) {
  auto src = jute::view::unsafe(fname);
  auto base = infer_base_folder(src);
  auto tgt = base + "/target/classes";
  auto pom_file = (base + "/pom.xml").cstr();
  auto tmpnam = (base + "/target/cavan-compile.args").cstr();

  auto pom = cavan::read_pom(pom_file);
  cavan::eff_pom(&pom);

  jojo::write(tmpnam, "-d "_hs + tgt + "\n");
  jojo::append(tmpnam, "-cp "_hs + tgt);

  bool test_scope = strstr(fname, "src/test/");

  add_deps(tmpnam, pom, test_scope, [](auto &) { return false; });

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
