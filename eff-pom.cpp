#pragma leco tool

import cavan;
import jojo;
import jute;
import hai;
import print;

using namespace jute::literals;

static void run(jute::view fname) {
  auto pom = cavan::read_pom(fname);
  cavan::eff_pom(pom);

  for (auto & [d, _] : pom->deps_mgmt) {
    putfn("dep mgmt -- %s:%s:%s:%s:%s", (*d.grp).cstr().begin(), d.art.cstr().begin(),
          (*d.ver).cstr().begin(), d.scp.cstr().begin(), d.typ.cstr().begin());
    if (d.exc)
      for (auto & [g, a] : *d.exc) {
        put("\e[31m");
        putfn("    excl -- %s:%s", g.cstr().begin(), a.cstr().begin());
        put("\e[0m");
      }
  }
  putln();
  for (auto & [d, _] : pom->deps) {
    pom->deps_mgmt.manage(&d);
    putfn("     dep -- %s:%s:%s:%s:%s", (*d.grp).cstr().begin(), d.art.cstr().begin(),
          (*d.ver).cstr().begin(), d.scp.cstr().begin(), d.typ.cstr().begin());
    if (d.exc)
      for (auto & [g, a] : *d.exc) {
        put("\e[31m");
        putfn("    excl -- %s:%s", g.cstr().begin(), a.cstr().begin());
        put("\e[0m");
      }
  }
}

int main(int argc, char ** argv) try {
  cavan::file_reader = jojo::read_cstr;
  for (auto i = 1; i < argc; i++) {
    putfn("reading %s", argv[i]);
    run(jute::view::unsafe(argv[i]));
  }
} catch (...) {
  return 1;
}
