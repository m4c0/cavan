#pragma leco tool

import cavan;
import hai;
import jojo;
import jute;
import silog;

static void run(hai::cstr xml) {
  auto pom = cavan::read_pom(traits::move(xml));
  cavan::read_parent_chain(&pom);
  cavan::merge_props(&pom);

  for (auto [k, v] : pom.props) {
    silog::log(silog::info, "%s = %s", k.cstr().begin(), v.cstr().begin());
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
