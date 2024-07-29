module cavan;

mno::req<cavan::pom> cavan::parse_pom(const cavan::tokens &t) {
  cavan::pom res{};
  return cavan::list_deps(t)
      .map([&](auto &ds) { res.deps = traits::move(ds); })
      .map([&] { return traits::move(res); });
}
