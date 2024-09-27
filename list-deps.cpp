#pragma leco tool

import cavan;
import hai;
import jojo;
import jute;
import missingno;
import silog;

using namespace cavan;
using namespace jute::literals;

static void run(jute::view fname) {
  auto pom = cavan::read_pom(fname);
  auto & deps = pom->deps;

  silog::log(silog::info, "found %d dependencies", deps.size());

  for (auto & [d, _] : deps) {
    silog::log(silog::debug, "%s:%s:%s:%s", d.grp.cstr().begin(), d.art.cstr().begin(), (*d.ver).cstr().begin(),
               d.scp.cstr().begin());
  }
}

int main(int argc, char ** argv) try {
  for (auto i = 1; i < argc; i++) {
    run(jute::view::unsafe(argv[i]));
  }
} catch (...) {
  return 1;
}
