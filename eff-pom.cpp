#pragma leco tool

import cavan;
import jojo;
import jute;
import hai;
import hashley;
import silog;

using namespace jute::literals;

static void run(hai::cstr xml) {
  auto pom = cavan::read_pom(traits::move(xml));
  cavan::eff_pom(pom);
}

int main(int argc, char ** argv) try {
  for (auto i = 1; i < argc; i++) {
    run(jojo::read_cstr(jute::view::unsafe(argv[i])));
  }
} catch (...) {
  return 1;
}
