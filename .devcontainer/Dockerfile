FROM devkitpro/toolchain-base

RUN dkp-pacman -Syyu --noconfirm switch-dev && \
    dkp-pacman -S --needed --noconfirm `dkp-pacman -Slq dkp-libs | grep '^switch-'` && \
    dkp-pacman -Scc --noconfirm && \
	dkp-pacman -U --needed --noconfirm https://github.com/withertech/libsmm/releases/download/1.0.0/switch-libsmm-1.0.0-1-any.pkg.tar.xz