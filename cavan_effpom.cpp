module cavan;

void cavan::eff_pom(cavan::pom * pom) try {
  if (pom->effective) return;

  read_parent_chain(pom);
  merge_props(pom);
  if (pom->ppom) {
    eff_pom(pom->ppom);

    // TODO: avoid copy, this adds up quickly
    for (auto & [d, depth] : pom->ppom->deps_mgmt) {
      pom->deps_mgmt.push_back(d, depth + 1);
    }
    for (auto & [d, depth] : pom->ppom->deps) {
      pom->deps.push_back(d, depth + 1);
    }
  }

  for (auto & [d, depth] : pom->deps_mgmt) {
    if (d.scp != "import"_s || d.typ != "pom"_s) continue;
    if (!d.pom) {
      d.ver = apply_props(pom, d.ver);
      d.pom = read_pom(d.grp, d.art, *d.ver);
      eff_pom(d.pom);
    }
    for (auto & [id, idepth] : d.pom->deps_mgmt) {
      pom->deps_mgmt.push_back(id, depth + idepth);
    }
  }

  for (auto & [d, _] : pom->deps) {
    if (d.ver.size() == 0) d.ver = pom->deps_mgmt[d].dep.ver;
    d.ver = cavan::apply_props(pom, d.ver);
  }

  pom->effective = true;
} catch (...) {
  cavan::whilst("calculating effective pom of " + pom->grp + ":" + pom->art + ":" + pom->ver);
}
