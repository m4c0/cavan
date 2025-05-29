module cavan;

enum effectiveness {
  NONE = 0,
  MERGED,
  DONE,
};

static void merge_parent_chain(cavan::pom * pom) try {
  if (pom->effectiveness >= MERGED) return;

  read_parent_chain(pom);
  merge_props(pom);
  if (pom->ppom) {
    merge_parent_chain(pom->ppom);

    // TODO: avoid copy, this adds up quickly
    for (auto & [d, depth] : pom->ppom->deps_mgmt) {
      pom->deps_mgmt.push_back(d, depth + 1);
    }
    for (auto & [d, depth] : pom->ppom->deps) {
      pom->deps.push_back(d, depth + 1);
    }
  }
  pom->effectiveness = MERGED;
} catch (...) {
  cavan::whilst("merging chain of deps for pom of " + pom->grp + ":" + pom->art + ":" + pom->ver);
}

static void update_deps_versions(cavan::pom * pom) try {
  for (auto & [d, _] : pom->deps) {
    pom->deps_mgmt.manage(&d);
    d.grp = cavan::apply_props(pom, d.grp);
    d.ver = cavan::apply_props(pom, d.ver);
    if (d.scp == "") d.scp = "compile";
  }
  for (auto & [d, _] : pom->deps_mgmt) {
    d.grp = cavan::apply_props(pom, d.grp);
    d.ver = cavan::apply_props(pom, d.ver);
  }
} catch (...) {
  cavan::whilst("calculating dependencies versions for pom of " + pom->grp + ":" + pom->art + ":" + pom->ver);
}

static void apply_imports(cavan::pom * pom) try {
  for (auto & [d, depth] : pom->deps_mgmt) {
    if (d.scp != "import"_s || d.typ != "pom"_s) continue;
    if (!d.pom) {
      d.ver = apply_props(pom, d.ver);
      d.pom = cavan::read_pom(*d.grp, d.art, *d.ver);
      eff_pom(d.pom);
    }
    for (auto & [id, idepth] : d.pom->deps_mgmt) {
      pom->deps_mgmt.push_back(id, depth + idepth);
    }
  }
} catch (...) {
  cavan::whilst("applying imported dependencies for pom of " + pom->grp + ":" + pom->art + ":" + pom->ver);
}

void cavan::eff_pom(cavan::pom * pom) try {
  if (pom->effectiveness == DONE) return;

  merge_parent_chain(pom);
  apply_imports(pom);
  update_deps_versions(pom);

  pom->effectiveness = DONE;
} catch (...) {
  cavan::whilst("calculating effective pom of " + pom->grp + ":" + pom->art + ":" + pom->ver);
}
