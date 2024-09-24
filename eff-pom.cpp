#pragma leco tool

import cavan;
import jojo;
import jute;
import hai;
import silog;

using namespace jute::literals;

static void run(hai::cstr xml) {
  auto pom = cavan::read_pom(traits::move(xml));
  cavan::eff_pom(&pom);

  for (auto & d : pom.deps) {
    silog::log(silog::info, "dep -- %s:%s:%s", d.grp.cstr().begin(), d.art.cstr().begin(), (*d.ver).cstr().begin());
  }
}

int main(int argc, char ** argv) try {
  for (auto i = 1; i < argc; i++) {
    silog::log(silog::info, "reading %s", argv[i]);
    run(jojo::read_cstr(jute::view::unsafe(argv[i])));
  }
} catch (...) {
  return 1;
}
