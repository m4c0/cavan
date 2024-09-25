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

  auto * pom = cavan::parse_pom(tokens);
  cavan::read_parent_chain(pom);

  while (pom) {
    silog::log(silog::info, "filename: %s", pom->filename.begin());
    silog::log(silog::info, "name: %s:%s:%s", pom->grp.cstr().begin(), pom->art.cstr().begin(),
               pom->ver.cstr().begin());
    silog::log(silog::info, "parent: %s:%s:%s", pom->parent.grp.cstr().begin(), pom->parent.art.cstr().begin(),
               pom->parent.ver.cstr().begin());

    silog::log(silog::info, "found %d properties", pom->props.size());
    for (auto & [k, v] : pom->props) {
      silog::log(silog::info, "- %s = %s", k.cstr().begin(), v.cstr().begin());
    }

    silog::log(silog::info, "found %d managed dependencies", pom->deps_mgmt.size());
    for (auto & d : pom->deps_mgmt) {
      silog::log(silog::info, "- %10s %s:%s:%s", d.scp.cstr().begin(), d.grp.cstr().begin(), d.art.cstr().begin(),
                 (*d.ver).cstr().begin());
    }

    silog::log(silog::info, "found %d dependencies", pom->deps.size());
    for (auto & d : pom->deps) {
      silog::log(silog::info, "- %10s %s:%s:%s", d.scp.cstr().begin(), d.grp.cstr().begin(), d.art.cstr().begin(),
                 (*d.ver).cstr().begin());
    }

    silog::log(silog::info, "found %d modules", pom->modules.size());
    for (auto & d : pom->modules) {
      silog::log(silog::info, "- %s", d.cstr().begin());
    }

    pom = &*pom->ppom;
    if (pom) silog::log(silog::info, "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-");
  }
}

int main(int argc, char ** argv) try {
  for (auto i = 1; i < argc; i++) {
    jojo::read(jute::view::unsafe(argv[i]), nullptr, dump_pom);
  }
} catch (...) {
  return 1;
}
