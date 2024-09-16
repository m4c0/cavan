module cavan;

cavan::kvmap::kvmap() : m_bucket { 1024000 }, m_deps {} {}

void cavan::kvmap::take(jute::view key, jute::view val, unsigned depth) {
  auto & idx = m_deps[key.cstr().begin()];
  if (idx == 0) {
    m_bucket.push_back(kv { key.cstr(), val.cstr(), depth });
    idx = m_bucket.size();
    return;
  }

  auto & dd = m_bucket[idx - 1];
  if (dd.depth <= depth) return;

  dd.val = val.cstr();
  dd.depth = depth;
}

[[nodiscard]] jute::view cavan::kvmap::operator[](jute::view key) const {
  auto idx = m_deps[key.cstr().begin()];
  if (idx == 0) return "TBD";
  return m_bucket[idx - 1].val;
}

void cavan::depmap::take(jute::view grp, jute::view art, jute::view ver, unsigned depth) {
  auto key = grp + ":" + art;
  kvmap::take(key.cstr(), ver, depth);
}
[[nodiscard]] jute::view cavan::depmap::version_of(jute::view grp, jute::view art) const {
  auto key = grp + ":" + art;
  return kvmap::operator[](key.cstr());
}

hai::cstr cavan::propmap::apply(jute::view str) const {
  hai::cstr res = str.cstr();
  for (unsigned i = 0; i < str.size() - 3; i++) {
    if (str[i] != '$') continue;
    if (str[i + 1] != '{') continue;

    unsigned j {};
    for (j = i; j < str.size() && str[j] != '}'; j++) {
    }

    if (j == str.size()) return res;

    jute::view before { str.begin(), i };
    jute::view prop { str.begin() + i + 2, j - i - 2 };
    jute::view after { str.begin() + j + 1, str.size() - j - 1 };

    auto concat = before + (*this)[prop] + after;
    res = concat.cstr();
    str = res;
    i--;
  }
  return res;
}
