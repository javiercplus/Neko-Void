#!/bin/bash
#
# NekoVoid Live ISO Builder - Nonfree Edition
# Genera la ISO con soporte nonfree: Steam, gaming, drivers propietarios, etc.
#

set -e

# ─────────────────────────────────────────────
# Configuración de salida
# ─────────────────────────────────────────────
ISO_NAME="nekovoid-flavor-5-xfce.iso"
ISO_TITLE="NekoVoid"

# ─────────────────────────────────────────────
# Repositorios (nonfree + multilib)
# ─────────────────────────────────────────────
REPOS_PKGS="void-repo-nonfree void-repo-multilib void-repo-multilib-nonfree"

# ─────────────────────────────────────────────
# Sistema base y utilidades
# ─────────────────────────────────────────────
BASE_SYSTEM="
    at-spi2-core
    bash-completion
    cryptsetup
    dbus
    dialog
    elogind
    grub
    mdadm
    nano
    rtkit
    xdo
    xsetroot
    xinit
"

SYSTEM_UTILS="
    7zip
    p7zip
    unrar
    zip
    xxd
    btop
    fastfetch
    curl
    wget
    git
    xdg-user-dirs
    xdg-utils
    ethtool
    iproute2
    lvm2
    polkit
    udisks2
    eudev
    void-docs-browse
    xtools-minimal
    openssh
    chrony
    procps-ng
"

# ─────────────────────────────────────────────
# Networking
# ─────────────────────────────────────────────
NETWORKING="
    NetworkManager
    network-manager-applet
    wpa_supplicant
    iw
"

# ─────────────────────────────────────────────
# Audio (PipeWire stack completo)
# ─────────────────────────────────────────────
AUDIO="
    pipewire
    wireplumber
    alsa-lib
    alsa-utils
    alsa-pipewire
    libjack-pipewire
    pavucontrol
    pulsemixer
    volumeicon
"

# ─────────────────────────────────────────────
# Xorg + drivers GPU
# ─────────────────────────────────────────────
XORG="
    xorg
    xmirror
    libva-intel-driver
    intel-media-driver
    xf86-video-amdgpu
    xf86-video-intel
    orca
"

GPU_DRIVERS="
    mesa
    mesa-dri
    mesa-vaapi
    vulkan-loader
    Vulkan-Tools
    libglvnd
    linux-firmware-intel
    linux-firmware-amd
"

# ─────────────────────────────────────────────
# Escritorio
# ─────────────────────────────────────────────
DESKTOP="
    lightdm 
    lightdm-gtk-greeter 
    xfce4 gnome-themes-standard 
    gnome-keyring network-manager-applet 
    gvfs-afc gvfs-mtp gvfs-smb udisks2 
    xfce4-pulseaudio-plugin
    xfce4-screenshooter
    picom
"

# ─────────────────────────────────────────────
# Aplicaciones de escritorio
# ─────────────────────────────────────────────
DESKTOP_APPS="
    firefox
    ristretto
    geany
    mpv
    arandr
    xarchiver
    gparted
    gnome-software
"

# ─────────────────────────────────────────────
# Flatpak + portales
# ─────────────────────────────────────────────
FLATPAK="
    flatpak
    xdg-desktop-portal
    xdg-desktop-portal-gtk
    octoxbps
"

# ─────────────────────────────────────────────
# Fuentes
# ─────────────────────────────────────────────
FONTS="
    noto-fonts-emoji
    noto-fonts-cjk
    noto-fonts-ttf
    font-awesome
    dejavu-fonts-ttf
    liberation-fonts-ttf
"

# ─────────────────────────────────────────────
# Multimedia / codecs
# ─────────────────────────────────────────────
MULTIMEDIA="
    ffmpeg
    gstreamer1
    gst-plugins-base1
    gst-plugins-good1
    gst-plugins-bad1
    gst-plugins-ugly1
"

# ─────────────────────────────────────────────
# Librerías 32-bit (requeridas para Steam)
# ─────────────────────────────────────────────
MULTILIB_32BIT="
    mesa-32bit
    mesa-dri-32bit
    libgcc-32bit
    libstdc++-32bit
    libdrm-32bit
    libglvnd-32bit
    glibc-32bit
    vulkan-loader-32bit
    libpulseaudio-32bit
    freetype-32bit
    fontconfig-32bit
    sqlite-32bit
    libXinerama-32bit
    libXcursor-32bit
    gtk+-32bit
    dbus-glib-32bit
    nss-32bit
    nspr-32bit
"

# ─────────────────────────────────────────────
# Gaming
# ─────────────────────────────────────────────
GAMING="
    gamemode
    MangoHud
"

# ─────────────────────────────────────────────
# Otros
# ─────────────────────────────────────────────
OTHER="
    ntp
    zramen
"

# ─────────────────────────────────────────────
# Accesibilidad
# ─────────────────────────────────────────────
ACCESSIBILITY="
    espeakup
    void-live-audio
    brltty
"

# ─────────────────────────────────────────────
# Driver NVIDIA (descomentar si tienes GPU NVIDIA)
# ─────────────────────────────────────────────
#NVIDIA="nvidia nvidia-libs-32bit"
NVIDIA=""

# ─────────────────────────────────────────────
# Construir la lista completa de paquetes
# ─────────────────────────────────────────────
ALL_PACKAGES="
    ${REPOS_PKGS}
    ${BASE_SYSTEM}
    ${SYSTEM_UTILS}
    ${NETWORKING}
    ${AUDIO}
    ${XORG}
    ${GPU_DRIVERS}
    ${DESKTOP}
    ${DESKTOP_APPS}
    ${FLATPAK}
    ${FONTS}
    ${MULTIMEDIA}
    ${MULTILIB_32BIT}
    ${GAMING}
    ${OTHER}
    ${ACCESSIBILITY}
    ${NVIDIA}
"

# Limpiar espacios extra y newlines, convertir a una línea
PACKAGES=$(echo ${ALL_PACKAGES} | tr -s ' ')

echo "============================================="
echo " NekoVoid Live ISO Builder (Nonfree)"
echo "============================================="
echo ""
echo "ISO de salida: ${ISO_NAME}"
echo "Paquetes totales: $(echo ${PACKAGES} | wc -w)"
echo ""

sudo ./mklive.sh \
    -I xfce-dir \
    -o "${ISO_NAME}" \
    -T "${ISO_TITLE}" \
    -p "${PACKAGES}" \
    -S "dbus elogind NetworkManager lightdm polkitd rtkit sshd chronyd"
