#pragma leco tool

import cavan;
import hai;
import jojo;
import jute;
import missingno;
import silog;

using namespace cavan;
using namespace jute::literals;

static void add_tdep(cavan::deps & out, const cavan::dep & d, bool test_scope) {
  auto dpom = cavan::read_pom(*d.grp, d.art, *d.ver);
  cavan::eff_pom(dpom);
  for (auto & [d, _] : dpom->deps) {
    if (d.opt) continue;
    if (d.cls != "jar"_s && d.cls != ""_s) continue;
    if (test_scope && d.scp != "test"_s && d.scp != "compile"_s && d.scp != "provided") continue;
    if (!test_scope && d.scp != "compile"_s && d.scp != "provided") continue;

    out.push_back(d);
  }
}

static void run(jute::view fname) {
  auto pom = cavan::read_pom(fname);
  cavan::eff_pom(pom);

  cavan::deps deps {};
  for (auto & [dep, _] : pom->deps) {
    add_tdep(deps, dep, true);
  }

  for (auto & [dep, _] : deps) {
    add_tdep(deps, dep, false);
  }
}

int main(int argc, char ** argv) try {
  for (auto i = 1; i < argc; i++) {
    run(jute::view::unsafe(argv[i]));
  }
} catch (...) {
  return 1;
}
