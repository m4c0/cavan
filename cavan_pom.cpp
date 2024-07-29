module cavan;
import silog;

mno::req<cavan::pom> cavan::parse_pom(const cavan::tokens &ts) {
  auto *t = ts.begin();

  if (!match(*t++, T_DIRECTIVE, "xml"))
    return mno::req<pom>::failed("missing <?xml?> directive");

  cavan::pom res{};
  return take(t, "project",
              [&] -> mno::req<void> {
                if (match(*t, T_OPEN_TAG, "dependencyManagement")) {
                  return take_if(t, "dependencyManagement", [&] {
                    return list_deps(t).map([&](auto &deps) {
                      res.deps_mgmt = traits::move(deps);
                    });
                  });
                }
                if (match(*t, T_OPEN_TAG, "dependencies")) {
                  return list_deps(t).map(
                      [&](auto &deps) { res.deps = traits::move(deps); });
                }
                return lint_tag(t);
              })
      .map([&] { return traits::move(res); });
}
