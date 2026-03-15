FROM fedora:43

RUN dnf install -y \
    gcc \
    make \
    pkg-config \
    libmnl-devel \
    pulseaudio-libs-devel \
    libX11-devel \
    && dnf clean all

WORKDIR /src

COPY . .

RUN make WITH_X11=1

CMD ["/src/verbar"]
