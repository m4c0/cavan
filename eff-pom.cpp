#pragma leco tool

import cavan;
import jute;
import hai;
import silog;

using namespace jute::literals;

static void run(jute::view fname) {
  auto pom = cavan::read_pom(fname);
  cavan::eff_pom(pom);

  for (auto & [d, _] : pom->deps_mgmt) {
    silog::log(silog::info, "dep mgmt -- %s:%s:%s", d.grp.cstr().begin(), d.art.cstr().begin(),
               (*d.ver).cstr().begin());
    if (d.exc)
      for (auto & [g, a] : *d.exc) {
        silog::log(silog::info, "    excl -- %s:%s", g.cstr().begin(), a.cstr().begin());
      }
  }
  for (auto & [d, _] : pom->deps) {
    silog::log(silog::info, "dep -- %s:%s:%s:%s:%s", d.grp.cstr().begin(), d.art.cstr().begin(),
               (*d.ver).cstr().begin(), d.scp.cstr().begin(), d.typ.cstr().begin());
    if (d.exc)
      for (auto & [g, a] : *d.exc) {
        silog::log(silog::info, "    excl -- %s:%s", g.cstr().begin(), a.cstr().begin());
      }
  }
}

int main(int argc, char ** argv) try {
  for (auto i = 1; i < argc; i++) {
    silog::log(silog::info, "reading %s", argv[i]);
    run(jute::view::unsafe(argv[i]));
  }
} catch (...) {
  return 1;
}
