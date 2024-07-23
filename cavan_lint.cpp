module cavan;

namespace cavan {
static mno::req<void> lint_tag(const token *&t) {
  if (!match(*t, T_OPEN_TAG))
    return mno::req<void>::failed("missing open tag");

  jute::view id{t->id};

  for (t++; !match(*t, T_END); t++) {
    if (match(*t, T_CLOSE_TAG, id)) {
      return mno::req{};
    }

    if (match(*t, T_CLOSE_TAG))
      return mno::req<void>::failed("mismatched close tag");

    if (match(*t, T_OPEN_TAG)) {
      auto res = lint_tag(t);
      if (!res.is_valid())
        return res;
      continue;
    }
  }
  return mno::req<void>::failed("missing open tag");
}

mno::req<void> lint_xml(const cavan::tokens &tokens) {
  auto *t = tokens.begin();

  if (!match(*t, T_OPEN_TAG, "?xml"))
    return mno::req<void>::failed("missing <?xml?> directive");

  return lint_tag(++t);
}
} // namespace cavan