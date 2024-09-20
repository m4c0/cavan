module cavan;

void cavan::merge_props(cavan::pom * pom) {
  auto * ppom = &*pom->ppom;
  if (!ppom) return;

  merge_props(ppom);

  auto idx = 0;
  for (auto [k, _] : pom->props) pom->prop_index[k] = ++idx;
  for (auto p : ppom->props) {
    auto & i = pom->prop_index[p.key];
    if (i) continue;
    pom->props.push_back_doubling(p);
    i = ++idx;
  }
}

jute::heap cavan::apply_props(cavan::pom * pom, jute::heap str) {
  for (unsigned i = 0; i < str.size(); i++) {
    if ((*str)[i] != '$') continue;
    if ((*str)[i + 1] != '{') continue;

    unsigned j {};
    for (j = i; j < str.size() && (*str)[j] != '}'; j++) {
    }

    if (j == str.size()) return str;

    jute::view before { str.begin(), i };
    jute::view prop { str.begin() + i + 2, j - i - 2 };
    jute::view after { str.begin() + j + 1, str.size() - j - 1 };

    auto pidx = pom->prop_index[prop];
    if (pidx == 0) continue;

    str = before + pom->props[pidx - 1].val + after;
    i--;
  }
  return str;
}
