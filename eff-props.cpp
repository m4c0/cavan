#pragma leco tool

import cavan;
import hai;
import jojo;
import jute;
import silog;

import hashley;

static void merge_props(cavan::pom * pom) {
  auto * ppom = &*pom->ppom;
  if (!ppom) return;

  merge_props(ppom);

  hashley::rowan has {};
  for (auto [k, _] : pom->props) has[k] = 1;
  for (auto p : ppom->props) {
    auto & it_has = has[p.key];
    if (it_has) continue;
    pom->props.push_back_doubling(p);
    it_has = 1;
  }
}

static void run(hai::cstr xml) {
  auto pom = cavan::read_pom(traits::move(xml));
  cavan::read_parent_chain(&pom);
  merge_props(&pom);

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
