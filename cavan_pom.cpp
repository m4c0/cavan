module cavan;

mno::req<cavan::pom> cavan::parse_pom(const cavan::tokens &ts) {
  cavan::pom res{};

  auto *t = ts.begin();

  if (!match(*t, T_DIRECTIVE, "xml"))
    return mno::req<pom>::failed("missing <?xml?> directive");

  t++;
  if (!match(*t, T_OPEN_TAG, "project"))
    return mno::req<pom>::failed("missing <project>");

  t++;
  for (; !match(*t, T_END) && !match(*t, T_CLOSE_TAG, "project"); t++) {
    if (match(*t, T_OPEN_TAG, "dependencyManagement")) {
      if (match(*t, T_OPEN_TAG, "dependencies")) {
        auto res = list_deps(t);
        if (!res.is_valid())
          return res.map([](auto &) { return pom{}; });
      }

      if (!match(*t, T_CLOSE_TAG, "dependencyManagement")) {
        return mno::req<pom>::failed(
            "found unsupported tag inside <dependencyManagement>");
      }

      continue;
    }
    if (match(*t, T_OPEN_TAG, "dependencies")) {
      auto res = list_deps(t);
      if (!res.is_valid())
        return res.map([](auto &) { return pom{}; });
      continue;
    }

    if (match(*t, T_OPEN_TAG)) {
      auto res = lint_tag(t);
      if (!res.is_valid())
        return res.map([] { return pom{}; });

      continue;
    }
  }

  if (!match(*t, T_CLOSE_TAG, "project"))
    return mno::req<pom>::failed("missing </project>");

  return mno::req{traits::move(res)};
}
