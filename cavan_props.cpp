module cavan;

void cavan::merge_props(cavan::pom * pom) {
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
