module cavan;
import hai;
import jute;

hai::array<cavan::pom *> cavan::read_modules(const cavan::pom * pom) try {
  hai::array<cavan::pom *> res { pom->modules.size() };
  auto dir = jute::view { pom->filename }.rsplit('/').before;
  for (auto i = 0; i < res.size(); i++) {
    auto cpom = dir + "/" + pom->modules.seek(i) + "/pom.xml";
    try {
      res[i] = cavan::read_pom(cpom.cstr());
    } catch (...) {
      cavan::whilst("reading module " + pom->modules.seek(i));
    }
  }
  return res;
} catch (...) {
  cavan::whilst("reading modules of " + jute::view { pom->filename });
}

void cavan::warm_modules_from_parent_chain(const cavan::pom * pom) {
  auto _ = read_modules(pom);
  if (pom->ppom) warm_modules_from_parent_chain(pom->ppom);
}
