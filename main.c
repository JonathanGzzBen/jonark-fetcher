#include "cc_common.h"
#include "cc_hashtable.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statvfs.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>

void load_os_release_info(CC_HashTable *p_table) {
  const unsigned int BUF_SIZE = 256;
  char buf[BUF_SIZE];
  const char *os_release_file_path = "/etc/os-release";
  FILE *os_release_file = fopen(os_release_file_path, "r");
  if (os_release_file == NULL) {
    fprintf(stderr, "Could not open \"%s\"\n", "/etc/os-release");
    return;
  }
  while (fgets(buf, BUF_SIZE, os_release_file) != NULL) {
    buf[strcspn(buf, "\n")] = '\0'; // Remove trailing newline

    // Skip empty or commented line
    if (buf[0] == '\0' || buf[0] == '\n' || buf[0] == '#') {
      continue;
    }

    char *equal_sign = strchr(buf, '=');
    if (equal_sign == NULL) {
      fprintf(stderr, "Malformed line: %s\n", buf);
      continue;
    }

    *equal_sign = '\0';

    char *key = buf;
    char *value = equal_sign + 1;

    // Handle quoted value
    if (value[0] == '"') {
      value++;
      char *end_quote = strchr(value, '"');
      *end_quote = '\0';
    } else {
      value[value - buf] = '\0';
    }

    char os_entry_key[BUF_SIZE];
    memset(os_entry_key, '\0', BUF_SIZE);
    const char *os_entries_prefix = "OS_";
    size_t os_entries_prefix_len = strlen(os_entries_prefix);
    strncpy(os_entry_key, os_entries_prefix, os_entries_prefix_len);
    strncat(os_entry_key, key, BUF_SIZE - os_entries_prefix_len);
    os_entry_key[BUF_SIZE - 1] = '\0';
    if (cc_hashtable_add(p_table, strdup(os_entry_key), strdup(value)) !=
        CC_OK) {
      fprintf(stderr, "Could not add to table: [Key=%s, Value=%s]\n", key,
              value);
      continue;
    }
  }
  fclose(os_release_file);
}

char *unsigned_long_to_str(unsigned long val) {
  const size_t BUF_SIZE = 256;
  char buf[BUF_SIZE];
  snprintf(buf, BUF_SIZE, "%lu", val);
  return strdup(buf);
}

char *long_to_str(unsigned long val) {
  const size_t BUF_SIZE = 256;
  char buf[BUF_SIZE];
  snprintf(buf, BUF_SIZE, "%lu", val);
  return strdup(buf);
}

int load_disk_usage(CC_HashTable *p_table) {
  struct statvfs buf;
  if (statvfs("/", &buf) != 0) {
    perror("statvfs");
    return -1;
  }

  unsigned long block_size = buf.f_frsize;
  unsigned long total_blocks = buf.f_blocks;
  unsigned long free_blocks = buf.f_bfree;
  unsigned long available_blocks = buf.f_bavail;
  unsigned long used_blocks = total_blocks - (total_blocks - free_blocks);

  const unsigned long bytes_in_a_gb = (1024 * 1024 * 1024);
  //  unsigned long total_gb = (total_blocks * block_size) / bytes_in_a_gb;
  unsigned long free_gb = (free_blocks * block_size) / bytes_in_a_gb;
  unsigned long available_gb = (available_blocks * block_size) / bytes_in_a_gb;
  unsigned long used_gb =
      ((total_blocks - free_blocks) * block_size) / bytes_in_a_gb;

  char *free_gb_str = unsigned_long_to_str(free_gb);
  char *available_gb_str = unsigned_long_to_str(available_gb);
  char *used_gb_str = unsigned_long_to_str(used_gb);

  if (cc_hashtable_add(p_table, strdup("DISK_FREE_GB"), free_gb_str) != CC_OK) {
    fprintf(stderr, "Could not add to table: [Key=%s, Value=%s]\n",
            "DISK_FREE_GB", free_gb_str);
  }
  if (cc_hashtable_add(p_table, strdup("DISK_AVAILABLE_GB"),
                       available_gb_str) != CC_OK) {
    fprintf(stderr, "Could not add to table: [Key=%s, Value=%s]\n",
            "DISK_AVAILABLE_GB", available_gb_str);
  }
  if (cc_hashtable_add(p_table, strdup("DISK_USED_GB"), used_gb_str) != CC_OK) {
    fprintf(stderr, "Could not add to table: [Key=%s, Value=%s]\n",
            "DISK_USED_GB", used_gb_str);
  }

  return 0;
}

int load_sys_dmi_info(CC_HashTable *p_table) {
  const unsigned int BUF_SIZE = 256;
  char buf[BUF_SIZE];
  const char *product_name_file_path =
      "/sys/devices/virtual/dmi/id/product_name";
  FILE *product_name_file = fopen(product_name_file_path, "r");
  if (product_name_file == NULL) {
    fprintf(stderr, "Could not open \"%s\"\n", product_name_file_path);
    return -1;
  }
  while (fgets(buf, BUF_SIZE, product_name_file) != NULL) {
    buf[strcspn(buf, "\n")] = '\0'; // Remove trailing newline
  }
  fclose(product_name_file);

  if (cc_hashtable_add(p_table, strdup("SYS_DMI_HARDWARE_MODEL"),
                       strdup(buf)) != CC_OK) {
    fprintf(stderr, "Could not add to table: [Key=%s, Value=%s]\n",
            "SYS_DMI_HARDWARE_MODEL", buf);
  }

  return 0;
}

int load_kernel_info(CC_HashTable *p_table) {
  struct utsname buf;

  if (uname(&buf) < 0) {
    fprintf(stderr, "Could not fetch kernel info\n");
    return -1;
  }

  if (cc_hashtable_add(p_table, strdup("KERNEL_ARCHITECTURE"),
                       strdup(buf.machine)) != CC_OK) {
    fprintf(stderr, "Could not add to table: [Key=%s, Value=%s]\n",
            "KERNEL_ARCHITECTURE", buf.machine);
  }

  if (cc_hashtable_add(p_table, strdup("KERNEL_RELEASE"),
                       strdup(buf.release)) != CC_OK) {
    fprintf(stderr, "Could not add to table: [Key=%s, Value=%s]\n",
            "KERNEL_RELEASE", buf.release);
  }
  return 0;
}

int load_sys_info(CC_HashTable *p_table) {
  struct sysinfo info;

  if (sysinfo(&info) < 0) {
    fprintf(stderr, "Could not fetch system info\n");
    return -1;
  }

  long uptime_hours = info.uptime / 3600;
  long uptime_minutes = (info.uptime - (3600 * uptime_hours)) / 60;
  long uptime_seconds =
      (info.uptime - (3600 * uptime_hours)) - (uptime_minutes * 60);
  char *uptime_str = malloc(256 * sizeof(char));
  snprintf(uptime_str, 256, "%02luh %02lum %02lus", uptime_hours,
           uptime_minutes, uptime_seconds);
  uptime_str[256] = '\0';

  // char *uptime_str = long_to_str(info.uptime);
  char *totalram_str = unsigned_long_to_str(info.totalram / (1024 * 1024));
  char *freeram_str = unsigned_long_to_str(info.freeram / (1024 * 1024));

  if (cc_hashtable_add(p_table, strdup("SYS_UPTIME"), uptime_str) != CC_OK) {
    fprintf(stderr, "Could not add to table: [Key=%s, Value=%s]\n",
            "SYS_UPTIME", uptime_str);
  }
  if (cc_hashtable_add(p_table, strdup("SYS_TOTALRAM_MB"), totalram_str) !=
      CC_OK) {
    fprintf(stderr, "Could not add to table: [Key=%s, Value=%s]\n",
            "SYS_TOTALRAM", totalram_str);
  }
  if (cc_hashtable_add(p_table, strdup("SYS_FREERAM_MB"), freeram_str) !=
      CC_OK) {
    fprintf(stderr, "Could not add to table: [Key=%s, Value=%s]\n",
            "SYS_FREERAM", freeram_str);
  }

  return 0;
}

int main(int argc, char **argv) {

  CC_HashTable *p_info_table;
  enum cc_stat cc_status;
  cc_status = cc_hashtable_new(&p_info_table);
  if (cc_status != CC_OK) {
    fprintf(stderr, "Could not allocate info hashtable\n");
    return 1;
  }

  load_os_release_info(p_info_table);
  load_sys_dmi_info(p_info_table);
  load_disk_usage(p_info_table);
  load_kernel_info(p_info_table);
  load_sys_info(p_info_table);

  const char *keys[] = {"OS_PRETTY_NAME", "SYS_DMI_HARDWARE_MODEL",
                        "KERNEL_RELEASE", "KERNEL_ARCHITECTURE",
                        "SYS_UPTIME",     "SYS_TOTALRAM_MB",
                        "SYS_FREERAM_MB", "DISK_AVAILABLE_GB",
                        "DISK_USED_GB"};
  size_t keys_count = sizeof(keys) / sizeof(keys[0]);
  for (size_t i = 0; i < keys_count; i++) {
    static const char *RED = "\e[0;31m";
    static const char *COLOR_RESET = "\e[0m";
    char *data;
    cc_hashtable_get(p_info_table, (void *)keys[i], (void **)&data);
    printf("%s%s: %s%s\n", RED, keys[i], COLOR_RESET, data);
  }

  printf("\n\nAvailable entries:\n");
  CC_HashTableIter hti;
  TableEntry *p_table_entry;
  cc_hashtable_iter_init(&hti, p_info_table);
  while (cc_hashtable_iter_next(&hti, &p_table_entry) != CC_ITER_END) {
    const char *key = p_table_entry->key;
    const char *value = p_table_entry->value;
    printf("[Key=%s,Value=%s]\n", key, value);
  }

  cc_hashtable_destroy(p_info_table);

  return 0;
}
