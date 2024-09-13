#pragma leco tool

import cavan;
import hai;
import jojo;
import jute;
import missingno;
import traits;
import silog;

using namespace cavan;
using namespace jute::literals;

static void dump_pom(void *, hai::cstr & xml) {
  auto tokens = cavan::split_tokens(xml);
  cavan::lint_xml(tokens);

  auto pom = cavan::parse_pom(tokens);
  while (true) {
    silog::log(silog::info, "filename: %s", pom.filename.begin());
    silog::log(silog::info, "name: %s:%s:%s", pom.grp.begin(), pom.art.begin(), pom.ver.begin());
    silog::log(silog::info, "parent: %s:%s:%s", pom.parent.grp.begin(), pom.parent.art.begin(), pom.parent.ver.begin());

    silog::log(silog::info, "found %d properties", pom.props.size());
    for (auto & [k, v] : pom.props) {
      silog::log(silog::info, "- %s = %s", k.begin(), v.begin());
    }

    silog::log(silog::info, "found %d managed dependencies", pom.deps_mgmt.size());
    for (auto & d : pom.deps_mgmt) {
      silog::log(silog::info, "- %s:%s:%s", d.grp.begin(), d.art.begin(), d.ver.begin());
    }

    silog::log(silog::info, "found %d dependencies", pom.deps.size());
    for (auto & d : pom.deps) {
      silog::log(silog::info, "- %s:%s:%s", d.grp.begin(), d.art.begin(), d.ver.begin());
    }

    if (pom.parent.grp.size() == 0) break;

    silog::log(silog::info, "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-");

    pom = cavan::read_pom(pom.parent.grp, pom.parent.art, pom.parent.ver);
  }
}

int main(int argc, char ** argv) try {
  for (auto i = 1; i < argc; i++) {
    jojo::read(jute::view::unsafe(argv[i]), nullptr, dump_pom);
  }
} catch (...) {
  return 1;
}
