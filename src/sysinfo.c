/*
 * sysinfo.c - Program sederhana untuk menampilkan info sistem
 * 
 * Program ini dikembangkan oleh Brifeb & ChatGPT (OpenAI).
 * Lisensi: GNU GPL v3.
 *
 * CHANGELOG:
 * v1.0 (2025-07-26): Rilis awal.
 * v1.1 (2025-07-26): Tambah info kapasitas disk (df -h style).
 * v1.2 (2025-07-26): Header tanggal & warna ANSI.
 * v1.3 (2025-07-26): Tambah opsi --version & --help.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/utsname.h>
#include <sys/statvfs.h>

#define VERSION "1.3"
#define COLOR_RESET  "\033[0m"
#define COLOR_HEADER "\033[1;36m"
#define COLOR_TITLE  "\033[1;33m"
#define COLOR_VALUE  "\033[1;37m"
#define COLOR_TABLE  "\033[0;32m"

// List filesystem untuk skip
static const char *SKIP_FS[] = {
    "proc","sysfs","devtmpfs","tmpfs","cgroup","cgroup2","pstore","securityfs",
    "debugfs","tracefs","configfs","overlay","squashfs","ramfs","autofs",
    "binfmt_misc","fusectl","bpf","nsfs",NULL
};
static const char *SKIP_DEV_PREFIX[] = {
    "none","proc","sysfs","tmpfs","devtmpfs","cgroup","overlay","udev",NULL
};

static int should_skip(const char *device, const char *fstype) {
    for (int i = 0; SKIP_FS[i]; ++i)
        if (strcmp(fstype, SKIP_FS[i]) == 0) return 1;
    for (int i = 0; SKIP_DEV_PREFIX[i]; ++i)
        if (strncmp(device, SKIP_DEV_PREFIX[i], strlen(SKIP_DEV_PREFIX[i])) == 0) return 1;
    return 0;
}

static void human_readable(unsigned long long bytes, char *out, size_t n) {
    const char *units[] = {"B","K","M","G","T","P"};
    int i = 0;
    double sz = (double)bytes;
    while (sz >= 1024.0 && i < 5) { sz /= 1024.0; i++; }
    snprintf(out, n, "%.1f%s", sz, units[i]);
}

// HEADER
void print_header_datetime() {
    time_t t = time(NULL);
    struct tm *tmp = localtime(&t);
    char buf[64];
    strftime(buf, sizeof(buf), "%d %B %Y %H:%M:%S", tmp);
    printf(COLOR_HEADER "=================== %s ==================\n" COLOR_RESET, buf);
}

// USER
void print_user() {
    char *user = getenv("USER");
    printf(COLOR_TITLE "User               : " COLOR_VALUE "%s\n" COLOR_RESET, user ? user : "(unknown)");
}

// OS RELEASE
void print_os_release() {
    FILE *fp = fopen("/etc/os-release", "r");
    if (!fp) {
        printf(COLOR_TITLE "OS Release         : " COLOR_VALUE "(tidak bisa dibaca)\n" COLOR_RESET);
        return;
    }
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "PRETTY_NAME=", 12) == 0) {
            char *val = strchr(line, '=');
            if (val) {
                val++;
                if (*val == '\"') val++;
                char *end = strchr(val, '\"');
                if (end) *end = '\0';
                printf(COLOR_TITLE "OS Release         : " COLOR_VALUE "%s\n" COLOR_RESET, val);
            }
            break;
        }
    }
    fclose(fp);
}

// KERNEL
void print_linux_kernel() {
    struct utsname uts;
    if (uname(&uts) == 0)
        printf(COLOR_TITLE "Kernel Linux       : " COLOR_VALUE "%s\n" COLOR_RESET, uts.release);
    else
        printf(COLOR_TITLE "Kernel Linux       : " COLOR_VALUE "(tidak bisa dibaca)\n" COLOR_RESET);
}

// DISK INFO
void print_mounted_drives_with_df() {
    printf(COLOR_TITLE "Mounted SSD/HDD    :\n" COLOR_RESET);
    FILE *fp = fopen("/proc/mounts", "r");
    if (!fp) {
        printf("  (tidak bisa dibaca)\n");
        return;
    }
    printf(COLOR_TABLE "  %-18s %-18s %-8s %8s %8s %8s %6s\n" COLOR_RESET,
           "Filesystem", "Mountpoint", "Type", "Size", "Used", "Avail", "Use%");

    char device[256], mountpoint[256], fstype[64], rest[256];
    while (fscanf(fp, "%255s %255s %63s %255[^\n]\n", device, mountpoint, fstype, rest) == 4) {
        if (should_skip(device, fstype)) continue;

        struct statvfs vfs;
        if (statvfs(mountpoint, &vfs) != 0) continue;

        unsigned long long total = (unsigned long long)vfs.f_frsize * vfs.f_blocks;
        unsigned long long free_ = (unsigned long long)vfs.f_frsize * vfs.f_bfree;
        unsigned long long used = total - free_;
        unsigned long long avail = (unsigned long long)vfs.f_frsize * vfs.f_bavail;
        int pct = (total > 0) ? (int)((used * 100.0) / total + 0.5) : 0;

        char h_total[16], h_used[16], h_avail[16];
        human_readable(total, h_total, sizeof(h_total));
        human_readable(used,  h_used,  sizeof(h_used));
        human_readable(avail, h_avail, sizeof(h_avail));

        printf("  %-18s %-18s %-8s %8s %8s %8s %5d%%\n",
               device, mountpoint, fstype, h_total, h_used, h_avail, pct);
    }
    fclose(fp);
}

// SUHU
void print_temperature() {
    FILE *fp = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
    if (!fp) {
        printf(COLOR_TITLE "Suhu               : " COLOR_VALUE "(tidak bisa dibaca)\n" COLOR_RESET);
        return;
    }
    int temp_milideg;
    if (fscanf(fp, "%d", &temp_milideg) == 1)
        printf(COLOR_TITLE "Suhu               : " COLOR_VALUE "%.1f°C\n" COLOR_RESET, temp_milideg / 1000.0);
    fclose(fp);
}

// UPTIME
void print_uptime() {
    FILE *fp = fopen("/proc/uptime", "r");
    if (!fp) {
        printf(COLOR_TITLE "Uptime             : " COLOR_VALUE "(tidak bisa dibaca)\n" COLOR_RESET);
        return;
    }
    double uptime_seconds;
    if (fscanf(fp, "%lf", &uptime_seconds) == 1) {
        int days = (int)(uptime_seconds / 86400);
        int hours = (int)((uptime_seconds - days * 86400) / 3600);
        int minutes = (int)((uptime_seconds - days * 86400 - hours * 3600) / 60);
        printf(COLOR_TITLE "Uptime             : " COLOR_VALUE "%d hari, %d jam, %d menit\n" COLOR_RESET,
               days, hours, minutes);
    }
    fclose(fp);
}

// HELP
void print_help() {
    printf("sysinfo - Menampilkan informasi sistem\n");
    printf("Usage: sysinfo [OPTION]\n\n");
    printf("  --help       Tampilkan bantuan ini\n");
    printf("  --version    Tampilkan versi program\n\n");
    printf("Tanpa opsi, program akan menampilkan informasi lengkap.\n");
}

// VERSION
void print_version() {
    printf("sysinfo v%s - by Brifeb & ChatGPT (OpenAI)\n", VERSION);
}

// MAIN
int main(int argc, char *argv[]) {
    if (argc > 1) {
        if (strcmp(argv[1], "--help") == 0) {
            print_help();
            return 0;
        }
        if (strcmp(argv[1], "--version") == 0) {
            print_version();
            return 0;
        }
    }

    print_header_datetime();
    print_user();
    print_os_release();
    print_linux_kernel();
    print_mounted_drives_with_df();
    print_temperature();
    print_uptime();
    return 0;
}
