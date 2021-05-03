#include <cstdlib>
#include <cstring>

int mull_should_mutate_identifier(const char *identifier) {
  char *mull_env_var = getenv("MULL");
  if (mull_env_var) {
    return strcmp(mull_env_var, identifier) == 0;
  }
  return 0;
}
