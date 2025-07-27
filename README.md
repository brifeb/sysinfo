# sysinfo

**sysinfo** adalah utilitas sederhana untuk menampilkan informasi sistem Linux seperti:
- Waktu dan tanggal sekarang
- User aktif
- Versi OS (dari `/etc/os-release`)
- Kernel Linux
- Disk yang ter-mount + kapasitas (df-style)
- Suhu CPU (dari `/sys/class/thermal/thermal_zone0/temp`)
- Uptime sistem

## Cara Build
```bash
make
