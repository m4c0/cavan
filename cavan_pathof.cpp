module cavan;
import jute;
import sysstd;

using namespace jute::literals;

static constexpr bool is_digits(jute::view v, int n) {
  if (v.size() != n) return false;
  for (auto c : v) if (c < '0' || c > '9') return false;
  return true;
}
static constexpr bool is_timestamped(jute::view r) {
  auto [dmy, r1] = r.split('.');
  if (r1 == "") return false;
  if (!is_digits(dmy, 8)) return false;

  auto [hms, bld] = r1.split('-');
  if (bld == "") return false;
  if (!is_digits(hms, 6)) return false;
  if (!is_digits(bld, bld.size())) return false;

  return true;
}
static_assert(is_timestamped("19520311.052501-42"));
static_assert(!is_timestamped("19520311-052501-42"));
static_assert(!is_timestamped("1952031a.052501-42"));
static_assert(!is_timestamped("19520311.05250b-42"));
static_assert(!is_timestamped("19520311.052501-4c"));
static_assert(!is_timestamped("195203.052501-1"));
static_assert(!is_timestamped("19520311.0525-1"));
static_assert(!is_timestamped("19520311.052501"));
static_assert(!is_timestamped("19520311."));
static_assert(!is_timestamped("19520311"));
static_assert(!is_timestamped(""));

static constexpr jute::heap path_of_ver(jute::view ver) {
  unsigned i = ver.size() - 2;
  unsigned n = 0;
  for (; i > 1; i--) {
    if (ver[i] == '-') n++;
    if (n == 2) break;
  }
  if (n != 2) return is_timestamped(ver) ? "SNAPSHOT" : ver;

  auto [base, r] = ver.subview(i + 1);
  return is_timestamped(r) ? base + "SNAPSHOT" : ver;
}
static_assert(*path_of_ver("1.0") == "1.0");
static_assert(*path_of_ver("19520311.052501-42") == "SNAPSHOT");
static_assert(*path_of_ver("1.0-19520311.052501-42") == "1.0-SNAPSHOT");
static_assert(*path_of_ver("1.0-19520311.052501-42a") == "1.0-19520311.052501-42a");
static_assert(*path_of_ver("19520311.052501-42-1.0") == "19520311.052501-42-1.0");
static_assert(*path_of_ver("1.0-a-b-c-19520311.052501-42") == "1.0-a-b-c-SNAPSHOT");

static constexpr auto path_of(jute::view home, jute::view grp, jute::view art, jute::view ver, jute::view type) {
  auto grp_path = grp.cstr();
  for (auto & c : grp_path) if (c == '.') c = '/';

  auto ver_path = path_of_ver(ver);

  auto path = home + "/.m2/repository/" + grp_path + "/" + art + "/" + *ver_path + "/" + art + "-" + ver + "." + type;
  return path.cstr();
}
static_assert(jute::view(path_of("~", "com.pany", "art", "1.0", "pom")) == "~/.m2/repository/com/pany/art/1.0/art-1.0.pom");
static_assert(jute::view(path_of("~", "com.pany", "art", "1.0-19520311.052501-42", "pom")) == "~/.m2/repository/com/pany/art/1.0-SNAPSHOT/art-1.0-19520311.052501-42.pom");

hai::cstr cavan::path_of(jute::view grp, jute::view art, jute::view ver, jute::view type) {
  auto home_env = sysstd::env("HOME");
  if (!home_env) fail("missing HOME environment variable");

  return ::path_of(jute::view::unsafe(home_env), grp, art, ver, type);
}

