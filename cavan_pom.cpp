module cavan;
import silog;

static mno::req<void> parse_project(const cavan::token *&t, cavan::pom &res) {
  using namespace cavan;

  if (match(*t, T_OPEN_TAG, "groupId"))
    return take_tag("groupId", t, &res.grp);

  if (match(*t, T_OPEN_TAG, "artifactId"))
    return take_tag("artifactId", t, &res.art);

  if (match(*t, T_OPEN_TAG, "version"))
    return take_tag("version", t, &res.ver);

  if (match(*t, T_OPEN_TAG, "parent"))
    return take_if(t, "parent", [&] {
      if (match(*t, T_OPEN_TAG, "groupId"))
        return take_tag("groupId", t, &res.parent.grp);

      if (match(*t, T_OPEN_TAG, "artifactId"))
        return take_tag("artifactId", t, &res.parent.art);

      if (match(*t, T_OPEN_TAG, "version"))
        return take_tag("version", t, &res.parent.ver);

      return mno::req<void>::failed("unknown tag inside <parent>");
    });

  if (match(*t, T_OPEN_TAG, "dependencyManagement")) {
    return take_if(t, "dependencyManagement", [&] {
      return list_deps(t).map(
          [&](auto &deps) { res.deps_mgmt = traits::move(deps); });
    });
  }
  if (match(*t, T_OPEN_TAG, "dependencies")) {
    return list_deps(t).map([&](auto &deps) { res.deps = traits::move(deps); });
  }
  return lint_tag(t);
}

mno::req<cavan::pom> cavan::parse_pom(const cavan::tokens &ts) {
  auto *t = ts.begin();

  if (!match(*t++, T_DIRECTIVE, "xml"))
    return mno::req<pom>::failed("missing <?xml?> directive");

  cavan::pom res{};
  return take(t, "project", [&] { return parse_project(t, res); }).map([&] {
    if (res.grp.size() == 0)
      res.grp = jute::view{res.parent.grp}.cstr();
    if (res.ver.size() == 0)
      res.ver = jute::view{res.parent.ver}.cstr();
    return traits::move(res);
  });
}
