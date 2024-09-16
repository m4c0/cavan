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
  while (str.size() > 3 && str[0] == '$' && str[1] == '{' && str[str.size() - 1] == '}') {
    auto prop = str.subview(2, str.size() - 3).middle;
    str = (*this)[prop];
  }
  return str.cstr();
}
