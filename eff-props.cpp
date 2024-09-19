#pragma leco tool

import cavan;
import hai;
import jojo;
import jute;
import silog;

import hashley;

static void add_all(hashley::rowan & has, cavan::pom * pom, cavan::props * acc) {
  for (auto p : pom->props) {
    auto & n = has[p.key];
    if (n) continue;

    n = 1;
    acc->push_back_doubling(p);
  }
  if (pom->ppom) add_all(has, &*pom->ppom, acc);
}

static void run(hai::cstr xml) {
  auto pom = cavan::read_pom(traits::move(xml));
  cavan::read_parent_chain(&pom);

  cavan::props acc { 1024 };
  hashley::rowan has {};
  add_all(has, &pom, &acc);

  for (auto [k, v] : acc) {
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
