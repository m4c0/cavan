#pragma leco tool

import cavan;
import hai;
import jojo;
import jute;
import silog;

static void run(jute::view fname) {
  auto pom = cavan::read_pom(fname);
  cavan::read_parent_chain(pom);
  cavan::merge_props(pom);

  for (auto [k, v] : pom->props) {
    auto vv = cavan::apply_props(pom, jute::heap { jute::no_copy {}, v });
    silog::log(silog::info, "%s = %s", k.cstr().begin(), (*vv).cstr().begin());
  }
  silog::log(silog::info, "Total: %d properties", pom->props.size());
}

int main(int argc, char ** argv) try {
  cavan::file_reader = jojo::read_cstr;
  for (auto i = 1; i < argc; i++) {
    silog::log(silog::info, "reading %s", argv[i]);
    run(jute::view::unsafe(argv[i]));
  }
} catch (...) {
  return 1;
}
